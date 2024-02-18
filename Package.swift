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
    .package(url: "https://github.com/wabiverse/SwiftUSD.git", from: "23.11.27"),
    .package(url: "https://github.com/apple/swift-llbuild.git", revision: "bc3ffd5"),
    .package(url: "https://github.com/furby-tm/swift-bundler", from: "2.0.9"),
  ] + Arch.OS.pkgDeps(),

  // --- 🎯 Package Targets. ---
  targets: [
    .target(name: "KrakenKit"),

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
      ] + Arch.OS.backend()
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

public enum Arch
{
  case apple
  case linux
  case windows

  public enum OS
  {
    public static func pkgDeps() -> [Package.Dependency]
    {
      #if os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin)
        [
          .package(url: "https://github.com/stackotter/swift-cross-ui", revision: "dd09e61")
        ]
      #else
        []
      #endif
    }

    public static func backend() -> [Target.Dependency]
    {
      #if os(Linux) || os(Android) || os(OpenBSD) || os(FreeBSD) || os(Windows) || os(Cygwin)
        [
          .product(name: "SwiftCrossUI", package: "swift-cross-ui", condition: .when(platforms: [.linux, .windows])),
          .product(name: "GtkBackend", package: "swift-cross-ui", condition: .when(platforms: [.linux, .windows])),
        ]
      #else
        []
      #endif
    }
  }
}
