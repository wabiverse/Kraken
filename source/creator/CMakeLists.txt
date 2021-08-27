setup_libdirs()

if(WIN32)
  if(EXISTS "${CMAKE_BINARY_DIR}/source/creator/Generated Files")
    set(KRAKEN_MICROSOFT_SDK
      # GENERATED WINDOWS SDK WITH KRAKEN PROJECTION
      "${CMAKE_BINARY_DIR}/source/creator/Generated Files"
    )
  endif()
  set(KRAKEN_MICROSOFT
    # FOR PREFIXES LIKE KRAKEN.MICROSOFT.APP
    ../../source
  )
endif()

kraken_include_dirs(
  ${KRAKEN_MICROSOFT_SDK}
  ${KRAKEN_MICROSOFT}
  ../../.
  ../kraken/anchor
  ../kraken/editors/include
  ../kraken/kraklib
  ../kraken/krakernel
  ../kraken/wm
  ../kraken/luxo
  ../kraken/universe
)

set(LIB
  maelstrom
  kraken_wm
  kraken_kernel
  kraken_luxo
  kraken_universe
  kraken_intern_utfconv
  kraken_python
)

list(INSERT LIB 0 ${WABI_MALLOC_LIBRARY})
list(INSERT LIB 0 ${TBB_LIBRARIES})

kraken_include_dirs(${TBB_INCLUDE_DIRS})
kraken_include_dirs(${Boost_INCLUDE_DIRS})
kraken_include_dirs(${PYTHON_INCLUDE_DIR})
kraken_include_dirs(${PTHREADS_INCLUDE_DIR})

if(WITH_TBB)
  if(NOT APPLE)
    # APPLE plaform uses full paths for linking libraries.
    link_directories(${LIBDIR}/tbb/lib)
  endif()
endif()

