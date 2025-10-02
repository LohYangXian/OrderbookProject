cc_library(
    name = "Orderbook",
    srcs = ["om/Orderbook.cpp"],
    hdrs = [
        "om/Orderbook.h",
        "om/Usings.h",
        "om/Side.h",
        "om/Order.h",
        "om/TradeInfo.h",
        "om/Trade.h",
    ],
    copts = ["-std=c++20"],
    deps = [
        "@nlohmann_json//:json",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    copts = ["-std=c++20"],
    deps = [
        ":Orderbook",
        "@nlohmann_json//:json",
    ],
)