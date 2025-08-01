/* Copyright 2019 The OpenXLA Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "xla/service/gpu/hlo_algorithm_denylist.h"

#include <cstdlib>
#include <memory>
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "xla/debug_options_flags.h"
#include "xla/hlo/ir/hlo_instructions.h"
#include "xla/hlo/ir/hlo_opcode.h"
#include "xla/hlo/testlib/hlo_hardware_independent_test_base.h"
#include "xla/hlo/testlib/verified_hlo_module.h"
#include "xla/service/hlo.pb.h"
#include "xla/stream_executor/dnn.h"
#include "xla/tests/test_utils.h"
#include "xla/tsl/platform/env.h"
#include "xla/tsl/platform/status_matchers.h"
#include "xla/tsl/platform/statusor.h"
#include "xla/tsl/platform/test.h"
#include "tsl/platform/path.h"

namespace xla {
namespace gpu {
namespace {
using ::testing::IsEmpty;
using ::testing::SizeIs;
using ::tsl::testing::IsOk;

class DenylistTest : public HloHardwareIndependentTestBase {
 protected:
  DenylistTest() {
    std::string existing_xla_flags;
    const char* env = std::getenv("XLA_FLAGS");
    if (env != nullptr) {
      existing_xla_flags = absl::StrCat(env, " ");
    }

    tsl::setenv(
        "XLA_FLAGS",
        absl::StrCat(
            existing_xla_flags, "--xla_gpu_algorithm_denylist_path=",
            tsl::io::JoinPath(tsl::testing::XlaSrcRoot(), "service", "gpu",
                              "data", "hlo_algorithm_denylist.pbtxt"))
            .data(),
        /*overwrite=*/1);
    ParseDebugOptionFlagsFromEnv(false);
    config_ =
        ParseTextProto<GpuBackendConfig>(
            "operation_queue_id: 0 wait_on_operation_queues: [] "
            "cudnn_conv_backend_config: { activation_mode: kNone "
            "conv_result_scale: 1 side_input_scale: 0 leakyrelu_alpha: 0} "
            "force_earliest_schedule: false")
            .value();
  }
  GpuBackendConfig config_;
};

