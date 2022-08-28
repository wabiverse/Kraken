/**
 * @RENDER ENGINE
 * The render engine class provides a safe mechanism
 * for embedding a external imaging api within the
 * kraken runtime. Implementing this class for any
 * external or internal api will unify it as apart
 * of the kraken rendering pipeline.
 * 
 * This is the Swift implementation for this class.
 * Additional support will come for cxx and python.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

import MetalKit
import AppKit
import SwiftUI

final class RenderEngine : NSViewRepresentable
{
  func makeCoordinator() -> Coordinator
  {
    Coordinator(self)
  }
    
  func makeNSView(context: NSViewRepresentableContext<MetalView>) -> MTKView
  {
    let gpu = MTLCopyAllDevicesWithObserver(handler: ChaosNotifier().GPUPing).devices.first
    let chaosView = ChaosView(frameRect: CGRect(x: 0, y: 0, width: 200, height: 200), device: gpu)
    return chaosView
  }
  
  func updateNSView(_ nsView: MTKView, context: NSViewRepresentableContext<MetalView>)
  {}
  
  class Coordinator : NSObject, MTKViewDelegate
  {
    var parent: MetalView
    var metalDevice: MTLDevice!
    var metalCommandQueue: MTLCommandQueue!
    
    init(_ parent: MetalView, device: MTLDevice)
    {
      self.parent = parent
      self.metalDevice = device
      self.metalCommandQueue = self.metalDevice.makeCommandQueue()!
      super.init()
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize)
    {}
    
    func draw(in view: MTKView)
    {
      guard let drawable = view.currentDrawable else {
        return
      }
      
      let commandBuffer = metalCommandQueue.makeCommandBuffer()
      
      let rpd = view.currentRenderPassDescriptor
      rpd?.colorAttachments[0].clearColor = MTLClearColorMake(0, 1, 0, 1)
      rpd?.colorAttachments[0].loadAction = .clear
      rpd?.colorAttachments[0].storeAction = .store
      
      let re = commandBuffer?.makeRenderCommandEncoder(descriptor: rpd!)
      re?.endEncoding()
      
      commandBuffer?.present(drawable)
      commandBuffer?.commit()
    }
  }
}