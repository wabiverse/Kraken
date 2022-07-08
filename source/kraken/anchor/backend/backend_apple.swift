/**
 *  backend_apple.swift
 *  Anchor Interface for Kraken on macOS.
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