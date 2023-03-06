load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//tools/install:install.bzl", "install")
load("//tools:cpplint.bzl", "cpplint")

package(default_visibility=["//visibility:public"])

cc_binary(
    name="libws_mobile.so",
    linkshared=True,
    linkstatic=False,
    linkopts = [
        "-Wl,-rpath,/apollo/modules/YOURMODULE/websocket_mobile/lib",
        # "-Wl,-rpath,$$ORIGIN/path2lib",
    ],
    deps=[":ws_mobile_lib"]
)


cc_library(
    name="ws_mobile_lib",
    srcs=[
        "mobile_communication.cpp",
        "websocket_server.cpp",
    ],
    hdrs=[
        "mobile_communication.h",
        "websocket_server.h",
        "gps_proj.hpp",
    ],
    includes = [
        "3rdparty/websocketpp",
    ],
    visibility=["//visibility:public"],
    deps=[
        "@boost",
        "//cyber",
        ":pj_transformer",
        "@com_github_nlohmann_json//:json",
        "//modules/YOURMODULE/websocket_mobile/proto:mobile_config_cc_proto",
        "//modules/YOURMODULE/websocket_mobile/proto:mobile_pos_msg_cc_proto",
        "@proj",
    ],
)
cc_library(
    name = "pj_transformer",
    srcs = ["gps_proj.hpp"],
    deps = [
        "//cyber",
        "@proj",
    ],
)
filegroup(
    name = "runtime_data",
    srcs = glob([
        "conf/*.pb.txt",
        "dag/*.dag",
        "launch/*.launch",
    ]),
)