list(APPEND LIB
  kraken_kernel
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${OPENCOLORIO_LIBRARIES}
  ${OPENIMAGEIO_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

if(WITH_PYTHON)
  kraken_include_dirs(../kraken/python)
  add_definitions(-DWITH_PYTHON)

  if(WITH_PYTHON_SECURITY)
    add_definitions(-DWITH_PYTHON_SECURITY)
  endif()
endif()

if(WITH_XR_OPENXR)
  add_definitions(-DWITH_XR_OPENXR)
endif()

if(WIN32)
  
  # Fixup WinRT generated metadata location.
  kraken_winrt_metadata_hotfix()

  set(WINDOWS_PACKAGE_SRC
    # APPLICATION DEFINITION SOURCE
    ../kraken/microsoft/app/App.cpp
    ../kraken/microsoft/app/App.xaml
    ../kraken/microsoft/app/App.idl

    # MAIN WINDOW SOURCE
    ../kraken/microsoft/mainwindow/MainWindow.cpp
    ../kraken/microsoft/mainwindow/MainWindow.xaml
    ../kraken/microsoft/mainwindow/MainWindow.idl

    # KRAKEN PROJECTION RUNTIME HEADERS
    ../kraken/microsoft/App.xaml.h
    ../kraken/microsoft/MainWindow.h

    # APPLICATION MANIFESTS
    ${CMAKE_BINARY_DIR}/bin/Release/kraken.manifest
    ${CMAKE_SOURCE_DIR}/release/windows/appx/Package.appxManifest

    # DESKTOP ICON & CURSOR RESOURCES
    ${CMAKE_SOURCE_DIR}/release/windows/icons/winkraken.rc

    # DESKTOP & WINDOWS STORE ICONS & BRANDING
    ${ASSET_FILES}
  )

  set_source_files_properties(
    ../kraken/microsoft/app/App.xaml
    PROPERTIES
      VS_XAML_TYPE ApplicationDefinition
  )

  set_source_files_properties(
    ../kraken/microsoft/mainwindow/MainWindow.xaml
    PROPERTIES
      VS_XAML_TYPE Window
  )
endif()

# Setup the exe sources and buildinfo
set(SRC
  creator.cpp
  creator_args.cpp
  ${KRAKEN_MICROSOFT}
  ${WINDOWS_PACKAGE_SRC}

  ${KRAKEN_MICROSOFT_SDK}
  creator.h
)

# MSVC 2010 gives linking errors with the manifest
if(WIN32 AND NOT UNIX)
  add_definitions(
    -DKRAKEN_VER_RC_STR="${KRAKEN_VERSION}"
    -DKRAKEN_VER_RC_1=${KRAKEN_VERSION_MAJOR}
    -DKRAKEN_VER_RC_2=${KRAKEN_VERSION_MINOR}
    -DKRAKEN_VER_RC_3=${KRAKEN_VERSION_PATCH}
    -DKRAKEN_VER_RC_4=0
  )
endif()

if(WITH_BUILDINFO)
  add_definitions(-DWITH_BUILDINFO)
  # --------------------------------------------------------------------------
  # These defines could all be moved into the header below

  # Write strings into a separate header since we can escape C-strings
  # in a way that's not practical when passing defines.
  set(BUILD_PLATFORM "${CMAKE_SYSTEM_NAME}")
  set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
  set(BUILD_CFLAGS "${CMAKE_C_FLAGS}")
  set(BUILD_CXXFLAGS "${CMAKE_CXX_FLAGS}")
  set(BUILD_LINKFLAGS "${PLATFORM_LINKFLAGS}")
  set(BUILD_SYSTEM "CMake")

  if(WITH_COMPILER_SHORT_FILE_MACRO)
    # It's not necessary to include path information
    # about the system building Blender in the executable.
    string(REPLACE "${PLATFORM_CFLAGS_FMACRO_PREFIX_MAP}" " " BUILD_CFLAGS "${BUILD_CFLAGS}")
    string(REPLACE "${PLATFORM_CFLAGS_FMACRO_PREFIX_MAP}" " " BUILD_CXXFLAGS "${BUILD_CXXFLAGS}")
  endif()

  # Use `configure_file` instead of definitions since properly
  # escaping the multiple command line arguments which themselves
  # contain strings and spaces becomes overly error-prone & complicated.
  configure_file(
    "${CMAKE_SOURCE_DIR}/build_files/cmake/buildinfo_static.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/buildinfo_static.h"
    ESCAPE_QUOTES
    @ONLY
  )

  unset(BUILD_PLATFORM)
  unset(BUILD_TYPE)
  unset(BUILD_CFLAGS)
  unset(BUILD_CXXFLAGS)
  unset(BUILD_LINKFLAGS)
  unset(BUILD_SYSTEM)

  # --------------------------------------------------------------------------
  # write header for values that change each build
  # note, generaed file is in build dir's source/creator
  #       except when used as an include path.

  add_definitions(-DWITH_BUILDINFO_HEADER)

  # include the output directory, where the buildinfo.h file is generated
  include_directories(${CMAKE_CURRENT_BINARY_DIR})


  # XXX, ${buildinfo_h_fake} is used here,
  # because we rely on that file being detected as missing
  # every build so that the real header "buildinfo.h" is updated.
  #
  # Keep this until we find a better way to resolve!

  set(buildinfo_h_real "${CMAKE_CURRENT_BINARY_DIR}/buildinfo.h")
  set(buildinfo_h_fake "${CMAKE_CURRENT_BINARY_DIR}/buildinfo.h_fake")

  if(EXISTS ${buildinfo_h_fake})
    message(FATAL_ERROR "File \"${buildinfo_h_fake}\" found, this should never be created, remove!")
  endif()

  # From the cmake documentation "If the output of the custom command is not actually created as a
  # file on disk it should be marked with the SYMBOLIC source file property."
  #
  # Not doing this leads to build warnings for the not generated file on windows when using msbuild
  SET_SOURCE_FILES_PROPERTIES(${buildinfo_h_fake} PROPERTIES SYMBOLIC TRUE)

  # a custom target that is always built
  add_custom_target(buildinfo ALL
    DEPENDS ${buildinfo_h_fake})

  # creates buildinfo.h using cmake script
  add_custom_command(
    OUTPUT
      ${buildinfo_h_fake}  # ensure we always run
      ${buildinfo_h_real}
    COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    # overrides only used when non-empty strings
    -DBUILD_DATE=${BUILDINFO_OVERRIDE_DATE}
    -DBUILD_TIME=${BUILDINFO_OVERRIDE_TIME}
    -P ${CMAKE_SOURCE_DIR}/build_files/cmake/buildinfo.cmake)

  # buildinfo.h is a generated file
  set_source_files_properties(
    ${buildinfo_h_real}
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

  unset(buildinfo_h_real)
  unset(buildinfo_h_fake)

  # add deps below, after adding kraken
  # -------------- done with header values.

  list(APPEND SRC
    buildinfo.cpp
  )

  # make an object library so can load with it in tests
  add_library(buildinfoobj OBJECT buildinfo.cpp)
  add_dependencies(buildinfoobj buildinfo)
endif()

add_cc_flags_custom_test(kraken)

# message(STATUS "Configuring kraken")
if(WITH_PYTHON_MODULE)
  add_definitions(-DWITH_PYTHON_MODULE)

  # creates ./bin/kpy.so which can be imported as a python module.
  #
  # note that 'SHARED' works on Linux and Windows,
  # but not OSX which _must_ be 'MODULE'
  add_library(kraken MODULE ${SRC})
  set_target_properties(
    kraken
    PROPERTIES
      PREFIX ""
      OUTPUT_NAME kpy
      LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin  # only needed on windows
  )

  if(APPLE)
    set_target_properties(kraken PROPERTIES MACOSX_BUNDLE TRUE)
  endif()

  if(WIN32)
    # python modules use this
    set_target_properties(
      kraken
      PROPERTIES
      SUFFIX ".pyd"
    )
  endif()

else()
  add_executable(kraken ${EXETYPE} ${SRC})
  add_dependencies(kraken maelstrom)
  if(WIN32)
    kraken_precompile_headers(kraken pch.cpp pch.h)
    set_target_properties(kraken PROPERTIES 
      VS_WINRT_COMPONENT TRUE
    )
  endif()
endif()

if(WITH_BUILDINFO)
  # explicitly say that the executable depends on the buildinfo
  add_dependencies(kraken buildinfo)
endif()

# Link with Pixar API
# We must collect all the Pokemon, not just
# some of the Pokemon, so we link whole archive
if(WIN32)
  target_link_options(kraken PUBLIC "LINKER:/WHOLEARCHIVE:maelstrom")
elseif(APPLE)
  target_link_libraries(kraken -Wl,-force_load maelstrom)
elseif(UNIX)
  target_link_libraries(kraken "-Wl,--whole-archive $<TARGET_FILE:maelstrom> -Wl,--no-whole-archive ${TBB_LIBRARIES}")
endif()

set(KRAKEN_TEXT_FILES
  ${CMAKE_SOURCE_DIR}/release/text/copyright.txt
  # generate this file
  # ${CMAKE_SOURCE_DIR}/release/text/readme.html
)


# -----------------------------------------------------------------------------
# Platform Specific Var: TARGETDIR_VER

if(UNIX AND NOT APPLE)
  if(WITH_PYTHON_MODULE)
    if(WITH_INSTALL_PORTABLE)
      set(TARGETDIR_VER ${KRAKEN_VERSION})
    else()
      set(TARGETDIR_VER ${PYTHON_SITE_PACKAGES}/${KRAKEN_VERSION})
    endif()
  else()
    if(WITH_INSTALL_PORTABLE)
      set(TARGETDIR_VER ${KRAKEN_VERSION})
    else()
      set(TARGETDIR_VER share/kraken/${KRAKEN_VERSION})
    endif()
  endif()

elseif(WIN32)
  set(TARGETDIR_VER ${KRAKEN_VERSION})

elseif(APPLE)
  if(WITH_PYTHON_MODULE)
    set(TARGETDIR_VER ${KRAKEN_VERSION})
  else()
    set(TARGETDIR_VER Kraken.app/Contents/Resources/${KRAKEN_VERSION})
  endif()
  # Skip relinking on cpack / install
  set_target_properties(kraken PROPERTIES BUILD_WITH_INSTALL_RPATH true)
endif()


if(UNIX)
  set_target_properties(kraken PROPERTIES BUILD_WITH_INSTALL_RPATH true)
  set_target_properties(kraken PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${KRAKEN_VERSION}/lib")
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${KRAKEN_VERSION}/lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)
endif()

# -----------------------------------------------------------------------------
# Install Targets (Generic, All Platforms)

if(UNIX)
  install(
    DIRECTORY
      ${LIBDIR}/bin/
    DESTINATION
      "/usr/local/${TARGETDIR_VER}/bin/."
  )

  install(
    DIRECTORY
      ${LIBDIR}/include/
    DESTINATION
      "/usr/local/${TARGETDIR_VER}/include/."
  )

  install(
    DIRECTORY
      ${LIBDIR}/lib/
    DESTINATION
      "/usr/local/${TARGETDIR_VER}/lib/."
  )

  install(
    DIRECTORY
      ${LIBDIR}/lib64/
    DESTINATION
      "/usr/local/${TARGETDIR_VER}/lib64/."
  )
endif()

if(UNIX)
  install(
    TARGETS
      maelstrom # <- Pixar monolithic
      # ---------------------------------
    DESTINATION
      "/usr/local/${TARGETDIR_VER}/lib/."
  )
else()
  install(
    TARGETS
      maelstrom # <- Pixar monolithic
      # ---------------------------------
    DESTINATION
      "./AppX/."
  )
endif()

if(WIN32)

  file(TO_CMAKE_PATH 
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Preview\\VC\\Redist\\MSVC\\14.30.30423\\x64\\Microsoft.VC142.CRT"
    MICROSOFT_VC142_CRT
  )

  file(TO_CMAKE_PATH 
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Preview\\VC\\Redist\\MSVC\\14.30.30401\\x64\\Microsoft.VC142.CRT"
    MICROSOFT_VC142_APP_CRT
  )

  file(TO_CMAKE_PATH
    $ENV{RMANTREE}
    RENDERMAN_LOCATION
  )

  # important to make a clean  install each time, else old scripts get loaded.
  install(
    CODE
    "file(REMOVE_RECURSE ${TARGETDIR_VER})"
  )

  # important to make a clean  install each time, else old scripts get loaded.
  install(
    CODE
    "file(REMOVE_RECURSE ./AppX/${TARGETDIR_VER})"
  )

  install(
    FILES
      # ARNOLD
      ${LIBDIR}/arnold/bin/AdClmHub_2.0.0.dll
      ${LIBDIR}/arnold/bin/adlmint.dll
      ${LIBDIR}/arnold/bin/AdpSDKCore.dll
      ${LIBDIR}/arnold/bin/AdpSDKUI.dll
      ${LIBDIR}/arnold/bin/AdskLicensingSDK_5.0.1.dll
      ${LIBDIR}/arnold/bin/ai.dll
      ${LIBDIR}/arnold/bin/optix.6.6.0.dll
      ${LIBDIR}/arnold/bin/UPI2.dll
      # RENDERMAN
      ${RENDERMAN_LOCATION}/bin/libpxrcore.dll
      ${RENDERMAN_LOCATION}/bin/libstats.dll
      ${RENDERMAN_LOCATION}/lib/libprman.dll
      # ALEMBIC
      ${LIBDIR}/alembic/bin/Alembic.dll
      ${LIBDIR}/hdf5/bin/hdf5.dll
      ${LIBDIR}/hdf5/bin/hdf5_cpp.dll
      ${LIBDIR}/hdf5/bin/hdf5_hl.dll
      ${LIBDIR}/hdf5/bin/hdf5_hl_cpp.dll
      # DRACO
      ${LIBDIR}/draco/lib/draco.dll
      # OPENEXR
      ${LIBDIR}/openexr/bin/Iex-3_0.dll
      ${LIBDIR}/openexr/bin/IlmThread-3_0.dll
      ${LIBDIR}/openexr/bin/Imath-3_0.dll
      ${LIBDIR}/openexr/bin/OpenEXR-3_0.dll
      ${LIBDIR}/openexr/bin/OpenEXRUtil-3_0.dll
      # EMBREE
      ${LIBDIR}/embree/bin/embree3.dll
      # BOOST
      ${LIBDIR}/boost/lib/boost_atomic-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_chrono-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_date_time-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_iostreams-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_numpy39-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_program_options-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_regex-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_system-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_thread-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_python39-${BOOST_LIBRARY_SUFFIX}.dll
      ${LIBDIR}/boost/lib/boost_filesystem-${BOOST_LIBRARY_SUFFIX}.dll
      # OSL
      ${LIBDIR}/osl/bin/oslcomp.dll
      ${LIBDIR}/osl/bin/oslexec.dll
      ${LIBDIR}/osl/bin/oslnoise.dll
      ${LIBDIR}/osl/bin/oslquery.dll
      # OPENCOLORIO
      ${LIBDIR}/opencolorio/bin/OpenColorIO_2_0.dll
      # OPENIMAGEIO
      ${LIBDIR}/OpenImageIO/bin/OpenImageIO.dll
      ${LIBDIR}/OpenImageIO/bin/OpenImageIO_Util.dll
      # OPENIMAGEDENOISE
      ${LIBDIR}/oidn/bin/OpenImageDenoise.dll
      # PTEX
      ${LIBDIR}/ptex/bin/Ptex.dll
      # BLOSC
      ${LIBDIR}/blosc/bin/blosc.dll
      # TIFF
      ${LIBDIR}/tiff/bin/tiff.dll
      # PNG
      ${LIBDIR}/png/bin/libpng16.dll
      # JPEG
      ${LIBDIR}/jpeg/bin/jpeg62.dll
      ${LIBDIR}/jpeg/bin/turbojpeg.dll
      # TBB
      ${LIBDIR}/tbb/bin/tbb.dll
      ${LIBDIR}/tbb/bin/tbbmalloc.dll
      ${LIBDIR}/tbb/bin/tbbmalloc_proxy.dll
      # ZLIB
      ${LIBDIR}/zlib/bin/zlib.dll
      # PYTHON
      ${LIBDIR}/python/39/bin/python3.dll
      ${LIBDIR}/python/39/bin/python39.dll
      # OPENVDB
      ${LIBDIR}/openvdb/bin/openvdb.dll
      # MICROSOFT CRT
      ${MICROSOFT_VC142_CRT}/concrt140.dll
      ${MICROSOFT_VC142_CRT}/msvcp140.dll
      ${MICROSOFT_VC142_CRT}/msvcp140_1.dll
      ${MICROSOFT_VC142_CRT}/msvcp140_2.dll
      ${MICROSOFT_VC142_CRT}/msvcp140_atomic_wait.dll
      ${MICROSOFT_VC142_CRT}/msvcp140_codecvt_ids.dll
      ${MICROSOFT_VC142_CRT}/vccorlib140.dll
      ${MICROSOFT_VC142_CRT}/vcruntime140.dll
      ${MICROSOFT_VC142_CRT}/vcruntime140_1.dll
      # MICROSOFT APP CRT ??
      ${MICROSOFT_VC142_APP_CRT}/concrt140_app.dll
      ${MICROSOFT_VC142_APP_CRT}/msvcp140_app.dll
      ${MICROSOFT_VC142_APP_CRT}/msvcp140_1_app.dll
      ${MICROSOFT_VC142_APP_CRT}/msvcp140_2_app.dll
      ${MICROSOFT_VC142_APP_CRT}/msvcp140_atomic_wait_app.dll
      ${MICROSOFT_VC142_APP_CRT}/msvcp140_codecvt_ids_app.dll
      ${MICROSOFT_VC142_APP_CRT}/vccorlib140_app.dll
      ${MICROSOFT_VC142_APP_CRT}/vcruntime140_app.dll
      ${MICROSOFT_VC142_APP_CRT}/vcruntime140_1_app.dll

    # Install all dependent .dlls to MSIX Appx Package.
    DESTINATION
      "./AppX/."
  )
endif()

if(WITH_PYTHON)
  # install(CODE "message(\"copying kraken scripts...\")")

  # exclude addons_contrib if release
  if("${KRAKEN_VERSION_CYCLE}" STREQUAL "release" OR
     "${KRAKEN_VERSION_CYCLE}" STREQUAL "rc")
    set(ADDON_EXCLUDE_CONDITIONAL "addons_contrib/*")
  else()
    set(ADDON_EXCLUDE_CONDITIONAL "_addons_contrib/*")  # dummy, wont do anything
  endif()

  # install(
  #   DIRECTORY ${CMAKE_SOURCE_DIR}/release/scripts
  #   DESTINATION ${TARGETDIR_VER}
  #   PATTERN ".git" EXCLUDE
  #   PATTERN ".gitignore" EXCLUDE
  #   PATTERN ".arcconfig" EXCLUDE
  #   PATTERN "__pycache__" EXCLUDE
  #   PATTERN "${ADDON_EXCLUDE_CONDITIONAL}" EXCLUDE
  #   PATTERN "${FREESTYLE_EXCLUDE_CONDITIONAL}" EXCLUDE
  # )

  unset(ADDON_EXCLUDE_CONDITIONAL)
  unset(FREESTYLE_EXCLUDE_CONDITIONAL)
endif()

if(WIN32)
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/fonts
    DESTINATION ${TARGETDIR_VER}/datafiles
  )
else()
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/fonts
    DESTINATION "/usr/local/${TARGETDIR_VER}/datafiles"
  )
endif()

# color management
# if(WITH_OPENCOLORIO)
#   install(
#     DIRECTORY ${CMAKE_SOURCE_DIR}/release/datafiles/colormanagement
#     DESTINATION ${TARGETDIR_VER}/datafiles
#   )
# endif()

# helpful tip when using make
if("${CMAKE_GENERATOR}" MATCHES ".*Makefiles.*")
  # message after building.
  add_custom_command(
    TARGET kraken POST_BUILD MAIN_DEPENDENCY kraken
    COMMAND ${CMAKE_COMMAND} -E
            echo 'now run: \"make install\" to copy runtime files and scripts to ${TARGETDIR_VER}'
  )
endif()


# -----------------------------------------------------------------------------
# Install Targets (Platform Specific)

if(UNIX AND NOT APPLE)

  if(NOT WITH_PYTHON_MODULE)
    if(WITH_DOC_MANPAGE)
      add_custom_target(
        kraken_man_page ALL
        COMMAND ${CMAKE_SOURCE_DIR}/doc/manpage/kraken.1.py
                ${EXECUTABLE_OUTPUT_PATH}/kraken
                ${CMAKE_CURRENT_BINARY_DIR}/kraken.1)
      add_dependencies(kraken_man_page kraken)
    endif()
  endif()

  # there are a few differences between portable and system install
  if(WITH_PYTHON_MODULE)
    if(WITH_INSTALL_PORTABLE)
      install(
        TARGETS kraken
        DESTINATION "/usr/local/${TARGETDIR_VER}/bin/."
      )
    else()
      install(
        TARGETS kraken
        LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}
      )
    endif()
    # none of the other files are needed currently
  elseif(WITH_INSTALL_PORTABLE)
    install(
      TARGETS kraken
      DESTINATION "/usr/local/${TARGETDIR_VER}/bin/."
    )

    if(WITH_DOC_MANPAGE)
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/kraken.1
        DESTINATION "/usr/local/${TARGETDIR_VER}/bin/."
      )
    endif()
    install(
      FILES
        ${CMAKE_SOURCE_DIR}/release/freedesktop/kraken.desktop
        ${CMAKE_SOURCE_DIR}/release/freedesktop/icons/scalable/apps/kraken.svg
        ${CMAKE_SOURCE_DIR}/release/freedesktop/icons/symbolic/apps/kraken-symbolic.svg
      DESTINATION "/usr/local/${TARGETDIR_VER}/bin/."
    )

    install(
      PROGRAMS
      ${CMAKE_SOURCE_DIR}/release/bin/kraken-thumbnailer.py
      DESTINATION "/usr/local/${TARGETDIR_VER}/bin/."
    )

    set(KRAKEN_TEXT_FILES_DESTINATION "/usr/local/${TARGETDIR_VER}/bin/.")
  else()
    # main kraken binary
    install(
      TARGETS kraken
      DESTINATION "/usr/local/${TARGETDIR_VER}/bin/."
    )
    if(WITH_DOC_MANPAGE)
      # manpage only with 'kraken' binary
      install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/kraken.1
        DESTINATION share/man/man1
      )
    endif()

    # misc files
    install(
      FILES ${CMAKE_SOURCE_DIR}/release/freedesktop/kraken.desktop
      DESTINATION share/applications
    )
    install(
      FILES ${CMAKE_SOURCE_DIR}/release/freedesktop/icons/scalable/apps/kraken.svg
      DESTINATION share/icons/hicolor/scalable/apps
    )
    install(
      FILES ${CMAKE_SOURCE_DIR}/release/freedesktop/icons/symbolic/apps/kraken-symbolic.svg
      DESTINATION share/icons/hicolor/symbolic/apps
    )
    install(
      PROGRAMS ${CMAKE_SOURCE_DIR}/release/bin/kraken-thumbnailer.py
      DESTINATION "/usr/local/${TARGETDIR_VER}/bin/."
    )
    set(KRAKEN_TEXT_FILES_DESTINATION share/doc/kraken)
  endif()

  if(WITH_PYTHON)
    if(WITH_PYTHON_INSTALL)

      install(
        PROGRAMS ${PYTHON_EXECUTABLE}
        DESTINATION "/usr/local/${TARGETDIR_VER}/python/bin/."
      )

      # on some platforms (like openSUSE) Python is linked
      # to be used from lib64 folder.
      # determine this from Python's libraries path
      #
      # ugh, its possible 'lib64' is just a symlink to 'lib' which causes incorrect use of 'lib64'
      get_filename_component(_pypath_real ${PYTHON_LIBPATH} REALPATH)
      if(${_pypath_real} MATCHES "lib64$")
        set(_target_LIB "lib64")
      else()
        set(_target_LIB "lib")
      endif()
      unset(_pypath_real)

      set(PYTHON_VERSION ${Python_VERSION_MAJOR}.${Python_VERSION_MINOR})

      if(NOT EXISTS ${PYTHON_LIBPATH}/python${PYTHON_VERSION})
        set(PYTHON_LIBPATH ${PYTHON_LIBPATH}/..)
      endif()

      # Copy the systems python into the install directory
      # install(CODE "message(\"copying a subset of the systems python...\")")
      install(
        DIRECTORY ${PYTHON_LIBPATH}/python${PYTHON_VERSION}
        DESTINATION "/usr/local/${TARGETDIR_VER}/python/${_target_LIB}"
        PATTERN "__pycache__" EXCLUDE               # * any cache *
        PATTERN "config-${PYTHON_VERSION}m/*.a" EXCLUDE  # static lib
        PATTERN "lib2to3" EXCLUDE                   # ./lib2to3
        PATTERN "site-packages/*" EXCLUDE           # ./site-packages/*
        PATTERN "tkinter" EXCLUDE                   # ./tkinter
        PATTERN "lib-dynload/_tkinter.*" EXCLUDE    # ./lib-dynload/_tkinter.co
        PATTERN "idlelib" EXCLUDE                   # ./idlelib
        PATTERN "test" EXCLUDE                      # ./test
        PATTERN "turtledemo" EXCLUDE                # ./turtledemo
        PATTERN "turtle.py" EXCLUDE                 # ./turtle.py
        PATTERN "wininst*.exe" EXCLUDE              # from distutils, avoid malware false positive
      )

      # Needed for distutils/pip
      # get the last part of the include dir, will be 'python{version}{abiflag}',
      get_filename_component(_py_inc_suffix ${PYTHON_INCLUDE_DIR} NAME)
      install(
        FILES ${PYTHON_INCLUDE_DIR}/pyconfig.h
        DESTINATION "/usr/local/${TARGETDIR_VER}/python/include/${_py_inc_suffix}"
      )
      unset(_py_inc_suffix)

      set(PYTHON_NUMPY_PATH /usr/lib/python3/dist-packages)

      if(EXISTS ${PYTHON_NUMPY_PATH}/numpy)
        # Install to the same directory as the source, so debian-like
        # distros are happy with their policy.
        set(_suffix "site-packages")
        if(${PYTHON_NUMPY_PATH} MATCHES "dist-packages")
          set(_suffix "dist-packages")
        endif()
        install(
          DIRECTORY ${PYTHON_NUMPY_PATH}/numpy
          DESTINATION "/usr/local/${TARGETDIR_VER}/python/${_target_LIB}/python${PYTHON_VERSION}/${_suffix}"
          PATTERN ".svn" EXCLUDE
          PATTERN "__pycache__" EXCLUDE           # * any cache *
          PATTERN "*.pyc" EXCLUDE                 # * any cache *
          PATTERN "*.pyo" EXCLUDE                 # * any cache *
          PATTERN "oldnumeric" EXCLUDE            # ./oldnumeric
          PATTERN "doc" EXCLUDE                   # ./doc
          PATTERN "tests" EXCLUDE                 # ./tests
          PATTERN "f2py" EXCLUDE                  # ./f2py - fortran/python interface code, not for kraken.
          PATTERN "include" EXCLUDE               # include dirs all over, we wont use NumPy/CAPI
          PATTERN "*.h" EXCLUDE                   # some includes are not in include dirs
          PATTERN "*.a" EXCLUDE                   # ./core/lib/libnpymath.a - for linking, we dont need.
        )
        unset(_suffix)
      endif()

      set(PYTHON_REQUESTS_PATH /usr/lib/python3/dist-packages)

      # Copy requests, we need to generalize site-packages
      if(EXISTS ${PYTHON_REQUESTS_PATH})
        set(_suffix "site-packages")
        if(${PYTHON_REQUESTS_PATH} MATCHES "dist-packages")
          set(_suffix "dist-packages")
        endif()
        install(
          DIRECTORY ${PYTHON_REQUESTS_PATH}/requests
          DESTINATION "/usr/local/${TARGETDIR_VER}/python/${_target_LIB}/python${PYTHON_VERSION}/${_suffix}"
          PATTERN ".svn" EXCLUDE
          PATTERN "__pycache__" EXCLUDE           # * any cache *
          PATTERN "*.pyc" EXCLUDE                 # * any cache *
          PATTERN "*.pyo" EXCLUDE                 # * any cache *
        )
        # On some platforms requests does have extra dependencies.
        set(_requests_deps "certifi" "chardet" "idna" "urllib3")
        foreach(_requests_dep ${_requests_deps})
          if(EXISTS ${PYTHON_REQUESTS_PATH}/${_requests_dep})
            install(
              DIRECTORY ${PYTHON_REQUESTS_PATH}/${_requests_dep}
              DESTINATION "/usr/local/${TARGETDIR_VER}/python/${_target_LIB}/python${PYTHON_VERSION}/${_suffix}"
              PATTERN ".svn" EXCLUDE
              PATTERN "__pycache__" EXCLUDE           # * any cache *
              PATTERN "*.pyc" EXCLUDE                 # * any cache *
              PATTERN "*.pyo" EXCLUDE                 # * any cache *
            )
          endif()
        endforeach()
        if(EXISTS ${PYTHON_REQUESTS_PATH}/six.py)
          install(
            FILES ${PYTHON_REQUESTS_PATH}/six.py
            DESTINATION "/usr/local/${TARGETDIR_VER}/python/${_target_LIB}/python${PYTHON_VERSION}/${_suffix}"
          )
        endif()
        unset(_requests_dep)
        unset(_requests_deps)
        unset(_suffix)
      endif()
      unset(_target_LIB)

    endif()
  endif()

