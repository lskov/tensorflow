# Description:
#   TensorFlow/TensorFlow Lite/XLA MLIR dialects and tools.

load(
    "//tensorflow:tensorflow.bzl",
    "tf_cc_binary",
    "tf_cc_test",
)
load("//tensorflow:tensorflow.default.bzl", "filegroup")
load("//tensorflow/core/platform:rules_cc.bzl", "cc_library")

package(
    # copybara:uncomment default_applicable_licenses = ["//tensorflow:license"],
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],
)

package_group(
    name = "subpackages",
    packages = ["//tensorflow/compiler/mlir/..."],
)

exports_files(glob(["g3doc/*.md"] + ["g3doc/_includes/tf_passes.md"]))

# To reference all tablegen files here when checking for updates to them.
filegroup(
    name = "td_files",
    srcs = glob(["**/*.td"]),
)

cc_library(
    name = "op_or_arg_name_mapper",
    srcs = ["op_or_arg_name_mapper.cc"],
    hdrs = ["op_or_arg_name_mapper.h"],
    deps = [
        "//tensorflow/compiler/mlir/utils:name_utils",
        "@com_google_absl//absl/strings",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Support",
    ],
)

tf_cc_test(
    name = "op_or_arg_name_mapper_test",
    srcs = ["op_or_arg_name_mapper_test.cc"],
    deps = [
        ":op_or_arg_name_mapper",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Support",
    ],
)

cc_library(
    name = "tf_mlir_opt_main",
    testonly = True,
    srcs = ["tf_mlir_opt_main.cc"],
    deps = [
        ":init_mlir",
        ":passes",
        ":register_common_dialects",
        "//tensorflow/compiler/mlir/quantization/stablehlo:bridge_passes",
        "//tensorflow/compiler/mlir/tensorflow:mlprogram_util",
        "//tensorflow/compiler/mlir/tensorflow/transforms:tensorflow_passes",
        "//tensorflow/compiler/mlir/tensorflow/transforms:tensorflow_test_passes",
        "//tensorflow/compiler/mlir/tensorflow/transforms:tf_graph_optimization_pass",
        "//tensorflow/compiler/mlir/tensorflow/transforms:tf_saved_model_passes",  # buildcleaner:keep
        "//tensorflow/compiler/mlir/tensorflow/transforms/host_runtime:lower_cluster_to_runtime_ops",
        "//tensorflow/compiler/mlir/tensorflow/transforms/host_runtime:runtime_passes",
        "//tensorflow/compiler/mlir/tensorflow/transforms/sparsecore:sparsecore_passes",
        "//tensorflow/compiler/mlir/tf2xla:compile_mlir_util",
        "//tensorflow/compiler/mlir/tf2xla/internal/passes:clustering_passes",
        "//tensorflow/compiler/mlir/tf2xla/internal/passes:mlir_to_graph_passes",
        "//tensorflow/compiler/mlir/tf2xla/transforms:tf_xla_passes",
        "//tensorflow/compiler/mlir/tf2xla/transforms:xla_legalize_tf",
        "@llvm-project//mlir:AllPassesAndDialects",
        "@llvm-project//mlir:MlirOptLib",
        "@llvm-project//mlir:Support",
        "@llvm-project//mlir:Transforms",
        "@local_xla//xla/mlir/framework/transforms:passes",
        "@local_xla//xla/mlir_hlo:all_passes",
    ],
)

cc_library(
    name = "passes",
    visibility = ["//visibility:public"],
    deps = [
        "@llvm-project//mlir:AffineDialect",
        "@llvm-project//mlir:QuantOps",
        # Link jit lib to link JIT devices required to run
        # xla-legalize-tf-with-tf2xla pass.
        "//tensorflow/compiler/jit",
        "//tensorflow/compiler/mlir/tensorflow",
        "//tensorflow/compiler/mlir/tensorflow/transforms:tensorflow_passes",
        "//tensorflow/compiler/mlir/tensorflow/transforms:tf_dialect_passes",
        "//tensorflow/compiler/mlir/tf2xla:compile_mlir_util",
    ],
)

cc_library(
    name = "init_mlir",
    srcs = ["init_mlir.cc"],
    hdrs = ["init_mlir.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//tensorflow/core:lib",
        "@llvm-project//llvm:Support",
    ],
)

cc_library(
    name = "mlir_graph_optimization_pass",
    srcs = ["mlir_graph_optimization_pass.cc"],
    hdrs = ["mlir_graph_optimization_pass.h"],
    deps = [
        "//tensorflow/compiler/mlir/tensorflow",
        "//tensorflow/compiler/mlir/tensorflow:attribute_utils",
        "//tensorflow/compiler/mlir/tensorflow:device_util",
        "//tensorflow/compiler/mlir/tensorflow:dump_mlir_util",
        "//tensorflow/compiler/mlir/tensorflow:mlir_roundtrip_flags",
        "//tensorflow/compiler/mlir/tf2xla:mlir_bridge_rollout_policy",
        "//tensorflow/compiler/mlir/tf2xla/api/v2:graph_to_tf_executor",
        "//tensorflow/compiler/mlir/tf2xla/api/v2:tf_executor_to_graph",
        "//tensorflow/core:core_cpu",
        "//tensorflow/core:framework",
        "//tensorflow/core:lib",
        "//tensorflow/core:protos_all_cc",
        "//tensorflow/core/common_runtime:device_set",
        "//tensorflow/core/protobuf:for_core_protos_cc",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:string_view",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:ArithDialect",
        "@llvm-project//mlir:FuncDialect",
        "@llvm-project//mlir:FuncExtensions",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:ShapeDialect",
    ],
    alwayslink = 1,
)

