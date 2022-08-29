set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hgiMetal)

if(NOT ${WITH_GPU_SUPPORT} OR NOT ${WITH_METAL})
  message(STATUS "Skipping ${WABI_PACKAGE} because WITH_GPU_SUPPORT or WITH_METAL is OFF")
  return()
endif()

FIND_LIBRARY(METAL_LIBRARY Metal)
FIND_LIBRARY(APPUIKIT_LIBRARY AppKit)

wabi_library(hgiMetal
    LIBRARIES
        arch
        hgi
        tf
        trace
        ${METAL_LIBRARY}
        ${APPUIKIT_LIBRARY}
        "-framework Foundation"
        "-framework Metal"
        "-framework QuartzCore"

    PUBLIC_HEADERS
        api.h
        capabilities.h
        computeCmds.h
        computePipeline.h
        diagnostic.h
        hgi.h
        texture.h
        blitCmds.h
        buffer.h
        graphicsCmds.h
        graphicsPipeline.h
        resourceBindings.h
        sampler.h
        shaderFunction.h
        shaderProgram.h
        Foundation/Foundation.hpp
        Foundation/NSArray.hpp
        Foundation/NSAutoreleasePool.hpp
        Foundation/NSBundle.hpp
        Foundation/NSData.hpp
        Foundation/NSDate.hpp
        Foundation/NSDefines.hpp
        Foundation/NSDictionary.hpp
        Foundation/NSEnumerator.hpp
        Foundation/NSError.hpp
        Foundation/NSLock.hpp
        Foundation/NSNotification.hpp
        Foundation/NSNumber.hpp
        Foundation/NSObjCRuntime.hpp
        Foundation/NSObject.hpp
        Foundation/NSPrivate.hpp
        Foundation/NSProcessInfo.hpp
        Foundation/NSRange.hpp
        Foundation/NSSet.hpp
        Foundation/NSSharedPtr.hpp
        Foundation/NSString.hpp
        Foundation/NSTypes.hpp
        Foundation/NSURL.hpp
        Metal/MTLAccelerationStructure.hpp
        Metal/MTLAccelerationStructureCommandEncoder.hpp
        Metal/MTLAccelerationStructureTypes.hpp
        Metal/MTLArgument.hpp
        Metal/MTLArgumentEncoder.hpp
        Metal/MTLBinaryArchive.hpp
        Metal/MTLBlitCommandEncoder.hpp
        Metal/MTLBlitPass.hpp
        Metal/MTLBuffer.hpp
        Metal/MTLCaptureManager.hpp
        Metal/MTLCaptureScope.hpp
        Metal/MTLCommandBuffer.hpp
        Metal/MTLCommandEncoder.hpp
        Metal/MTLCommandQueue.hpp
        Metal/MTLComputeCommandEncoder.hpp
        Metal/MTLComputePass.hpp
        Metal/MTLComputePipeline.hpp
        Metal/MTLCounters.hpp
        Metal/MTLDefines.hpp
        Metal/MTLDepthStencil.hpp
        Metal/MTLDevice.hpp
        Metal/MTLDrawable.hpp
        Metal/MTLDynamicLibrary.hpp
        Metal/MTLEvent.hpp
        Metal/MTLFence.hpp
        Metal/MTLFunctionConstantValues.hpp
        Metal/MTLFunctionDescriptor.hpp
        Metal/MTLFunctionHandle.hpp
        Metal/MTLFunctionLog.hpp
        Metal/MTLFunctionStitching.hpp
        Metal/MTLHeaderBridge.hpp
        Metal/MTLHeap.hpp
        Metal/MTLIOCommandBuffer.hpp
        Metal/MTLIOCommandQueue.hpp
        Metal/MTLIOCompressor.hpp
        Metal/MTLIndirectCommandBuffer.hpp
        Metal/MTLIndirectCommandEncoder.hpp
        Metal/MTLIntersectionFunctionTable.hpp
        Metal/MTLLibrary.hpp
        Metal/MTLLinkedFunctions.hpp
        Metal/MTLParallelRenderCommandEncoder.hpp
        Metal/MTLPipeline.hpp
        Metal/MTLPixelFormat.hpp
        Metal/MTLPrivate.hpp
        Metal/MTLRasterizationRate.hpp
        Metal/MTLRenderCommandEncoder.hpp
        Metal/MTLRenderPass.hpp
        Metal/MTLRenderPipeline.hpp
        Metal/MTLResource.hpp
        Metal/MTLResourceStateCommandEncoder.hpp
        Metal/MTLResourceStatePass.hpp
        Metal/MTLSampler.hpp
        Metal/MTLStageInputOutputDescriptor.hpp
        Metal/MTLTexture.hpp
        Metal/MTLTypes.hpp
        Metal/MTLVertexDescriptor.hpp
        Metal/MTLVisibleFunctionTable.hpp
        Metal/Metal.hpp
        QuartzCore/CADefines.hpp
        QuartzCore/CAMetalDrawable.hpp
        QuartzCore/CAMetalLayer.hpp
        QuartzCore/CAPrivate.hpp
        QuartzCore/QuartzCore.hpp

    PRIVATE_HEADERS
        conversions.h
        shaderGenerator.h
        shaderSection.h

    CPPFILES
        blitCmds.mm
        buffer.mm
        capabilities.mm
        computePipeline.mm
        computeCmds.mm
        conversions.mm
        diagnostic.mm
        graphicsCmds.mm
        graphicsPipeline.mm
        hgi.mm
        metal.cpp
        resourceBindings.mm
        sampler.mm
        shaderFunction.mm
        shaderGenerator.mm
        shaderProgram.mm
        shaderSection.mm
        texture.mm

    RESOURCE_FILES
        resources/plugInfo.json
)
