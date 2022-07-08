/**
 *  backend_apple.swift
 *  Anchor System for Kraken on macOS.
 *
 *  The interface gets auto-generated into
 *  Anchor.h and the Anchor backend for Cocoa
 *  calls directly into this swift class.
 *
 *  This system is in charge of managing the
 *  underlying system time, displays & window
 *  events, and other Operating System bits.
 *
 *  Created by Wabi Animation Studios
 *  Copyright © 2022 Wabi. All rights reserved.
 */

import Foundation
import Darwin
import AppKit

open class AnchorSystemApple : NSObject
{
  @objc 
  public static func sayHello()
  {
    fputs("Hello Anchor.\n", stderr)
  }
}