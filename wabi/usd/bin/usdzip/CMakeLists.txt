set(WABI_PREFIX wabi/usd)
set(WABI_PACKAGE usd)

wabi_python_bin(usdzip
    DEPENDENCIES
        usd
)

wabi_install_test_dir(
    SRC testenv/testUsdZipInputFiles
    DEST testUsdZipInputFiles
)

wabi_install_test_dir(
    SRC testenv/testUsdZipAsset
    DEST testUsdZipAsset
)

wabi_install_test_dir(
    SRC testenv/testUsdZipAsset
    DEST testUsdZipARKitAsset
)

wabi_register_test(testUsdZipInputFiles
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdzip flat.usdz src -l flat.txt"
    DIFF_COMPARE flat.txt
    EXPECTED_RETURN_CODE 0
)

wabi_install_test_dir(
    SRC testenv/testUsdZipInputFiles
    DEST testUsdZipInputFiles2
)

wabi_register_test(testUsdZipInputFiles2
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdzip -r recursive.usdz src -l recursive.txt"
    DIFF_COMPARE recursive.txt
    EXPECTED_RETURN_CODE 0
)

wabi_install_test_dir(
    SRC testenv/testUsdZipInputFiles
    DEST testUsdZipInputFiles3
)

wabi_register_test(testUsdZipInputFiles3
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdzip flat_inputs.usdz src/a.txt src/b.png"
    EXPECTED_RETURN_CODE 0
)

wabi_install_test_dir(
    SRC testenv/testUsdZipInputFiles
    DEST testUsdZipInputFiles4
)

wabi_register_test(testUsdZipInputFiles4
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdzip recursive_inputs.usdz src/a.txt src/b.png src/sub"
    EXPECTED_RETURN_CODE 0
)

wabi_register_test(testUsdZipMissingInput
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdzip nonexistent.usdz nonexistent.usd"
    EXPECTED_RETURN_CODE 1
)

wabi_register_test(testUsdZipAsset
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdzip -a root.usd package.usdz -l contents.txt"
    DIFF_COMPARE contents.txt
    EXPECTED_RETURN_CODE 0
)

wabi_register_test(testUsdZipARKitAsset
    PYTHON
    COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdzip --arkitAsset root.usd package.usdz -l contents_arkit.txt"
    DIFF_COMPARE contents_arkit.txt
    EXPECTED_RETURN_CODE 0
)
