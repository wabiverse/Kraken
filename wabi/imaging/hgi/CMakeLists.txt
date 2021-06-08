set(WABI_PREFIX wabi/imaging)
set(WABI_PACKAGE hgi)

wabi_library(hgi
    LIBRARIES
        gf
        plug
        tf

    PUBLIC_CLASSES
        attachmentDesc
        blitCmds
        buffer
        cmds
        computeCmds
        computePipeline
        graphicsCmds
        graphicsCmdsDesc
        graphicsPipeline
        hgi
        resourceBindings
        sampler
        shaderFunction
        shaderFunctionDesc
        shaderGenerator
        shaderProgram
        shaderSection
        texture
        tokens
        types

    PUBLIC_HEADERS
        api.h
        blitCmdsOps.h
        enums.h
        handle.h
)
