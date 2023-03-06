load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("//tools:python_rules.bzl", "py_proto_library")

package(default_visibility = ["//visibility:public"])

#mobile_config_proto
py_proto_library(
    name = "mobile_config_py_proto",
    deps = [
        ":mobile_config_proto",
    ],
)

cc_proto_library(
    name = "mobile_config_cc_proto",
    deps = [
        ":mobile_config_proto",
    ],
)

proto_library(
    name = "mobile_config_proto",
    srcs = [
        "mobile_config.proto",
    ],
)


py_proto_library(
    name = "mobile_pos_msg_py_proto",
    deps = [
        ":mobile_pos_msg_proto",
    ],
)

cc_proto_library(
    name = "mobile_pos_msg_cc_proto",
    deps = [
        ":mobile_pos_msg_proto",
    ],
)

proto_library(
    name = "mobile_pos_msg_proto",
    srcs = [
        "mobile_pos_msg.proto",
    ],
    deps = [
        "//modules/common/proto:header_proto",
    ]
)