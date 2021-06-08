set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_python_bin(usdchecker
    DEPENDENCIES
        usd
        usdGeom
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker1
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker2
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker3
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker4
)

wabi_register_test(testUsdChecker1
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker clean/clean.usd"
    EXPECTED_RETURN_CODE 0
)


wabi_register_test(testUsdChecker2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker clean/clean_flat.usdc"
    EXPECTED_RETURN_CODE 0
)

wabi_register_test(testUsdChecker3
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker clean/clean.usdz"
    EXPECTED_RETURN_CODE 0
)

wabi_register_test(testUsdChecker4
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker clean/clean_flat.usdz"
    EXPECTED_RETURN_CODE 0
)


wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker5
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker6
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker7
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker8
)

wabi_register_test(testUsdChecker5
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker --arkit clean/clean_arkit.usdz"
    EXPECTED_RETURN_CODE 0
)


wabi_register_test(testUsdChecker6
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker --arkit clean/clean_flat.usdc"
    EXPECTED_RETURN_CODE 1
)

wabi_register_test(testUsdChecker7
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker --arkit --rootPackageOnly clean/clean_flat.usdz"
    EXPECTED_RETURN_CODE 0
)

wabi_register_test(testUsdChecker8
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker --arkit clean/clean_flat.usdz"
    EXPECTED_RETURN_CODE 1
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker9
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker10
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker11
)

wabi_install_test_dir(
    SRC testenv/testUsdChecker
    DEST testUsdChecker12
)

wabi_register_test(testUsdChecker9
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker bad/badUsdz.usdz"
    EXPECTED_RETURN_CODE 1
)


wabi_register_test(testUsdChecker10
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker --arkit --dumpRules bad/variants.usdc -o variants_failedChecks.txt"
    DIFF_COMPARE variants_failedChecks.txt
    EXPECTED_RETURN_CODE 1
)

wabi_register_test(testUsdChecker11
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker bad/brokenRef.usd"
    EXPECTED_RETURN_CODE 1
)

wabi_register_test(testUsdChecker12
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdchecker --arkit bad/badShaderUnsupportedTexture.usdc"
    EXPECTED_RETURN_CODE 1
)
