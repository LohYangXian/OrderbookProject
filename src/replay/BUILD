cc_binary(
    name = "replay",
    srcs = ["replay.cpp"],
    deps = [
        "@websocketpp//:websocketpp",
        "@openssl//:openssl",
        "@asio//:asio",
    ],
    linkopts = [
        "-lssl",
        "-lcrypto",
    ],
)