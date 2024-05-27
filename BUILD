cc_library(
    name = "nvme",
    srcs = glob(["src/*.cpp"]),
    hdrs = glob([
        "include/*.h",
        "src/*.h"
    ]),
    copts = [
        "-Iinclude",
        "-Isrc"
    ],
    deps = [],)

cc_binary(
    name = "nvme_example",
    srcs = [
        "example/nvme_init.cpp",
    ],
    copts = [
        "-Iinclude",
        "-Isrc"
    ],
    deps = [
        ":nvme"
    ],)

cc_binary(
    name = "mini_nvme_unittest",
    srcs = [
        "test/mini_nvme_unittest.cpp",
    ],
    copts = [
        "-Iinclude",
        "-Isrc"
    ],
    deps = [
        ":nvme",
        "@com_google_googletest//:gtest",
    ],)