elseif(WIN32)

  set(KRAKEN_TEXT_FILES_DESTINATION ".")

  if(WITH_WINDOWS_PDB)
    if(WITH_WINDOWS_STRIPPED_PDB)
      # Icky hack for older cmake from https://stackoverflow.com/a/21198501
      # $<CONFIG> will work in newer cmake but the version currently (3.12)
      # on the buildbot does not support this endavour.
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/kraken_public.pdb DESTINATION . RENAME kraken.pdb)
    else()
      install(FILES $<TARGET_PDB_FILE:kraken> DESTINATION . RENAME kraken.pdb)
    endif()
  endif()

  if(WITH_OPENVDB)
    install(
      FILES ${LIBDIR}/openvdb/bin/openvdb.dll
      DESTINATION "."
      CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
    )
    install(
      FILES ${LIBDIR}/openvdb/bin/openvdb_d.dll
      DESTINATION "."
      CONFIGURATIONS Debug
    )
  endif()

  set(PYTHON_VERSION 3.9)
  set(_PYTHON_VERSION_NO_DOTS 39)

  if(WITH_PYTHON)

    install(
      FILES
        ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python${_PYTHON_VERSION_NO_DOTS}.dll
      DESTINATION
        "."
    )

    string(REPLACE "." "" _PYTHON_VERSION_NO_DOTS ${PYTHON_VERSION})

    if(NOT CMAKE_COMPILER_IS_GNUCC)
      install(
        FILES ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python${_PYTHON_VERSION_NO_DOTS}.dll
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python3.dll
        DESTINATION "."
        CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
      )

      install(
        FILES ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python${_PYTHON_VERSION_NO_DOTS}.dll
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python3.dll
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python${_PYTHON_VERSION_NO_DOTS}_d.dll
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python3_d.dll
        DESTINATION "."
        CONFIGURATIONS Debug
      )
    endif()

    if(WITH_PYTHON_INSTALL)
      # note, as far as python is concerned 'RelWithDebInfo' is not debug since its without debug flags.

      install(DIRECTORY DESTINATION ${TARGETDIR_VER}/python)
      install(DIRECTORY DESTINATION ${TARGETDIR_VER}/python/lib)

      install(
        DIRECTORY ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/lib
        DESTINATION ${KRAKEN_VERSION}/python/
        CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
        PATTERN ".svn" EXCLUDE
        PATTERN "*_d.*" EXCLUDE                  # * debug libraries *
        PATTERN "__pycache__" EXCLUDE           # * any cache *
        PATTERN "*.pyc" EXCLUDE                 # * any cache *
        PATTERN "*.pyo" EXCLUDE                 # * any cache *
      )

       install(
        DIRECTORY ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/lib
        DESTINATION ${KRAKEN_VERSION}/python/
        CONFIGURATIONS Debug
        PATTERN ".svn" EXCLUDE
        PATTERN "__pycache__" EXCLUDE           # * any cache *
        PATTERN "*.pyc" EXCLUDE                 # * any cache *
        PATTERN "*.pyo" EXCLUDE                 # * any cache *)

      )

      install(
        DIRECTORY ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/DLLs
        DESTINATION ${KRAKEN_VERSION}/python
        CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
        PATTERN "*.pdb" EXCLUDE
        PATTERN "*_d.*" EXCLUDE
      )

      install(
        DIRECTORY ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/DLLs
        DESTINATION ${KRAKEN_VERSION}/python
        CONFIGURATIONS Debug
      )

      install(
        FILES ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python${_PYTHON_VERSION_NO_DOTS}.dll
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python.exe
        DESTINATION ${KRAKEN_VERSION}/python/bin
        CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
      )
      install(
        FILES ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python${_PYTHON_VERSION_NO_DOTS}.dll
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python${_PYTHON_VERSION_NO_DOTS}_d.dll
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python_d.exe
              ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/bin/python.exe
        DESTINATION ${KRAKEN_VERSION}/python/bin
        CONFIGURATIONS Debug
      )

      if(WINDOWS_PYTHON_DEBUG)
        install(
          FILES ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/libs/python${_PYTHON_VERSION_NO_DOTS}.pdb
          DESTINATION "."
          CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
        )

        install(
          FILES ${LIBDIR}/python/${_PYTHON_VERSION_NO_DOTS}/libs/python${_PYTHON_VERSION_NO_DOTS}_d.pdb
          DESTINATION "."
          CONFIGURATIONS Debug
        )
      endif()
    endif()

    unset(_PYTHON_VERSION_NO_DOTS)
  endif()

