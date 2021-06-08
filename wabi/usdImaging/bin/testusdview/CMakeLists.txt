set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdviewq)

if (NOT WABI_BUILD_USDVIEW)
    return()
endif()

wabi_python_bin(testusdview
    DEPENDENCIES
        usdviewq
)

if (NOT WABI_HEADLESS_TEST_MODE)
    wabi_install_test_dir(
        SRC testenv/testUsdviewAlive
        DEST testUsdviewAlive
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewFileArguments
        DEST testUsdviewFileArguments1
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewFileArguments
        DEST testUsdviewFileArguments2
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewFileArguments
        DEST testUsdviewFileArguments3
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewFileArguments
        DEST testUsdviewFileArguments4
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewFileArguments
        DEST testUsdviewFileArguments5
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewFileArguments
        DEST testUsdviewFileArguments6
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewFileArguments
        DEST testUsdviewFileArguments7
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewWrapper
        DEST testUsdviewWrapper1
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewWrapper
        DEST testUsdviewWrapper2
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewWrapper
        DEST testUsdviewWrapper3
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewWrapper
        DEST testUsdviewWrapper4
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewWrapper
        DEST testUsdviewWrapper5
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewPrimPathBar
        DEST testUsdviewPrimPathBar
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewPrimSearch
        DEST testUsdviewPrimSearch
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewPropertySearch
        DEST testUsdviewPropertySearch
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewMetadatatabSelect
        DEST testUsdviewMetadatatabSelect
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewTimeSamples
        DEST testUsdviewTimeSamples
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewNavigationKeys
        DEST testUsdviewNavigationKeys
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewReloadReopen
        DEST testUsdviewReloadReopen
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewNoPlugins
        DEST testUsdviewNoPlugins
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewInterpreterNoRender
        DEST testUsdviewInterpreterNoRender
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewLoadPlugins
        DEST testUsdviewLoadPlugins_alive
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewLoadPlugins
        DEST testUsdviewLoadPlugins_multipleContainers
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewLoadPlugins
        DEST testUsdviewLoadPlugins_missingContainer
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewLoadPlugins
        DEST testUsdviewLoadPlugins_duplicateCommand
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewLoadPlugins
        DEST testUsdviewLoadPlugins_chained
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewUpAxis
        DEST testUsdviewUpAxis_zUp
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewUpAxis
        DEST testUsdviewUpAxis_yUp
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewDeactivate
        DEST testUsdviewDeactivate
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewDefaultFontFamily
        DEST testUsdviewDefaultFontFamily
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewSkinning
        DEST testUsdviewSkinning
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewPrimTreeExpansion
        DEST testUsdviewPrimTreeExpansion
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewInfGeom
        DEST testUsdviewInfGeom
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewShaderEdits
        DEST testUsdviewShaderEdits
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewPrimvarEdits
        DEST testUsdviewPrimvarEdits
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewInstancingEdits
        DEST testUsdviewInstancingEdits
    )

    wabi_install_test_dir(
        SRC testenv/testUsdviewAnimatedBounds
        DEST testUsdviewAnimatedBounds
    )

    wabi_register_test(testUsdviewAlive
        PRE_COMMAND "${PYTHON_EXECUTABLE} testUsdviewAliveSetup.py"
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testAlive.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewFileArguments1
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testValidFileArg.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewFileArguments2
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testValidFileArg.py test.usd"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewFileArguments3
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testValidFileArg.py test.usdc"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewFileArguments4
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testInvalidFileArg.py test.py"
        STDERR_REDIRECT py_test_out
        DIFF_COMPARE py_test_out
        EXPECTED_RETURN_CODE 1
    )

    wabi_register_test(testUsdviewFileArguments5
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testInvalidFileArg.py invalidSyntax.usda"
        CLEAN_OUTPUT "(?:[A-Za-z]:)?/(?!refSphere2).*/"
        STDERR_REDIRECT invalidSyntax_test_out
        DIFF_COMPARE invalidSyntax_test_out
        EXPECTED_RETURN_CODE 1
    )

    wabi_register_test(testUsdviewFileArguments6
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testInvalidFileArg.py test.txt"
        STDERR_REDIRECT txt_test_out
        DIFF_COMPARE txt_test_out
        EXPECTED_RETURN_CODE 1
    )

    wabi_register_test(testUsdviewFileArguments7
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testInvalidFileArg.py missing"
        STDERR_REDIRECT missing_test_out
        DIFF_COMPARE missing_test_out
        EXPECTED_RETURN_CODE 1
    )

    wabi_register_test(testUsdviewWrapper1
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testCallback.py"
        EXPECTED_RETURN_CODE 2
    )

    wabi_register_test(testUsdviewWrapper2
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testCallback_Invalid_1.py test.usda"
        EXPECTED_RETURN_CODE 1
    )

    wabi_register_test(testUsdviewWrapper3
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testCallback_Invalid_2.py test.usda"
        EXPECTED_RETURN_CODE 1
    )

    wabi_register_test(testUsdviewWrapper4
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testCallback_Invalid_3.py test.usda"
        EXPECTED_RETURN_CODE 1
    )

    wabi_register_test(testUsdviewWrapper5
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview test.usda"
        EXPECTED_RETURN_CODE 2
    )

    wabi_register_test(testUsdviewPrimPathBar
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewPrimPathBar.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewPrimSearch
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewPrimSearch.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewPropertySearch
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewPropertySearch.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewMetadatatabSelect
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewMetadatatabSelect.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewTimeSamples
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewTimeSamples.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewNavigationKeys
        RUN_SERIAL
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewNavigationKeys.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewReloadReopen
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewReloadReopen.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewNoPlugins
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewNoPlugins.py --noplugins test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewInterpreterNoRender
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewInterpreterNoRender.py --norender test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewLoadPlugins_alive
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewLoadPlugins_alive.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewLoadPlugins_multipleContainers
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewLoadPlugins_multipleContainers.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewLoadPlugins_missingContainer
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewLoadPlugins_missingContainer.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewLoadPlugins_duplicateCommand
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewLoadPlugins_duplicateCommand.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewLoadPlugins_chained
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewLoadPlugins_chained.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewUpAxis_zUp
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewUpAxis_zUp.py testZUp.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewUpAxis_yUp
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewUpAxis_yUp.py testYUp.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewDeactivate
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewDeactivate.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewDefaultFontFamily
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewDefaultFontFamily.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewSkinning
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewSkinning.py arm.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewPrimTreeExpansion
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewPrimTreeExpansion.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewInfGeom
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewInfGeom.py infGeom.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewShaderEdits
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewShaderEdits.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewPrimvarEdits
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewPrimvarEdits.py primvars.usda"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdviewInstancingEdits
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewInstancingEdits.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

wabi_register_test(testUsdviewAnimatedBounds
        PYTHON
	COMMAND "${CMAKE_INSTALL_PREFIX}/bin/testusdview --testScript testUsdviewAnimatedBounds.py test.usda"
        EXPECTED_RETURN_CODE 0
    )

endif()
