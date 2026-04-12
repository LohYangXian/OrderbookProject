load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main_server",
    srcs = ["main_server.cpp"],
    copts = [
        "-std=c++20",
        ], 
    deps = [
        "//src/server:Server",
        "//src/om:Orderbook",
        ],
)

cc_binary(
    name = "main_engine_benchmark",
    srcs = ["main_engine_benchmark.cpp"],
    copts = [
        "-std=c++20",
        ],
    deps = [
        "//src/om:Orderbook",
        ],
)