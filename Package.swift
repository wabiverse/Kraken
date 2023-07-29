// swift-tools-version: 5.9
import PackageDescription

#if DEBUG
  let pixarHeadersPath = "../build_darwin_debug/bin/include"
#else
  let pixarHeadersPath = "../build_darwin_release/bin/include"
#endif

// ----------------------------------------------------------
// :: :  💫 The Animation Foundation  :                    ::
// ----------------------------------------------------------
let package = Package(
  name: "AnimationFoundation",

  // --- 📦 Package Products. ---
  products: [
    .library(name: "PixarUSD",
             targets: ["PixarUSD"]),

    .executable(name: "Kraken",
                targets: ["Kraken"]),
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    // -----------------------------------------------------------
    // :: :  🦄 Pixar - Universal Scene Description(USD)  :     ::
    // -----------------------------------------------------------
    .target(name: "PixarUSD",
            path: ".",
            exclude: [
              "./.build",
              "./.github",
              "./.vscode",
              "./build_files",
              "./doc",
              "./extern",
              "./intern",
              "./release",
              "./source",
              "./test",
              "./Tests",
              "./wabi/base/js/rapidjson/msinttypes",
              "./wabi/base/gf/range.template.cpp",
              "./wabi/base/gf/matrix.template.cpp",
              "./wabi/base/gf/matrix2.template.cpp",
              "./wabi/base/gf/matrix3.template.cpp",
              "./wabi/base/gf/matrix4.template.cpp",
              "./wabi/base/gf/wrapMatrix.template.cpp",
              "./wabi/base/gf/wrapMatrix2.template.cpp",
              "./wabi/base/gf/wrapMatrix3.template.cpp",
              "./wabi/base/gf/wrapMatrix4.template.cpp",
              "./wabi/base/gf/vec.template.cpp",
            ],
            publicHeadersPath: "wabi",
            cxxSettings: [
              .unsafeFlags([
                "-I\(pixarHeadersPath)",
                "-I../lib/apple_darwin_arm64/python/include/python3.10",
                "-I/opt/homebrew/include"
              ]),
              .define("WITH_PYTHON", to: "1"),
              .define("WITH_METAL", to: "1"),
              .define("WITH_USD", to: "1"),
              .define("WITH_TBB", to: "1"),
              .define("WITH_BOOST", to: "1"),
              .define("WITH_PIXAR_USDVIEW", to: "1")
            ],
            swiftSettings: [
              .interoperabilityMode(.Cxx),
              .unsafeFlags([
                "-I\(pixarHeadersPath)",
                "-I../lib/apple_darwin_arm64/python/include/python3.10",
                "-I/opt/homebrew/include"
              ]),
              .define("WITH_PYTHON"),
              .define("WITH_METAL"),
              .define("WITH_USD"),
              .define("WITH_TBB"),
              .define("WITH_BOOST"),
              .define("WITH_PIXAR_USDVIEW")
            ]),

    // ---------------------------------------------------------
    // :: :  🐙 Kraken - The Animation Platform Suite.  :     ::
    // ---------------------------------------------------------
    .executableTarget(name: "Kraken",
                      path: "source/creator",
            cxxSettings: [
              .unsafeFlags([
                "-I\(pixarHeadersPath)",
                "-I../lib/apple_darwin_arm64/python/include/python3.10",
                "-I/opt/homebrew/include",
              ]),
              .define("WITH_PYTHON", to: "1"),
              .define("WITH_METAL", to: "1"),
              .define("WITH_USD", to: "1"),
              .define("WITH_TBB", to: "1"),
              .define("WITH_BOOST", to: "1"),
              .define("WITH_PIXAR_USDVIEW", to: "1"),
            ],
            swiftSettings: [
              .interoperabilityMode(.Cxx),
              .unsafeFlags([
                "-I\(pixarHeadersPath)",
                "-I../lib/apple_darwin_arm64/python/include/python3.10",
                "-I/opt/homebrew/include",
              ]),
              .define("WITH_PYTHON"),
              .define("WITH_METAL"),
              .define("WITH_USD"),
              .define("WITH_TBB"),
              .define("WITH_BOOST"),
              .define("WITH_PIXAR_USDVIEW"),
            ]),
  ],
  cLanguageStandard: .c90,
  cxxLanguageStandard: .cxx20
)
