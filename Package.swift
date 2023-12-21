// swift-tools-version: 5.9
import PackageDescription

let package = Package(
  name: "Metaverse",
  platforms: [
    .macOS(.v14),
    .visionOS(.v1),
    .iOS(.v16),
    .tvOS(.v16),
    .watchOS(.v9)
  ],
  // --- 📦 Package Products. ---
  products: [
    .library(
      name: "KrakenKit",
      targets: ["KrakenKit"]
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
    .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.8"),
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", from: "23.11.6")
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    .target(name: "KrakenKit"),

    .target(
      name: "KrakenUI",
      dependencies: [
        .product(name: "Pixar", package: "SwiftUSD"),
        .target(name: "KrakenKit"),
      ],
      swiftSettings: [
        // needed for SwiftUSD.
        .interoperabilityMode(.Cxx)
      ]
    ),

    .executableTarget(
      name: "Kraken",
      dependencies: [
        .product(name: "Pixar", package: "SwiftUSD"),
        .target(name: "KrakenKit"),
        .target(name: "KrakenUI"),
      ],
      resources: [
        .process("Resources")
      ],
      swiftSettings: [
        .define("DEBUG", .when(configuration: .debug)),
        // needed for SwiftUSD.
        .interoperabilityMode(.Cxx)
      ]
    ),
  ]
)