############################################################ TBB ##########

install(
  FILES
    ${LIBDIR}/tbb/bin/tbb.dll
  DESTINATION "."
  CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
)

install(
  FILES
    ${LIBDIR}/tbb/bin/tbb_debug.dll
  DESTINATION "."
  CONFIGURATIONS Debug
)

if(WITH_TBB_MALLOC_PROXY)
  install(
    FILES
      ${LIBDIR}/tbb/bin/tbbmalloc.dll
      ${LIBDIR}/tbb/bin/tbbmalloc_proxy.dll
    DESTINATION "."
    CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
  )
  install(
    FILES
      ${LIBDIR}/tbb/bin/tbbmalloc_debug.dll
      ${LIBDIR}/tbb/bin/tbbmalloc_proxy_debug.dll
    DESTINATION "."
    CONFIGURATIONS Debug
  )
endif()

############################################################ BOOST ##########

# set(BOOST_POSTFIX "vc143-mt-x64-1_70.dll")
# install(
#   FILES
#     ${BOOST_LIBPATH}/boost_atomic-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_chrono-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_date_time-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_filesystem-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_iostreams-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_program_options-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_python39-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_regex-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_system-${BOOST_POSTFIX}
#     ${BOOST_LIBPATH}/boost_thread-${BOOST_POSTFIX}
#   DESTINATION "."
#   CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
# )

