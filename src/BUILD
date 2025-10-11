cc_binary(
    name = "main_manual",
    srcs = ["main_manual.cpp"],
    copts = ["-std=c++20"],
    deps = [
        "//src/om:Orderbook",
        "@nlohmann_json//:json",
    ],
)

cc_binary(
    name = "main_server",
    srcs = ["main_server.cpp"],
    copts = [
        "-std=c++20",
        ], 
    deps = [
        "//src/server:Server",
        "//src/om:Orderbook",
        "@nlohmann_json//:json",
        ],   
    linkopts = [
        "-lprofiler",
    ],
)