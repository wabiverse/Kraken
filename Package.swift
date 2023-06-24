// swift-tools-version: 5.9
import PackageDescription

#if DEBUG
  let pixarHeadersPath = "../build_darwin_release/bin/debug/include"
#else
  let pixarHeadersPath = "../build_darwin_release/bin/release/include"
#endif

// ----------------------------------------------------------
// :: :  💫 The Animation Foundation  :                    ::
// ----------------------------------------------------------
let package = Package(
  name: "AnimationFoundation",

  // --- 📦 Package Products. ---
  products: [
    .library(name: "Pixar",
             targets: ["Pixar"]),

    .executable(name: "Kraken",
                targets: ["Kraken"]),
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    // -----------------------------------------------------------
    // :: :  🦄 Pixar - Universal Scene Description(USD)  :     ::
    // -----------------------------------------------------------
    .target(name: "Pixar",
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
            ],
            publicHeadersPath: "wabi",
            swiftSettings: [.interoperabilityMode(.Cxx)]),

    // ---------------------------------------------------------
    // :: :  🐙 Kraken - The Animation Platform Suite.  :     ::
    // ---------------------------------------------------------
    .executableTarget(name: "Kraken",
                      path: "source/creator",
                      swiftSettings: [.interoperabilityMode(.Cxx)]),
  ]
)
