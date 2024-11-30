/* --------------------------------------------------------------
 * :: :  K  R  A  K  E  N  :                                   ::
 * --------------------------------------------------------------
 * @wabistudios :: metaverse :: kraken
 *
 * This program is free software; you can redistribute it, and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. Check out
 * the GNU General Public License for more details.
 *
 * You should have received a copy for this software license, the
 * GNU General Public License along with this program; or, if not
 * write to the Free Software Foundation, Inc., to the address of
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *                            Copyright (C) 2023 Wabi Foundation.
 *                                           All Rights Reserved.
 * --------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * -------------------------------------------------------------- */

import CxxStdlib
import Foundation
import PixarUSD
import SwiftUI

public extension Kraken.IO
{
  /**
   * A manager for validating and managing the
   * stage files used by Kraken for USD operations. */
  struct Stage
  {
    /** The shared instance of the stage manager singleton. */
    public static let manager = Stage()

    /** Temp directory for saving user data. */
    public var tmpDir: URL

    /** The date formatter for timestamp metadata. */
    private let formatter: DateFormatter

    /** The underlying file manager for file ops. */
    private let fileManager: FileManager

    private init()
    {
      formatter = DateFormatter()
      formatter.dateFormat = "MM-dd-yyyy HH:mm:ss"

      fileManager = FileManager.default

      tmpDir = fileManager.temporaryDirectory
    }

    /**
     * Validates the usd file at the given path.
     *
     * Creates a new file if it does not exist.
     * If the file is not readable or writable,
     * it will be removed and recreated with the
     * correct file permissions.
     *
     * - Parameter file: The usd filepath to validate. */
    @discardableResult
    public func validate(url fileURL: URL?) -> String
    {
      var isBinary = false
      if Kraken.IO.USD.blacklistedFileExts.contains(fileURL?.pathExtension ?? "")
      {
        isBinary = true
      }

      let filePath = fileURL?.path
        ?? "\(Bundle.main.resourcePath ?? ".")/Untitled.usda"

      guard
        fileManager.fileExists(atPath: filePath),
        fileManager.isReadableFile(atPath: filePath),
        fileManager.isWritableFile(atPath: filePath)
      else
      {
        try? fileManager.removeItem(atPath: filePath)

        if !isBinary
        {
          fileManager.createFile(
            atPath: filePath,
            contents: (try? String(
              contentsOfFile: filePath
            ).data(using: .utf8)) ?? "".data(using: .utf8),
            attributes: [.posixPermissions: 0o777]
          )
        }

        return filePath
      }

      return filePath
    }

    public func getTmpURL() -> URL
    {
      tmpDir.appending(component: "Untitled.usda")
    }

    public func getRandomTmpURL() -> URL
    {
      let uuid = UUID().uuidString.suffix(5)
      let rdmDir = tmpDir.appending(component: uuid)

      return rdmDir.appending(component: "Untitled.usda")
    }

    enum AppSupportDirectory
    {
      case kraken
      case other(String)

      public var rawValue: String
      {
        switch self
        {
          case .kraken: "Kraken"
          case let .other(custom): custom
        }
      }
    }

    func getAppSupportDirectory(for application: AppSupportDirectory) -> URL?
    {
      do
      {
        let appDirectory: URL

        // get the user's application support directory.
        let appSupportURL = try fileManager.url(
          for: .applicationSupportDirectory,
          in: .userDomainMask,
          appropriateFor: nil,
          create: true
        )

        switch application
        {
          case .kraken:
            // append kraken subdirectory.
            appDirectory = appSupportURL.appendingPathComponent("\(application.rawValue)/\(Kraken.version)")
            // create the directory if it doesn't exist.
            if !fileManager.fileExists(atPath: appDirectory.path)
            {
              try fileManager.createDirectory(at: appDirectory, withIntermediateDirectories: true, attributes: nil)
            }
          case let .other(appName):
            // append other subdirectory.
            appDirectory = appSupportURL.appendingPathComponent("\(appName)")
        }

        return appDirectory
      }
      catch
      {
        print("Error accessing or creating app support directory: \(error)")
        return nil
      }
    }

    public func getUserPrefURL() -> URL
    {
      if let userPref = getAppSupportDirectory(for: .kraken)?.appendingPathComponent("config/userpref.usd") {
        return userPref
      }

      return tmpDir.appendingPathComponent("config/userpref.usd")
    }

    public func getStartupURL() -> URL
    {
      tmpDir.appending(component: "Startup.usd")
    }

    public func makeTmp() -> Kraken.IO.USD
    {
      Kraken.IO.USD(fileURL: getTmpURL())
    }

    /**
     * Reloads and saves (.usd) file contents and syncs
     * modifications to the stage in real time.
     *
     * When this method is called, it will validate
     * the file at the given path, write the stage data
     * to the file, and then reload the stage in real
     * time to ensure the changes are reflected.
     *
     * - Parameters:
     *   - stageData: The stage data to save to the file.
     *   - file: The usd filepath to save the data to.
     *   - stage: The stage to save and reload. */
    public func reloadAndSave(stage: inout UsdStageRefPtr)
    {
      stage.reload()
      save(&stage)
    }

    /**
     * Loads and saves (.usda) file contents and syncs
     * modifications to the stage in real time.
     *
     * When this method is called, it will validate
     * the file at the given path, write the stage data
     * to the file, and then reload the stage in real
     * time to ensure the changes are reflected.
     *
     * - Parameters:
     *   - stageData: The stage data to save to the file.
     *   - file: The usd filepath to save the data to.
     *   - stage: The stage to save and reload. */
    public func loadAndSave(stage: inout UsdStageRefPtr)
    {
      stage.pointee.Load()
      save(&stage)
    }

    /**
     * Saves usd file and sets timestamp metadata.
     *
     * - Parameters:
     *   - stage: The stage to save. */
    public func save(_ stage: inout UsdStageRefPtr)
    {
      /* set the timestamp metadata. */
      let time = timestamp()
      let metadata = "Kraken v\(Kraken.version) | \(time)"

      /* set the metadata on the stage. */
      stage.getPseudoRoot().set(doc: metadata)
      stage.save()
    }

    public func timestamp() -> String
    {
      formatter.string(from: Date.now)
    }

    /**
     * Checks if the file is a binary usd file.
     *
     * Currently checks if the file extension is
     * a (.usda) file extension, if it is any other
     * extension, or none, it is considered binary.
     *
     * - Parameters:
     *   - file: The file configuration to check.
     * - Returns: Whether the file is binary or not. */
    public static func isBinary(_ file: ReferenceFileDocumentConfiguration<Kraken.IO.USD>) -> Bool
    {
      file.fileURL?.pathExtension != "usda"
    }
  }
}