# set(BOOST_DEBUG_POSTFIX "vc143-mt-gd-x64-1_70.dll")
# install(
#   FILES
#     ${BOOST_DEBUG_LIBPATH}/boost_atomic-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_chrono-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_date_time-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_filesystem-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_iostreams-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_program_options-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_python39-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_regex-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_system-${BOOST_DEBUG_POSTFIX}
#     ${BOOST_DEBUG_LIBPATH}/boost_thread-${BOOST_DEBUG_POSTFIX}
#   DESTINATION "."
#   CONFIGURATIONS Debug
# )

######################################################## PYTHON ##########

# install(
#   DIRECTORY
#     # ${LIBDIR}/usd-static/lib/python/
#     ${CMAKE_SOURCE_DIR}/source/kraken/draw/engines/ember/python/
#   DESTINATION
#     ${TARGETDIR_VER}/scripts/modules
# )

######################################################## ARNOLD ##########

# if(WITH_ARNOLD)
#   install(
#     FILES
#       ${CMAKE_SOURCE_DIR}/intern/arnold/render_delegate/plugInfo.json
#     DESTINATION
#       "${TARGETDIR_VER}/datafiles/plugin/hdArnold/resources"
#   )

#   install(
#     FILES
#       ${CMAKE_SOURCE_DIR}/intern/arnold/ndr/plugInfo.json
#     DESTINATION
#       "${TARGETDIR_VER}/datafiles/plugin/ndrArnold/resources"
#   )

