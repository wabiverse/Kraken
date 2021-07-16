set(WABI_PREFIX wabi/usdImaging)
set(WABI_PACKAGE usdviewq)

if (NOT WITH_PIXAR_USDVIEW)
    return()
endif()

wabi_library(usdviewq
    LIBRARIES
        tf
        usd
        usdGeom
        ${Boost_PYTHON_LIBRARY}

    INCLUDE_DIRS
        ${Boost_INCLUDE_DIRS}

    PUBLIC_CLASSES
        utils

    PYTHON_CPPFILES
        moduleDeps.cpp

    PUBLIC_HEADERS
        api.h

    PYMODULE_CPPFILES
        module.cpp
        wrapUtils.cpp

    PYMODULE_FILES
        python/__init__.py
        python/appEventFilter.py
        python/arrayAttributeView.py
        python/customAttributes.py
        python/frameSlider.py
        python/appController.py
        python/usdviewApi.py
        python/plugin.py
        python/scalarTypes.py
        python/stageView.py
        python/usdviewContextMenuItem.py
        python/primContextMenu.py
        python/primContextMenuItems.py
        python/headerContextMenu.py
        python/attributeViewContextMenu.py
        python/layerStackContextMenu.py
        python/prettyPrint.py
        python/pythonInterpreter.py
        python/settings.py
        python/settings2.py
        python/adjustClipping.py
        python/adjustDefaultMaterial.py
        python/preferences.py
        python/primViewItem.py
        python/common.py
        python/legendUtil.py
        python/primLegend.py
        python/primTreeWidget.py
        python/propertyLegend.py
        python/attributeValueEditor.py
        python/overridableLineEdit.py
        python/variantComboBox.py
        python/qt.py
        python/rootDataModel.py
        python/selectionDataModel.py
        python/viewSettingsDataModel.py
        python/freeCamera.py
        python/debugFlagsWidget.py
        resources/ui/usdviewstyle.qss

    PYSIDE_UI_FILES
        resources/ui/mainWindowUI.ui
        resources/ui/adjustClippingUI.ui
        resources/ui/adjustDefaultMaterialUI.ui
        resources/ui/attributeValueEditorUI.ui
        resources/ui/preferencesUI.ui
        resources/ui/primLegendUI.ui
        resources/ui/propertyLegendUI.ui

    DOXYGEN_FILES
        docs/README.md
        docs/blackBoxTesting.md
)

if(UNIX)
    install(DIRECTORY
        resources/icons
        DESTINATION /usr/local/share/kraken/${TARGETDIR_VER}/python/lib/python3.9/site-packages/wabi/Usdviewq
    )

    install(DIRECTORY
        resources/fonts
        DESTINATION /usr/local/share/kraken/${TARGETDIR_VER}/python/lib/python3.9/site-packages/wabi/Usdviewq
    )
elseif(WIN32)
    install(DIRECTORY
        resources/icons
        DESTINATION ${TARGETDIR_VER}/scripts/modules/wabi/Usdviewq
    )

    install(DIRECTORY
        resources/fonts
        DESTINATION ${TARGETDIR_VER}/scripts/modules/wabi/Usdviewq
    )
endif()