cc_library(
    name = "mlir_graph_optimization_pass_registration",
    srcs = [
        "mlir_graph_optimization_pass_registration.cc",
    ],
    deps = [
        ":mlir_graph_optimization_pass",
        "//tensorflow/core:core_cpu",
    ],
    alwayslink = 1,
)

# This should just be a wrapper around tf_mlir_opt_main. Don't add
# direct dependencies to this binary.
tf_cc_binary(
    name = "tf-opt",
    testonly = True,
    deps = [
        ":tf_mlir_opt_main",
    ],
)

tf_cc_binary(
    name = "tf-reduce",
    testonly = True,
    srcs = ["tf_mlir_reduce_main.cc"],
    deps = [
        ":init_mlir",
        ":register_common_dialects",
        "//tensorflow/compiler/mlir/tensorflow",
        "//tensorflow/compiler/mlir/tensorflow/transforms:tensorflow_reduce_patterns_inc_gen",
        "@llvm-project//mlir:AllPassesAndDialects",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:MlirReduceLib",
        "@llvm-project//mlir:Reducer",
    ],
)

cc_library(
    name = "register_common_dialects",
    testonly = True,
    srcs = ["register_common_dialects.cc"],
    hdrs = ["register_common_dialects.h"],
    deps = [
        "//tensorflow/compiler/mlir/tensorflow",
        "//tensorflow/compiler/mlir/tensorflow:mlprogram_util",
        "//tensorflow/compiler/mlir/tools/kernel_gen/ir:tf_framework_ops",
        "//tensorflow/core/ir/types:Dialect",
        "@llvm-project//mlir:AllExtensions",
        "@llvm-project//mlir:AllPassesAndDialects",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:MlirOptLib",
        "@llvm-project//mlir:QuantOps",
        "@llvm-project//mlir:ShapeDialect",
        "@llvm-project//mlir:TensorDialect",
        "@llvm-project//mlir:TosaDialect",
        "@llvm-project//mlir:Transforms",
        "@local_xla//xla/mlir/framework/ir:xla_framework",
        "@local_xla//xla/mlir_hlo:hlo_dialect_registration",
        "@stablehlo//:register",
    ],
)

tf_cc_test(
    name = "register_common_dialects_test",
    srcs = ["register_common_dialects_test.cc"],
    deps = [
        ":register_common_dialects",
        "@com_google_googletest//:gtest_main",
        "@llvm-project//mlir:IR",
    ],
)

tf_cc_binary(
    name = "tf-mlir-translate",
    testonly = True,
    srcs = ["tf_mlir_translate_main.cc"],
    deps = [
        ":init_mlir",
        "//tensorflow/compiler/mlir/tensorflow:tf_xla_mlir_translate",
        "//tensorflow/compiler/mlir/tensorflow:translate_lib",
        "//tensorflow/compiler/mlir/tf2xla/tests/registration:graph_to_tf_executor_registration",
        "//tensorflow/compiler/mlir/tools:translate_cl_options",
        "//tensorflow/compiler/mlir/tools:translate_registration",
        "//tensorflow/core:lib",
        "//tensorflow/core:tensorflow",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Support",
        "@llvm-project//mlir:TranslateLib",
        "@local_xla//xla/hlo/translate/hlo_to_mhlo:translate_registration",
        "@local_xla//xla/hlo/translate/mhlo_to_hlo:translate_registration",
    ],
)

tf_cc_test(
    name = "mlir_graph_optimization_pass_test",
    srcs = ["mlir_graph_optimization_pass_test.cc"],
    deps = [
        ":mlir_graph_optimization_pass",
        "//tensorflow/core:core_cpu",
        "//tensorflow/core:framework",
        "//tensorflow/core:ops",
        "//tensorflow/core:portable_gif_internal",
        "//tensorflow/core:protos_all_cc",
        "//tensorflow/core:test",
        "//tensorflow/core:test_main",
        "//tensorflow/core/common_runtime:device_set",
        "//tensorflow/core/common_runtime:optimization_registry",
        "//tensorflow/core/framework:tensor_testutil",
        "//tensorflow/core/lib/monitoring:cell_reader",
        "//tensorflow/core/platform:status",
        "//tensorflow/core/protobuf:for_core_protos_cc",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
    ],
)

filegroup(
    name = "litfiles",
    srcs = glob(["runlit*py"]),
    visibility = ["//tensorflow:__subpackages__"],
)

exports_files(["run_lit.sh"])
