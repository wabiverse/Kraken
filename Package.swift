// swift-tools-version: 5.9
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
    .library(
      name: "KrakenKit",
      targets: ["KrakenKit"]
    ),
    .library(
      name: "KrakenLib",
      targets: ["KrakenLib"]
    ),
    .library(
      name: "KrakenUI",
      targets: ["KrakenUI"]
    ),
    .executable(
      name: "Kraken",
      targets: ["Kraken"]
    ),
  ],

  // --- 🦄 Package Dependencies. ---
  dependencies: [
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", from: "23.11.32"),
    .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.9"),
    .package(url: "https://github.com/furby-tm/swift-cross-ui", revision: "a26353e")
  ],

  // --- 🎯 Package Targets. ---
  targets: [
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

    .target(
      name: "KrakenUI",
      dependencies: [
        .target(name: "KrakenKit"),
        .product(
          name: "SwiftCrossUI",
          package: "swift-cross-ui",
          moduleAliases: ["SwiftCrossUI": "SwiftUI"],
          condition: .when(platforms: [.linux, .windows])
        ),
        .product(
          name: "GtkBackend",
          package: "swift-cross-ui",
          condition: .when(platforms: [.linux, .windows])
        ),
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
        .target(name: "KrakenUI"),
      ],
      resources: [
        .process("Resources")
      ],
      swiftSettings: [
        .interoperabilityMode(.Cxx)
      ]
    ),
  ],
  cxxLanguageStandard: .cxx17
)
