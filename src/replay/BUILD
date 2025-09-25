cc_binary(
    name = "replay",
    srcs = ["replay.cpp"],
    deps = [
        "@websocketpp//:websocketpp",
        "@openssl//:openssl",
        "@asio//:asio",
    ],
    linkopts = [
        "-L/opt/homebrew/Cellar/openssl@3/3.5.2/lib", # We need this to find OpenSSL on macOS with Homebrew
        "-lssl", "-lcrypto"],
)