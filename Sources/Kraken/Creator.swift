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
import PyBundle
import Python

/* --- xxx --- */

@main
enum Creator
{
  static func main()
  {
    /* Embed & init python. */
    PyBundle.shared.pyInit()
    PyBundle.shared.pyInfo()

    /* Using Pixar's USD (Arch) from Swift. */
    let cwd = Pixar.Arch.getCwd()
    let exePath = Pixar.Arch.getExecutablePath()
    let isMain = Pixar.Arch.isMainThread()
    let threadId = Pixar.Arch.getMainThreadId()
    let pageSize = Pixar.Arch.getPageSize()

    PXRMsg.Log.point("Current working directory", to: cwd)
    PXRMsg.Log.point("Path to running executable", to: exePath)
    PXRMsg.Log.point("Are we on the main thread?", to: isMain)
    PXRMsg.Log.point("The id of the main thread", to: threadId)
    PXRMsg.Log.point("System memory paging size", to: pageSize)

    /* Using Pixar's USD (Gf) from Swift (no namespace). */
    let vecA = GfVec2f(1, 2)
    let vecB = GfVec2f(3, 4)
    var vecC = vecA + vecB
    vecC *= 2

    /* Using Pixar's USD (Gf) from Swift (Pixar.Gf namespace). */
    let pxrVecA = Pixar.Gf.Vec2f(1, 2)
    let pxrVecB = Pixar.Gf.Vec2f(3, 4)
    var pxrVecC = pxrVecA + pxrVecB
    pxrVecC *= 2

    PXRMsg.Log.point("The value of vecC", to: pxrVecC)

    /* Using Pixar's USD (Js) from Swift (no namespace). */
    let jsValue = Pixar.Js.Value(true)

    PXRMsg.Log.point("The value of jsValue", to: jsValue.GetBool())

    /* Using Pixar's USD (Js) from Swift (Pixar.Gf namespace). */
    let jsonvalue = JsValue(true)

    PXRMsg.Log.point("The value of jsonvalue", to: jsonvalue.GetBool())

    print("Kraken launched. Will exit.")
  }
}

/* --- xxx --- */
