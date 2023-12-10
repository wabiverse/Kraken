// swift-tools-version: 5.9
import PackageDescription

let package = Package(
  name: "Metaverse",
  platforms: [
    .macOS(.v12),
    .visionOS(.v1),
    .iOS(.v12),
    .tvOS(.v12),
    .watchOS(.v4),
  ],
  // --- 📦 Package Products. ---
  products: [
    .library(
      name: "KrakenPy",
      targets: ["KrakenPy"]
    ),
    .executable(
      name: "Kraken",
      targets: ["Kraken"]
    ),
  ],

  // --- 🦄 Package Dependencies. ---
  dependencies: [
    .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.8"),
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", from: "23.8.20")
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    .target(
      name: "KrakenPy",
      dependencies: [
        .product(name: "PyTf", package: "SwiftUSD"),
        .product(name: "PyGf", package: "SwiftUSD"),
        .product(name: "PyTrace", package: "SwiftUSD"),
        .product(name: "PyVt", package: "SwiftUSD"),
        .product(name: "PyWork", package: "SwiftUSD"),
        .product(name: "PyPlug", package: "SwiftUSD"),
        .product(name: "PyAr", package: "SwiftUSD"),
        .product(name: "PyKind", package: "SwiftUSD"),
        .product(name: "PySdf", package: "SwiftUSD"),
        .product(name: "PyPcp", package: "SwiftUSD"),
        .product(name: "PyUsd", package: "SwiftUSD")
      ],
      swiftSettings: [
        // needed for SwiftUSD.
        .interoperabilityMode(.Cxx)
      ]
    ),

    .executableTarget(
      name: "Kraken",
      dependencies: [
        .product(name: "Pixar", package: "SwiftUSD")
      ],
      swiftSettings: [
        // needed for SwiftUSD.
        .interoperabilityMode(.Cxx)
      ]
    ),
  ]
)