#   install(
#     DIRECTORY
#       ${LIBDIR}/arnold/bin/
#     DESTINATION
#       "."
#   )

#   install(
#     FILES
#       ${LIBDIR}/arnold-usd/Release/bin/arnold_to_usd.exe
#     DESTINATION
#       "."
#   )
# endif()

elseif(APPLE)
  if(NOT WITH_PYTHON_MODULE)
    # Uppercase name for app bundle
    set_target_properties(kraken PROPERTIES OUTPUT_NAME KRAKEN)
  endif()

  # handy install macro to exclude files, we use \$ escape for the "to"
  # argument when calling so ${BUILD_TYPE} does not get expanded
  macro(install_dir from to)
    install(
      DIRECTORY ${from}
      DESTINATION ${to}
      PATTERN ".git" EXCLUDE
      PATTERN ".svn" EXCLUDE
      PATTERN "*.pyc" EXCLUDE
      PATTERN "*.pyo" EXCLUDE
      PATTERN "*.orig" EXCLUDE
      PATTERN "*.rej" EXCLUDE
      PATTERN "__pycache__" EXCLUDE
      PATTERN "__MACOSX" EXCLUDE
      PATTERN ".DS_Store" EXCLUDE
      PATTERN "config-${PYTHON_VERSION}m/*.a" EXCLUDE  # static lib
      PATTERN "lib2to3" EXCLUDE                   # ./lib2to3
      PATTERN "tkinter" EXCLUDE                   # ./tkinter
      PATTERN "lib-dynload/_tkinter.*" EXCLUDE    # ./lib-dynload/_tkinter.co
      PATTERN "idlelib" EXCLUDE                   # ./idlelib
      PATTERN "test" EXCLUDE                      # ./test
      PATTERN "turtledemo" EXCLUDE                # ./turtledemo
      PATTERN "turtle.py" EXCLUDE                 # ./turtle.py
      PATTERN "wininst*.exe" EXCLUDE              # from distutils, avoid malware false positive
    )
  endmacro()

  set(OSX_APP_SOURCEDIR ${CMAKE_SOURCE_DIR}/release/darwin/Kraken.app)

  # setup Info.plist
  execute_process(COMMAND date "+%Y-%m-%d"
                  OUTPUT_VARIABLE KRAKEN_DATE
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  set_target_properties(kraken PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${OSX_APP_SOURCEDIR}/Contents/Info.plist
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${KRAKEN_VERSION}.${KRAKEN_VERSION_PATCH}"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${KRAKEN_VERSION}.${KRAKEN_VERSION_PATCH} ${KRAKEN_DATE}")

  # Gather the date in finder-style
  execute_process(COMMAND date "+%m/%d/%Y/%H:%M"
  OUTPUT_VARIABLE SETFILE_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

  # Give the bundle actual creation/modification date
  #
  # Note that the directory might not yet exist, which happens when CMake is first run.
  if(NOT EXISTS ${EXECUTABLE_OUTPUT_PATH}/Kraken.app)
    file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/Kraken.app)
  endif()
  execute_process(COMMAND SetFile -d ${SETFILE_DATE} -m ${SETFILE_DATE}
                  ${EXECUTABLE_OUTPUT_PATH}/Kraken.app)

  install(
    TARGETS kraken
    DESTINATION "."
  )

  # install release and app files
  set(KRAKEN_TEXT_FILES_DESTINATION Kraken.app/Contents/Resources/text)

  install(
    FILES ${OSX_APP_SOURCEDIR}/Contents/PkgInfo
    DESTINATION Kraken.app/Contents
  )

  install_dir(
    ${OSX_APP_SOURCEDIR}/Contents/Resources
    Kraken.app/Contents/
  )

  if(WITH_LLVM AND NOT LLVM_STATIC)
    install(
      FILES ${LIBDIR}/llvm/lib/libLLVM-3.4.dylib
      DESTINATION Kraken.app/Contents/MacOS
    )
  endif()

  # python
  if(WITH_PYTHON AND NOT WITH_PYTHON_MODULE AND NOT WITH_PYTHON_FRAMEWORK)
    # Copy the python libs into the install directory
    install_dir(
      ${PYTHON_LIBPATH}
      ${TARGETDIR_VER}/python/lib
    )

    install(DIRECTORY ${LIBDIR}/python/bin
      DESTINATION ${TARGETDIR_VER}/python
      USE_SOURCE_PERMISSIONS
    )

    # Needed for distutils/pip
    # get the last part of the include dir, will be 'python{version}{abiflag}',
    get_filename_component(_py_inc_suffix ${PYTHON_INCLUDE_DIR} NAME)
    install(
      FILES ${PYTHON_INCLUDE_DIR}/pyconfig.h
      DESTINATION ${TARGETDIR_VER}/python/include/${_py_inc_suffix}
    )
    unset(_py_inc_suffix)
  endif()

endif()

# -----------------------------------------------------------------------------
# Generic Install, for all targets

# KRAKEN PYTHON MODULE
if(WITH_PYTHON)
  if(UNIX)
    install(
      DIRECTORY
        ${CMAKE_SOURCE_DIR}/release/scripts/modules/
      DESTINATION
        ${TARGETDIR_VER}/python/lib/python3.9/site-packages
    )
  elseif(WIN32)
    install(
      DIRECTORY
        ${CMAKE_SOURCE_DIR}/release/scripts/modules/
      DESTINATION
        ${TARGETDIR_VER}/scripts/modules
    )  
  endif()
endif()

# ARNOLD PYTHON MODULE
if(WITH_ARNOLD)
  if(UNIX)
    install(
      DIRECTORY
        ${LIBDIR}/python/arnold
      DESTINATION
        ${TARGETDIR_VER}/python/lib/python3.9/site-packages
    )
  elseif(WIN32)
    install(
      DIRECTORY
        ${LIBDIR}/arnold/python/arnold/
      DESTINATION
        ${TARGETDIR_VER}/scripts/modules/arnold
    )  
  endif()
endif()

if(DEFINED KRAKEN_TEXT_FILES_DESTINATION)

  install(
    CODE
    "
    file(READ \"${CMAKE_SOURCE_DIR}/release/text/readme.html\" DATA_SRC)
    string(REGEX REPLACE \"KRAKEN_VERSION\" \"${KRAKEN_VERSION}\" DATA_DST \"\${DATA_SRC}\")
    file(WRITE \"${CMAKE_BINARY_DIR}/release/text/readme.html\" \"\${DATA_DST}\")
    unset(DATA_SRC)
    unset(DATA_DST)
    "
  )
  list(APPEND KRAKEN_TEXT_FILES
    ${CMAKE_BINARY_DIR}/release/text/readme.html
  )

  install(
    FILES ${KRAKEN_TEXT_FILES}
    DESTINATION "${KRAKEN_TEXT_FILES_DESTINATION}"
  )

  install(
    DIRECTORY
    ${CMAKE_SOURCE_DIR}/release/license
    DESTINATION "${KRAKEN_TEXT_FILES_DESTINATION}"
)
endif()

# install more files specified elsewhere
delayed_do_install(${TARGETDIR_VER})

unset(KRAKEN_TEXT_FILES)
unset(KRAKEN_TEXT_FILES_DESTINATION)


# -----------------------------------------------------------------------------
# Geometry Icons

# Geometry icons.
get_property(_icon_names GLOBAL PROPERTY ICON_GEOM_NAMES)
set(_icon_files)
foreach(_f ${_icon_names})
  list(APPEND _icon_files
    "${CMAKE_SOURCE_DIR}/release/datafiles/icons/${_f}.dat"
  )
endforeach()
install(
  FILES ${_icon_files}
  DESTINATION ${TARGETDIR_VER}/datafiles/icons
)

unset(_icon_names)
unset(_icon_files)
unset(_f)

# -----------------------------------------------------------------------------
# Plugins

# install(
#   DIRECTORY
#     ${CMAKE_SOURCE_DIR}/release/datafiles/usd
#   DESTINATION ${TARGETDIR_VER}/datafiles
# )

# install(
#   DIRECTORY
#     ${LIBDIR}/usd/plugin/usd/
#   DESTINATION ${TARGETDIR_VER}/datafiles/plugin
#   CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel
# )

# install(
#   DIRECTORY
#     ${LIBDIR}/usd-debug/plugin/usd/
#   DESTINATION ${TARGETDIR_VER}/datafiles/plugin
#   CONFIGURATIONS Debug
# )

# -----------------------------------------------------------------------------
# Icons

if(UNIX)
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/iconfiles/
      ${CMAKE_SOURCE_DIR}/release/windows/icons/
    DESTINATION "/usr/local/${TARGETDIR_VER}/datafiles/icons"
  )
else()
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/iconfiles/
      ${CMAKE_SOURCE_DIR}/release/windows/icons/
    DESTINATION ${TARGETDIR_VER}/datafiles/icons
  )
endif()

# -----------------------------------------------------------------------------
# Shaders

# install(
#   DIRECTORY
#     ${CMAKE_SOURCE_DIR}/source/kraken/draw/engines/overlay/shaders/
#   DESTINATION ${TARGETDIR_VER}/datafiles/shaders
# )

# -----------------------------------------------------------------------------
# Textures

if(UNIX)
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/textures/
    DESTINATION "/usr/local/${TARGETDIR_VER}/datafiles/textures"
  )
else()
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/textures/
    DESTINATION ${TARGETDIR_VER}/datafiles/textures
  )
