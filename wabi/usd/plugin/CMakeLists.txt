if(WITH_ALEMBIC)
  add_subdirectory(usdAbc)
endif()

if(WITH_DRACO)
  add_subdirectory(usdDraco)
endif()

if(WITH_MATERIALX)
  add_subdirectory(usdMtlx)
endif()

if(WITH_ARNOLD)
  add_subdirectory(ndrArnold)
endif()

if(WITH_CYCLES)
  add_subdirectory(ndrCycles)
endif()

add_subdirectory(sdrOsl)
