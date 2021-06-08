set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdrecord)

wabi_python_bin(usdrecord
    DEPENDENCIES
        usd
        usdAppUtils)

wabi_install_test_dir(
    SRC testenv/RecordTest
    DEST testUsdRecordSingleFrame
)

wabi_install_test_dir(
    SRC testenv/RecordTest
    DEST testUsdRecordMultipleFrames
)

if (NOT WABI_HEADLESS_TEST_MODE)

    wabi_register_test(testUsdRecordSingleFrame
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdrecord AnimCube.usda AnimCube.png"
        EXPECTED_RETURN_CODE 0
    )

    wabi_register_test(testUsdRecordMultipleFrames
        PYTHON
        COMMAND "${CMAKE_INSTALL_PREFIX}/bin/usdrecord AnimCube.usda AnimCube.#.png --frames 1,5,10"
        EXPECTED_RETURN_CODE 0
    )

endif()
