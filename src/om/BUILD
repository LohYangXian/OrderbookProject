load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "Orderbook",
    srcs = ["Orderbook.cpp"],
    hdrs = [
        "Orderbook.h",
        "Usings.h",
        "Side.h",
        "Order.h",
        "TradeInfo.h",
        "Trade.h",
        "OrderModify.h",
    ],
    copts = ["-std=c++20"],
    deps = ["@nlohmann_json//:json"],

    visibility = ["//visibility:public"],

    includes = ["./"]

)
