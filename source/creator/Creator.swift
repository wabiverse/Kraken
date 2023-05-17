/* -----------------------------------------------------------
 * :: :  K  R  A  K  E  N  :                                ::
 * -----------------------------------------------------------
 * @wabistudios :: multiverse :: kraken
 *
 * CREDITS.
 *
 * T.Furby              @furby-tm       <devs@wabi.foundation>
 *
 *
 *         Copyright (C) 2023 Wabi Animation Studios, Ltd. Co.
 *                                        All Rights Reserved.
 * -----------------------------------------------------------
 *  . x x x . o o o . x x x . : : : .    o  x  o    . : : : .
 * ----------------------------------------------------------- */


import Foundation
import Python


/* -------------------------------------------------------------------- */
/** 
 * # ``Kraken`` 
 * The Animation Foundation, built upon a simple principle, to give all
 * artists the tools they need to create great things.
 *
 * ## Overview */


/* -------------------------------------------------------------------- */
/* MARK: - Kraken Runtime Creator */
/** 
 * ### Startup
 *
 * Where it all begins.
 * - ``Creator`` */
@main
struct Creator
{
  /* -------------------------------------------------------------------- */
  /* MARK: - Kraken Runtime Initialization */
  /** 
   * ### Main Function
   *
   * #### Kraken's main function responsibilities are:
   * - setup subsystems.
   * - handle arguments.
   * - run #WM_main() event loop,
   *   or exit immediately when running in background-mode. */
  static func main()
  {
    pyInit()
  }

  /* -------------------------------------------------------------------- */
  /* MARK: - Python Runtime Initialization */
  /** 
   * ### Python Initialize
   *
   * #### Initializes the embedded Python runtime. */
  static func pyInit() -> Void
  {
    guard let pystd = Bundle.main.path(forResource: "1.50/python/lib/python3.10", ofType: nil)
    else { return }

    guard let dynload = Bundle.main.path(forResource: "1.50/python/lib/python3.10/lib-dynload", ofType: nil) 
    else { return }

    setenv("PYTHONHOME", pystd, 1)
    setenv("PYTHONPATH", "\(pystd):\(dynload)", 1)
    Py_Initialize()
  }
}