TEST_F(DenylistTest, DefaultTest) {
  TF_ASSERT_OK_AND_ASSIGN(std::unique_ptr<VerifiedHloModule> module,
                          ParseAndReturnVerifiedModule(R"hlo(
      HloModule module
      ENTRY main {
          arg1 = f16[256,224,224,4]{3,2,1,0} parameter(0)
          arg2 = f16[7,7,4,64]{2,1,0,3} parameter(1)
          ROOT root = (f16[256,112,112,64]{3,2,1,0}, u8[0]{0}) custom-call(arg1, arg2), window={size=7x7 stride=2x2 pad=3_3x3_3}, dim_labels=b01f_01io->b01f, custom_call_target="__cudnn$convForward", backend_config={"operation_queue_id":"0","wait_on_operation_queues":[],"cudnn_conv_backend_config":{"activation_mode":"kNone","conv_result_scale":1,"side_input_scale":0,"leakyrelu_alpha":0},"force_earliest_schedule":false,"reification_cost":[]}
      }
  )hlo"));

  ASSERT_EQ(module->entry_computation()->root_instruction()->opcode(),
            HloOpcode::kCustomCall);

  ComputeCapability cc;
  cc.set_major(7);
  cc.set_minor(0);
  CudnnVersion cudnn_version;
  cudnn_version.set_major(7);
  cudnn_version.set_minor(6);
  cudnn_version.set_patch(2);
  auto list = GetDisabledConvAlgorithms(
      cc, cudnn_version, /*blas_version=*/"9000",
      static_cast<const HloCustomCallInstruction&>(
          *module->entry_computation()->root_instruction()));
  // HloStringWithGpuBackendConfig(
  //     R"((f16[256,112,112,64]{3,2,1,0}, u8[0]{0})
  //     custom-call(f16[256,224,224,4]{3,2,1,0}, f16[7,7,4,64]{2,1,0,3}),
  //     window={size=7x7 stride=2x2 pad=3_3x3_3}, dim_labels=b01f_01io->b01f,
  //     custom_call_target="__cudnn$convForward")", config_));
  EXPECT_THAT(list, testing::UnorderedElementsAre(
                        stream_executor::dnn::AlgorithmDesc{0, true},
                        stream_executor::dnn::AlgorithmDesc{0, false},
                        stream_executor::dnn::AlgorithmDesc{1, true},
                        stream_executor::dnn::AlgorithmDesc{1, false},
                        stream_executor::dnn::AlgorithmDesc{42, true},
                        stream_executor::dnn::AlgorithmDesc{42, false}));
}

TEST_F(DenylistTest, NoBlasVersionSet) {
  TF_ASSERT_OK_AND_ASSIGN(std::unique_ptr<VerifiedHloModule> module,
                          ParseAndReturnVerifiedModule(R"hlo(
      HloModule module
      ENTRY main {
          arg1 = f16[256,224,224,4]{3,2,1,0} parameter(0)
          arg2 = f16[7,7,4,64]{2,1,0,3} parameter(1)
          ROOT root = (f16[256,112,112,64]{3,2,1,0}, u8[0]{0}) custom-call(arg1, arg2), window={size=7x7 stride=2x2 pad=3_3x3_3}, dim_labels=b01f_01io->b01f, custom_call_target="__cudnn$convForward", backend_config={"operation_queue_id":"0","wait_on_operation_queues":[],"cudnn_conv_backend_config":{"activation_mode":"kNone","conv_result_scale":1,"side_input_scale":0,"leakyrelu_alpha":0},"force_earliest_schedule":false,"reification_cost":[]}
      }
  )hlo"));

  ASSERT_EQ(module->entry_computation()->root_instruction()->opcode(),
            HloOpcode::kCustomCall);

  ComputeCapability cc;
  cc.set_major(7);
  cc.set_minor(0);
  CudnnVersion cudnn_version;
  cudnn_version.set_major(7);
  cudnn_version.set_minor(6);
  cudnn_version.set_patch(2);
  auto list = GetDisabledConvAlgorithms(
      cc, cudnn_version, /*blas_version=*/"120301",
      static_cast<const HloCustomCallInstruction&>(
          *module->entry_computation()->root_instruction()));
  EXPECT_THAT(list, testing::UnorderedElementsAre(
                        stream_executor::dnn::AlgorithmDesc{42, true},
                        stream_executor::dnn::AlgorithmDesc{42, false}));
}

TEST_F(DenylistTest, EntryFromHardcodedList) {
  TF_ASSERT_OK_AND_ASSIGN(std::unique_ptr<VerifiedHloModule> module,
                          ParseAndReturnVerifiedModule(R"hlo(
      HloModule module
      ENTRY main {
         arg1 = f32[512,512,7,7]{3,2,1,0} parameter(0)
         arg2 = f32[512,512,3,3]{3,2,1,0} parameter(1)
         arg3 = f32[512]{0} parameter(2)
         ROOT root = (f32[512,512,7,7]{3,2,1,0}, u8[0]{0}) custom-call(arg1, arg2, arg3), window={size=3x3 pad=1_1x1_1}, dim_labels=bf01_oi01->bf01, custom_call_target="__cudnn$convBiasActivationForward", backend_config={"operation_queue_id":"0","wait_on_operation_queues":[],"cudnn_conv_backend_config":{"activation_mode":"kNone","conv_result_scale":1,"side_input_scale":0,"leakyrelu_alpha":0},"force_earliest_schedule":false,"reification_cost":[]}
      }
  )hlo"));

  ASSERT_EQ(module->entry_computation()->root_instruction()->opcode(),
            HloOpcode::kCustomCall);

  ComputeCapability cc;
  cc.set_major(7);
  cc.set_minor(0);
  CudnnVersion cudnn_version;
  cudnn_version.set_major(9);
  cudnn_version.set_minor(0);
  cudnn_version.set_patch(0);
  auto list = GetDisabledConvAlgorithms(
      cc, cudnn_version, /*blas_version=*/"9000",
      static_cast<const HloCustomCallInstruction&>(
          *module->entry_computation()->root_instruction()));
  EXPECT_THAT(list, testing::ElementsAre(
                        stream_executor::dnn::AlgorithmDesc{14, false}));
}

TEST_F(DenylistTest, GenerateDenyListEntry) {
  TF_ASSERT_OK_AND_ASSIGN(std::unique_ptr<VerifiedHloModule> module,
                          ParseAndReturnVerifiedModule(R"hlo(
      HloModule module
      ENTRY main {
         arg1 = f32[512,512,7,7]{3,2,1,0} parameter(0)
         arg2 = f32[512,512,3,3]{3,2,1,0} parameter(1)
         arg3 = f32[512]{0} parameter(2)
         ROOT root = (f32[512,512,7,7]{3,2,1,0}, u8[0]{0}) custom-call(arg1, arg2, arg3), window={size=3x3 pad=1_1x1_1}, dim_labels=bf01_oi01->bf01, custom_call_target="__cudnn$convBiasActivationForward", backend_config={"operation_queue_id":"0","wait_on_operation_queues":[],"cudnn_conv_backend_config":{"activation_mode":"kNone","conv_result_scale":1,"side_input_scale":0,"leakyrelu_alpha":0},"force_earliest_schedule":false,"reification_cost":[]}
      }
  )hlo"));

  ASSERT_EQ(module->entry_computation()->root_instruction()->opcode(),
            HloOpcode::kCustomCall);

  ComputeCapability cc;
  cc.set_major(7);
  cc.set_minor(0);
  CudnnVersion cudnn_version;
  cudnn_version.set_major(9);
  cudnn_version.set_minor(0);
  cudnn_version.set_patch(0);
  absl::string_view blas_version = "9000";

  TF_ASSERT_OK_AND_ASSIGN(
      std::string denylist,
      GenerateDenyListEntry(
          static_cast<const HloCustomCallInstruction&>(
              *module->entry_computation()->root_instruction()),
          stream_executor::dnn::AlgorithmDesc{14, false}, cc, cudnn_version,
          blas_version));

  DenyListMapType denylist_map;
  // Obviously this should return an empty list, since the denylist is empty.
  EXPECT_THAT(GetDisabledConvAlgorithms(
                  denylist_map, cc, cudnn_version, blas_version,
                  static_cast<const HloCustomCallInstruction&>(
                      *module->entry_computation()->root_instruction())),
              IsEmpty());

  ASSERT_THAT(ParseTextFormatDenyList(denylist_map, denylist), IsOk());

  // After reading the previously generated denylist entry, we should match this
  // exact entry.
  EXPECT_THAT(GetDisabledConvAlgorithms(
                  denylist_map, cc, cudnn_version, blas_version,
                  static_cast<const HloCustomCallInstruction&>(
                      *module->entry_computation()->root_instruction())),
              SizeIs(1));
}

}  // namespace
}  // namespace gpu
}  // namespace xla
