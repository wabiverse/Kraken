// swift-tools-version: 5.9
import PackageDescription

let package = Package(
  name: "WabiFoundation",
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
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", from: "23.11.21"),
    .package(url: "https://github.com/apple/swift-llbuild.git", revision: "bc3ffd5"),
    .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.9"),
    .package(url: "https://github.com/stackotter/swift-cross-ui", revision: "dd09e61")
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    .target(name: "KrakenKit"),

    .target(
      name: "KrakenUI",
      dependencies: [
        .product(name: "PixarUSD", package: "SwiftUSD"),
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
        .product(name: "PixarUSD", package: "SwiftUSD"),
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
