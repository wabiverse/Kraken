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

/**
 * A manager for validating and managing the
 * stage files used by Kraken for USD operations. */
public struct StageManager
{
  /** The shared instance of the stage manager singleton. */
  public static let shared = StageManager()

  private init()
  {}

  /** The underlying file manager for file ops. */
  private let fileManager = FileManager.default

  /**
   * Validates the usd file at the given path.
   *
   * Creates a new file if it does not exist.
   * If the file is not readable or writable,
   * it will be removed and recreated with the
   * correct file permissions.
   *
   * - Parameter file: The usd filepath to validate. */
  public func validate(atPath file: String)
  {
    guard fileManager.fileExists(atPath: file),
          fileManager.isReadableFile(atPath: file),
          fileManager.isWritableFile(atPath: file)
    else
    {
      try? fileManager.removeItem(atPath: file)

      fileManager.createFile(
        atPath: file,
        contents: (try? String(
          contentsOfFile: file
        ).data(using: .utf8)) ?? "".data(using: .utf8),
        attributes: [.posixPermissions: 0o777]
      )

      return
    }
  }

  /**
   * Saves (.usda) file contents and syncs modifications
   * to the stage in real time.
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
  public func save(contentsOfFile stageData: String, atPath file: String, stage: inout UsdStageRefPtr)
  {
    validate(atPath: file)

    let path = URL(fileURLWithPath: file)

    do
    {
      try stageData.write(to: path, atomically: true, encoding: .utf8)
    }
    catch
    {
      print("Error writing to file: \(path.absoluteString)")
    }

    stage.pointee.Reload()

    stage.save()
  }
}
