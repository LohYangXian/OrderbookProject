load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "Server",
    srcs = ["Server.cpp"],
    hdrs = ["Server.h"],
    copts = ["-std=c++20"],
    deps = [
        "//src/om:Orderbook",
        "@nlohmann_json//:json",
    ],
    visibility = ["//src:__pkg__"],  # Only src/ can depend on this
)