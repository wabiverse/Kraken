set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_python_bin(usddiff
    DEPENDENCIES
        tf
        sdf
)

wabi_install_test_dir(
    SRC testenv/testExitCodesForDiff
    DEST testUsdDiffExitCodesForDiff1
)

wabi_register_test(testUsdDiffExitCodesForDiff1
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.usda a.usda --noeffect"
    EXPECTED_RETURN_CODE 0
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testExitCodesForDiff
    DEST testUsdDiffExitCodesForDiff2
)

wabi_register_test(testUsdDiffExitCodesForDiff2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.usda b.usda --noeffect"
    EXPECTED_RETURN_CODE 1
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffToolSelectionDefault
    DEST testUsdDiffToolSelectionDefault
)

wabi_register_test(testUsdDiffToolSelectionDefault
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.usda b.usda --noeffect"
    STDOUT_REDIRECT out.txt
    DIFF_COMPARE out.txt
    EXPECTED_RETURN_CODE 1
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffToolVariousUsdFormats
    DEST testUsdDiffToolVariousFormats1
)

wabi_register_test(testUsdDiffToolVariousFormats1
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.usda a.usd --noeffect"
    EXPECTED_RETURN_CODE 0
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffToolVariousUsdFormats
    DEST testUsdDiffToolVariousFormats2
)

wabi_register_test(testUsdDiffToolVariousFormats2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.usdc a.usd --noeffect"
    EXPECTED_RETURN_CODE 0
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffToolVariousUsdFormats
    DEST testUsdDiffToolVariousFormats3
)

if (WITH_ALEMBIC)
	wabi_register_test(testUsdDiffToolVariousFormats3
	    PYTHON
	    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.abc a.abc --noeffect"
	    EXPECTED_RETURN_CODE 0
	    PRE_PATH
		${CMAKE_INSTALL_PREFIX}/bin
	)

	wabi_install_test_dir(
	    SRC testenv/testDiffToolVariousUsdFormats
	    DEST testUsdDiffToolVariousFormats4
	)

	wabi_register_test(testUsdDiffToolVariousFormats4
	    PYTHON
	    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.abc a.txt --noeffect"
	    EXPECTED_RETURN_CODE 2
	    PRE_PATH
		${CMAKE_INSTALL_PREFIX}/bin
	)

	wabi_install_test_dir(
	    SRC testenv/testDiffToolVariousUsdFormats
	    DEST testUsdDiffToolVariousFormats5
	)

	wabi_register_test(testUsdDiffToolVariousFormats5
	    PYTHON
	    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.txt a.abc --noeffect"
	    EXPECTED_RETURN_CODE 2
	    PRE_PATH
		${CMAKE_INSTALL_PREFIX}/bin
        )
else()
    message(STATUS "Skipping alembic-based usddiff tests because WITH_ALEMBIC=OFF")
endif()

if (WITH_DRACO)
    # Checks that USD diff tool successfully recognizes and loads Draco files.
    wabi_install_test_dir(
        SRC testenv/testDiffToolDracoFormat
        DEST testUsdDiffWithDracoFiles
    )

    wabi_register_test(testUsdDiffWithDracoFiles
    PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect CubeWithGenericPrimvars.drc CubeWithGenericPrimvars.drc"
        EXPECTED_RETURN_CODE 0
        PRE_PATH
            ${CMAKE_INSTALL_PREFIX}/bin
    )

    # Checks that the content of Draco file compares equally to the
    # corresponding USDA file.
    wabi_install_test_dir(
        SRC testenv/testDiffToolDracoFormat
	DEST testUsdDiffWithUsdaAndDracoFiles
    )

    wabi_register_test(testUsdDiffWithUsdaAndDracoFiles
    PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect CubeWithGenericPrimvars.drc CubeWithGenericPrimvars.usda"
        EXPECTED_RETURN_CODE 0
        PRE_PATH
            ${CMAKE_INSTALL_PREFIX}/bin
    )

    # Checks that USD diff tool can load USDA file that refer to Draco file.
    # Note that the files are flattened in order to pull in the content of
    # referred Draco files and not just their references.
    wabi_install_test_dir(
        SRC testenv/testDiffToolDracoFormat
        DEST testUsdDiffWithReferredDracoFiles
    )

    wabi_register_test(testUsdDiffWithReferredDracoFiles
    PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect --flatten CubeWithGenericPrimvarsCompressed.usda CubeWithGenericPrimvarsCompressed.usda"
        EXPECTED_RETURN_CODE 0
        PRE_PATH
            ${CMAKE_INSTALL_PREFIX}/bin
    )

    # Checks that USD diff tool can load USDZ file that includes Draco files.
    # Note that the files are flattened in order to pull in the content of
    # referred Draco files and not just their references.
    wabi_install_test_dir(
	SRC testenv/testDiffToolDracoFormat
        DEST testUsdDiffWithDracoUsdz
    )

    wabi_register_test(testUsdDiffWithDracoUsdz
    PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect --flatten ComplexSetCompressed.usdz ComplexSetCompressed.usdz"
        EXPECTED_RETURN_CODE 0
        PRE_PATH
            ${CMAKE_INSTALL_PREFIX}/bin
    )
else()
    message(STATUS "Skipping Draco-based usddiff tests because WITH_DRACO=OFF")
endif()

wabi_install_test_dir(
    SRC testenv/testDiffToolComposedResults
    DEST testUsdDiffComposedResults1
)

wabi_register_test(testUsdDiffComposedResults1
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.usd b.usd --noeffect"
    EXPECTED_RETURN_CODE 1
    STDOUT_REDIRECT layer_diff.txt
    DIFF_COMPARE layer_diff.txt
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffToolComposedResults
    DEST testUsdDiffComposedResults2
)

wabi_register_test(testUsdDiffComposedResults2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff a.usd b.usd --noeffect --flatten"
    EXPECTED_RETURN_CODE 1
    STDOUT_REDIRECT composed_diff.txt
    DIFF_COMPARE composed_diff.txt
    CLEAN_OUTPUT "(?:[A-Za-z]:)?\\/.*\\/"
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories1
)

wabi_register_test(testUsdDiffWithDirectories1
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect a/ a/"
    EXPECTED_RETURN_CODE 0
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories2
)

wabi_register_test(testUsdDiffWithDirectories2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect a/ b/"
    EXPECTED_RETURN_CODE 1
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories3
)

wabi_register_test(testUsdDiffWithDirectories3
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect . a/"
    EXPECTED_RETURN_CODE 0
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories4
)

wabi_register_test(testUsdDiffWithDirectories4
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect a.usda a/"
    EXPECTED_RETURN_CODE 0
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories5
)

wabi_register_test(testUsdDiffWithDirectories5
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect a.usda b/"
    EXPECTED_RETURN_CODE 1
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories6
)

wabi_register_test(testUsdDiffWithDirectories6
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect b/ a.usda"
    EXPECTED_RETURN_CODE 1
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories7
)

wabi_register_test(testUsdDiffWithDirectories7
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect a.usda b.usda b/"
    EXPECTED_RETURN_CODE 1
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)

wabi_install_test_dir(
    SRC testenv/testDiffWithDirectories
    DEST testUsdDiffWithDirectories8
)

wabi_register_test(testUsdDiffWithDirectories8
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usddiff --noeffect b/ a.usda b.usda"
    EXPECTED_RETURN_CODE 1
    PRE_PATH
        ${CMAKE_INSTALL_PREFIX}/bin
)
