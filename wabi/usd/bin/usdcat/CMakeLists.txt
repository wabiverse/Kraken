set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_python_bin(usdcat
    DEPENDENCIES
        ar
        sdf
        usd
)

wabi_install_test_dir(
    SRC testenv/testCatToFile
    DEST testUsdCatOutToFile
)

wabi_register_test(testUsdCatOutToFile
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat input.usda --out output.usda"
    DIFF_COMPARE output.usda
    EXPECTED_RETURN_CODE 0
)

wabi_install_test_dir(
    SRC testenv/testVariadicCatting
    DEST testUsdCatVariadicCatting
)

wabi_register_test(testUsdCatVariadicCatting
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat a.usd b.usd c.usd"
    EXPECTED_RETURN_CODE 0
)

wabi_install_test_dir(
    SRC testenv/testMissingOrInvalidFiles
    DEST testUsdCatMissingOrInvalidFiles1
)

wabi_register_test(testUsdCatMissingOrInvalidFiles1
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat hello.txt"
    EXPECTED_RETURN_CODE 1
)

wabi_install_test_dir(
    SRC testenv/testMissingOrInvalidFiles
    DEST testUsdCatMissingOrInvalidFiles2
)

wabi_register_test(testUsdCatMissingOrInvalidFiles2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat foo.usda"
    EXPECTED_RETURN_CODE 1
)

wabi_install_test_dir(
    SRC testenv/testUsdCatMask
    DEST testUsdCatMask
)

wabi_register_test(testUsdCatMask
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat --mask /InsideMask --flatten --skipSourceFileComment input.usda -o output.usda"
    DIFF_COMPARE output.usda
    EXPECTED_RETURN_CODE 0
)

wabi_register_test(testUsdCatMask2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat --mask /InsideMask input.usda -o output.usda"
    EXPECTED_RETURN_CODE 1
)

wabi_install_test_dir(
    SRC testenv/testLoadOnly
    DEST testUsdCatLoadOnly
)

wabi_register_test(testUsdCatLoadOnly
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat --loadOnly input.usda"
    EXPECTED_RETURN_CODE 0
)

wabi_install_test_dir(
    SRC testenv/testUsdCatLayerMetadata
    DEST testUsdCatLayerMetadata
)

wabi_register_test(testUsdCatLayerMetadata
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdcat --layerMetadata input.usda -o output.usda"
    DIFF_COMPARE output.usda
    EXPECTED_RETURN_CODE 0
)
