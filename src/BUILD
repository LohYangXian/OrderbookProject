cc_library(
    name = "order_book",
    srcs = ["om/order_book.cpp"],
    hdrs = ["om/order_book.h"],
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
        ":order_book",
        "@nlohmann_json//:json",
    ],
)