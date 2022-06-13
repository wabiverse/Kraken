######################################################################################### KRAKEN ######

set(KRAKEN_RELEASE_MODE       ON  CACHE BOOL "" FORCE)
set(WITH_BUILDINFO           ON  CACHE BOOL "" FORCE)
set(WITH_LICENSING           ON  CACHE BOOL "" FORCE)

########################################################################################### USD ######

set(WITH_USD                 ON  CACHE BOOL "" FORCE)
set(WITH_PIXAR_AR_BETA       ON  CACHE BOOL "" FORCE)
set(WITH_PIXAR_USDVIEW       ON  CACHE BOOL "" FORCE)
set(WITH_PIXAR_TOOLKIT       ON  CACHE BOOL "" FORCE)
set(WITH_SAFETY_OVER_SPEED   ON  CACHE BOOL "" FORCE)

######################################################################################### HYDRA ######

set(WITH_OPENSUBDIV          ON  CACHE BOOL "" FORCE)
set(WITH_OPENCOLORIO         ON  CACHE BOOL "" FORCE)
set(WITH_ARNOLD              ON  CACHE BOOL "" FORCE)
set(WITH_PRORENDER          OFF  CACHE BOOL "" FORCE)
set(WITH_CYCLES              ON  CACHE BOOL "" FORCE)
set(WITH_RENDERMAN           ON  CACHE BOOL "" FORCE)
set(WITH_PHOENIX             ON  CACHE BOOL "" FORCE)
set(WITH_EMBREE              ON  CACHE BOOL "" FORCE)
if(WIN32)
  set(WITH_DIRECTX           ON  CACHE BOOL "" FORCE)
elseif(APPLE)
  set(WITH_METAL             ON  CACHE BOOL "" FORCE)
  set(WITH_VULKAN           OFF  CACHE BOOL "" FORCE)
  set(WITH_DIRECTX          OFF  CACHE BOOL "" FORCE)
elseif(UNIX)
  set(WITH_VULKAN            ON  CACHE BOOL "" FORCE)
  set(WITH_DIRECTX          OFF  CACHE BOOL "" FORCE)
endif()

################################################################################## FILE FORMATS ######

set(WITH_ALEMBIC             ON  CACHE BOOL "" FORCE)
set(WITH_MATERIALX          OFF  CACHE BOOL "" FORCE)
set(WITH_OSL                 ON  CACHE BOOL "" FORCE)

################################################################################# IMAGE FORMATS ######

set(WITH_IMAGE_OPENEXR       ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_OPENJPEG      ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_TIFF          ON  CACHE BOOL "" FORCE)

######################################################################################## PYTHON ######

set(WITH_PYTHON              ON  CACHE BOOL "" FORCE)
set(WITH_PYTHON_PIXAR        OFF CACHE BOOL "" FORCE)
set(WITH_PYTHON_INSTALL      ON  CACHE BOOL "" FORCE)

####################################################################################### VOLUMES ######

set(WITH_OPENVDB             ON  CACHE BOOL "" FORCE)
set(WITH_OPENVDB_BLOSC       ON  CACHE BOOL "" FORCE)

############################################################################# MEMORY ALLOCATION ######

set(WITH_ASSERT_ABORT        OFF CACHE BOOL "" FORCE)
set(WITH_MEM_MIMALLOC        OFF CACHE BOOL "" FORCE)

################################################################################### COMPRESSION ######

set(WITH_DRACO               ON  CACHE BOOL "" FORCE)

############################################################################### MULTI-THREADING ######

set(WITH_TBB                 ON  CACHE BOOL "" FORCE)

############################################################################### VIRTUAL REALITY ######

set(WITH_XR_OPENXR              ON  CACHE BOOL "" FORCE)

########################################################################################## DOCS ######

if(UNIX AND NOT APPLE)
  set(WITH_DOC_MANPAGE         ON  CACHE BOOL "" FORCE)
endif()