// swift-tools-version: 5.9
import PackageDescription

// -------------------------------------------------------------------------
// :: :  💫 The Open Source Metaverse  :   ::
// -------------------------------------------------------------------------
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
    .executable(
      name: "Kraken",
      targets: ["Kraken"]
    ),
  ],

  // --- 🦄 Package Dependencies. ---
  dependencies: [
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", from: "23.8.0"),
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    // ---------------------------------------------------------
    // :: :  🐙 Kraken - The Animation Platform Suite.  :     ::
    // ---------------------------------------------------------
    .executableTarget(
      name: "Kraken",
      dependencies: [
        .product(name: "Pixar", package: "SwiftUSD"),
      ]
    ),
  ]
)
