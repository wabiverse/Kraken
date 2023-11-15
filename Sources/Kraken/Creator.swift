/* --------------------------------------------------------------
 * :: :  K  R  A  K  E  N  :                                   ::
 * --------------------------------------------------------------
 * @wabistudios :: multiverse :: kraken
 *
 * CREDITS.
 *
 * T.Furby                 @furby-tm       <devs@wabi.foundation>
 *
 *            Copyright (C) 2023 Wabi Animation Studios, Ltd. Co.
 *                                           All Rights Reserved.
 * --------------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * -------------------------------------------------------------- */

import Foundation
import Pixar

/* --- xxx --- */

@main
enum Creator
{
  static func main()
  {
    /* Using Pixar's USD from Swift. */
    let cwd = Pixar.Arch.getCwd()
    let exePath = Pixar.Arch.getExecutablePath()
    let isMain = Pixar.Arch.isMainThread()
    let threadId = Pixar.Arch.getMainThreadId()
    let pageSize = Pixar.Arch.getPageSize()

    PXRMSG.Log.point("Current working directory", to: cwd)
    PXRMSG.Log.point("Path to running executable", to: exePath)
    PXRMSG.Log.point("Are we on the main thread?", to: isMain)
    PXRMSG.Log.point("The id of the main thread", to: threadId)
    PXRMSG.Log.point("System memory paging size", to: pageSize)

    print("Kraken launched. Will exit.")
  }
}

/* --- xxx --- */



