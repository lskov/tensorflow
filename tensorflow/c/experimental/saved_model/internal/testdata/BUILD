load("//tensorflow:strict.default.bzl", "py_strict_binary")
load("//tensorflow:tensorflow.default.bzl", "filegroup")

package(
    # copybara:uncomment default_applicable_licenses = ["//tensorflow:license"],
    licenses = ["notice"],
)

# Run this binary manually, with an argument pointing to the testdata/
# directory, to generate the test files used by the filegroup rule below.
py_strict_binary(
    name = "gen_saved_models",
    srcs = ["gen_saved_models.py"],
    deps = [
        "//tensorflow/python/compat:v2_compat",
        "//tensorflow/python/eager:def_function",
        "//tensorflow/python/framework:constant_op",
        "//tensorflow/python/framework:dtypes",
        "//tensorflow/python/framework:tensor_spec",
        "//tensorflow/python/module",
        "//tensorflow/python/ops:resource_variable_ops",
        "//tensorflow/python/ops:variables",
        "//tensorflow/python/ops:while_loop",
        "//tensorflow/python/saved_model",
        "@absl_py//absl:app",
    ],
)

# Files generated by the binary above.
filegroup(
    name = "saved_models",
    srcs = glob([
        "SimpleWhileLoop/**",
        "UninitializedVariable/**",
    ]),
    visibility = [
        "//tensorflow/c/experimental/saved_model/internal:__pkg__",
    ],
)
