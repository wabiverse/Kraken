// swift-tools-version: 5.10
import PackageDescription

let package = Package(
  name: "Kraken",
  platforms: [
    .macOS(.v14),
    .visionOS(.v1),
    .iOS(.v17),
    .tvOS(.v17),
    .watchOS(.v10)
  ],
  // --- 📦 Package Products. ---
  products: [
    // --- 🎨 Editors ---
    .library(
      name: "CosmoEditor",
      targets: ["CodeView", "CosmoEditor"]
    ),
    .library(
      name: "CodeLanguages",
      targets: ["LanguagesBundle", "CodeLanguages"]
    ),
    // --- 🦑 Kraken ---
    .library(
      name: "KrakenKit",
      targets: ["KrakenKit"]
    ),
    .library(
      name: "KrakenLib",
      targets: ["KrakenLib"]
    ),
    .executable(
      name: "Kraken",
      targets: ["Kraken"]
    ),
  ],

  // --- 🦄 Package Dependencies. ---
  dependencies: [
    // .package(url: "https://github.com/stackotter/swift-cross-ui", revision: "f57f7ab"),
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", from: "23.11.42"),
    .package(url: "https://github.com/ChimeHQ/SwiftTreeSitter", revision: "96ad58b"),
    .package(url: "https://github.com/ChimeHQ/TextFormation.git", from: "0.8.2"),
    .package(url: "https://github.com/ChimeHQ/TextStory.git", from: "0.8.0"),
    .package(url: "https://github.com/apple/swift-collections", from: "1.1.0"),
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    // --- 🦑 Kraken ---
    .target(
      name: "KrakenKit",
      dependencies: [
        .product(name: "PixarUSD", package: "SwiftUSD"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx)
      ]
    ),
    .target(
      name: "KrakenLib",
      dependencies: [
        .product(name: "PixarUSD", package: "SwiftUSD"),
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx)
      ]
    ),
    .executableTarget(
      name: "Kraken",
      dependencies: [
        .product(name: "PixarUSD", package: "SwiftUSD"),
        .target(name: "KrakenKit"),
        .target(name: "KrakenLib"),
        .target(name: "CosmoEditor"),
      ],
      resources: [
        .process("Resources")
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx)
      ]
    ),
    // --- 🎨 Editors ---
    .target(
      name: "CosmoEditor",
      dependencies: [
        .target(name: "CodeView"),
        .target(name: "CodeLanguages"),
        .product(name: "TextFormation", package: "TextFormation"),
      ],
      path: "Sources/Editors/Code/Cosmo"
    ),
    .target(
      name: "CodeView",
      dependencies: [
        .product(name: "TextStory", package: "TextStory"),
        .product(name: "Collections", package: "swift-collections"),
      ],
      path: "Sources/Editors/Code/CodeView"
    ),
    .target(
      name: "LanguagesBundle",
      path: "Sources/Editors/Code/LanguagesBundle",
      publicHeadersPath: "include",
      cSettings: [
        .headerSearchPath("TreeSitterC/include"),
        .headerSearchPath("TreeSitterCPP/include"),
        .headerSearchPath("TreeSitterJSDoc/include"),
        .headerSearchPath("TreeSitterJSON/include"),
        .headerSearchPath("TreeSitterPython/include"),
        .headerSearchPath("TreeSitterRust/include"),
        .headerSearchPath("TreeSitterSwift/include"),
        .headerSearchPath("TreeSitterTOML/include"),
        .headerSearchPath("TreeSitterUSD/include"),
      ]
    ),
    .target(
      name: "CodeLanguages",
      dependencies: [
        .target(name: "LanguagesBundle"),
        .product(name: "SwiftTreeSitter", package: "SwiftTreeSitter"),
      ],
      path: "Sources/Editors/Code/Languages",
      resources: [
        .copy("Resources/tree-sitter-c"),
        .copy("Resources/tree-sitter-cpp"),
        .copy("Resources/tree-sitter-jsdoc"),
        .copy("Resources/tree-sitter-json"),
        .copy("Resources/tree-sitter-python"),
        .copy("Resources/tree-sitter-rust"),
        .copy("Resources/tree-sitter-swift"),
        .copy("Resources/tree-sitter-toml"),
        .copy("Resources/tree-sitter-usd"),
      ]
    ),
    .testTarget(
      name: "CodeEditorTests",
      dependencies: [
        .target(name: "CosmoEditor"),
        .target(name: "CodeLanguages"),
      ],
      path: "Tests/Editors/Code"
    ),
  ],
  cxxLanguageStandard: .cxx17
)