endif()

# -----------------------------------------------------------------------------
# Color Management (OCIO)

if(UNIX)
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/datafiles/colormanagement/
    DESTINATION "/usr/local/${TARGETDIR_VER}/datafiles/colormanagement"
  )
else()
  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/release/datafiles/colormanagement/
    DESTINATION ${TARGETDIR_VER}/datafiles/colormanagement
  )
endif()

# -----------------------------------------------------------------------------
# Setup link libs

target_link_libraries(kraken ${LIB})
unset(LIB)

setup_platform_linker_flags(maelstrom)
setup_platform_linker_flags(maelstrom_static)

setup_platform_linker_flags(kraken)
setup_platform_linker_libs(kraken)

if(APPLE)
  set_target_properties(kraken PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/osx_locals.map)
elseif(UNIX)
  set_target_properties(kraken PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/kraken.map)
endif()

if(WIN32)
  # set_target_properties(kraken PROPERTIES VS_GLOBAL_VcpkgEnabled "false")
  set_target_properties(kraken PROPERTIES
    PDB_NAME "kraken_private"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    if(WITH_WINDOWS_PDB AND WITH_WINDOWS_STRIPPED_PDB)
      # This is slightly messy, but single target generators like ninja will not have the
      # CMAKE_CFG_INTDIR variable and multitarget generators like msbuild will not have
      # CMAKE_BUILD_TYPE. This can be simplified by target_link_options and the $<CONFIG>
      # generator expression in newer cmake (2.13+) but until that time this fill have suffice.
      if(CMAKE_BUILD_TYPE)
        set_property(TARGET kraken APPEND_STRING PROPERTY LINK_FLAGS  " /PDBSTRIPPED:${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/kraken_public.pdb")
      else()
        set_property(TARGET kraken APPEND_STRING PROPERTY LINK_FLAGS  " /PDBSTRIPPED:${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/kraken_public.pdb")
      endif()
  endif()
endif()

# -----------------------------------------------------------------------------
# Setup Kraken

if(WIN32 AND NOT WITH_PYTHON_MODULE)
  install(
    TARGETS kraken
    COMPONENT Kraken
    DESTINATION "."
  )
  
  set_target_properties(kraken PROPERTIES
    VS_WINRT_COMPONENT TRUE
  )

  # Installs NuGet Packages.
  kraken_init_nuget()

  # Other required dependencies to MSIX AppX Package
  # Python, Color Management, Icons, Scripts, Etc.
  if(KRAKEN_RELEASE_MODE)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/Release/${KRAKEN_VERSION}
            DESTINATION "./AppX/.")
  else()
    install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/Debug/${KRAKEN_VERSION}
            DESTINATION "./AppX/.")
  endif()
endif()

# -----------------------------------------------------------------------------
# Post-install script

if(POSTINSTALL_SCRIPT)
  install(SCRIPT ${POSTINSTALL_SCRIPT})
endif()
