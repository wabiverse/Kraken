// swift-tools-version: 5.9
import PackageDescription

// -------------------------------------------------------------------------
// :: :  💫 The Open Source Metaverse  :   ::
// -------------------------------------------------------------------------
let package = Package(
  name: "Metaverse",

  // --- 📦 Package Products. ---
  products: [
    .executable(
      name: "Kraken",
      targets: ["Kraken"]
    ),
  ],

  // --- 🦄 Package Dependencies. ---
  dependencies: [
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", branch: "main"),
  ],

  // --- 🎯 Package Targets. ---
  targets: [
    // ---------------------------------------------------------
    // :: :  🐙 Kraken - The Animation Platform Suite.  :     ::
    // ---------------------------------------------------------
    .executableTarget(
      name: "Kraken",
      dependencies: [
        .product(name: "USD", package: "SwiftUSD"),
      ]
    ),
  ]
)
