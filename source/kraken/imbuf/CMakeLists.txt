#imbuf relies on ocio.
add_subdirectory(intern/opencolorio)

set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../krakernel
  ../kraklib
  ../gpu
  ../universe
  ../luxo
  ../editors/include
  ../../../intern/guardedalloc
)

set(SRC
  intern/colormanagement.c
  intern/filetype.c
  intern/module.c

  IMB_colormanagement.h
  IMB_imbuf.h
  
  intern/IMB_colormanagement_intern.h
  intern/IMB_filetype.h
)

add_definitions(-DWITH_OCIO)
  
set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${PTHREADS_INCLUDE_DIR}
  ${JPEG_INCLUDE_DIR}
  ${PNG_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIR}
  ${TIFF_INCLUDE_DIR}
  ${OPENJPEG_INCLUDE_DIRS}
  ${FFMPEG_INCLUDE_DIRS}
  ${WEBP_INCLUDE_DIRS}
  ${ILMBASE_INCLUDES}/Imath
  ${OPENEXR_INCLUDE_DIR}
)

list(APPEND LIB
  maelstrom
  maelstrom_static
  kraken_kernel
  kraken_lib
  kraken_intern_guardedalloc
  kraken_intern_opencolorio
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${PTHREADS_LIBRARY}
  ${PNG_LIBRARIES}
  ${JPEG_LIBRARIES}
  ${TIFF_LIBRARY}
  ${FFMPEG_LIBRARIES}
  ${OPENJPEG_LIBRARIES}
  ${WEBP_LIBRARIES}
  ${ALL_IMAGING_LIBS}
)

add_definitions(-DWITH_TIFF)
add_definitions(-DWITH_OPENIMAGEIO)
add_definitions(-DWITH_OPENJPEG ${OPENJPEG_DEFINES})
add_definitions(-DWITH_FFMPEG)
add_definitions(-DWITH_WEBP)
add_definitions(-DWITH_OPENEXR)

list(APPEND INC
  intern/opencolorio
)

if(WIN32)
  list(APPEND INC
    ../../../intern/utfconv
  )
endif()

kraken_add_lib(kraken_imbuf "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_imbuf kraken_intern_opencolorio)