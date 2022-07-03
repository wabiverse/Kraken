# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import subprocess
import re
import sys
import textwrap
from exhale import utils

# -- Platform specific stuff -------------------------------------------------

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(1, '../../../release/scripts/modules')

if sys.platform == "linux" or sys.platform == "linux2":
  sys.path.insert(2, '')
  SVN_INCLUDE_ROOT = ""
  SYS_INCLUDE_ROOT = ""
elif sys.platform == "darwin":
  sys.path.insert(2, '../../../../lib/apple_darwin_arm64/python/lib/python3.10')
  SVN_INCLUDE_ROOT = "../../../../lib/apple_darwin_arm64/python/include/python3.10"
  SYS_INCLUDE_ROOT = "/opt/homebrew/include"
elif sys.platform == "win32":
  sys.path.insert(2, '')
  SVN_INCLUDE_ROOT = ""
  SYS_INCLUDE_ROOT = ""

print("\nKRAKEN VERSION RELEASE: {}\n{} All Rights Reserved.\n".format("1.50", "Wabi Animation Studios"))

version = "1.50"
release = version

import sphinx_rtd_theme

html_logo = 'outline.svg'
html_theme = "sphinx_rtd_theme"
html_show_sphinx = False;
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

display_version = True;

# -- Project information -----------------------------------------------------

project = 'Kraken'
copyright = '2022, Wabi'
author = 'Furby™'

# The full version, including alpha/beta/rc tags
release = 'latest'

html_context = {
  'display_github': True,
  'github_user': 'furby-tm',
  'github_repo': 'Kraken',
  'github_version': 'master',
  'theme_navigation_depth': 4,
  'conf_py_path': '/doc/sphinx/source/',
  'kraken_version': version,
}


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
  'breathe',
  'exhale',
]

breathe_projects = {"kraken": "../_build/xml"}
breathe_default_project = "kraken"

KRAKEN_ROOT_DIRECTORY = "../../../."

def specificationsForKind(kind):
    '''
    For a given input ``kind``, return the list of reStructuredText specifications
    for the associated Breathe directive.
    '''
    # Change the defaults for .. doxygenclass:: and .. doxygenstruct::
    if kind == "class" or kind == "struct":
        return [
          ":members:",
          ":protected-members:",
          ":private-members:",
          ":undoc-members:"
        ]
    # Change the defaults for .. doxygenenum::
    elif kind == "enum":
        return [":no-link:"]
    # An empty list signals to Exhale to use the defaults
    else:
        return []

# Adds a considerable amount of time to docs, removed until cleaned up...
# INPUT                   = "../../../source/kraken/anchor"
# INPUT                  += "../../../source/kraken/editors"

# For now, prefer the manual effort of adding doc paths so we're not wasting time
# generating a bunch of pure garbage.
exhale_args = {
    ############################################################################
    # Main extension setup.                                                    #
    ############################################################################
    "containmentFolder":     "./api",
    "rootFileName":          "kraken_api_root.rst",
    "rootFileTitle":         "Kraken Developer Reference",
    "doxygenStripFromPath":  "../../../source/kraken",
    ############################################################################
    # Suggested optional arguments.                                            #
    ############################################################################
    "createTreeView":        True,
    "exhaleExecutesDoxygen": True,
    "customSpecificationsMapping": utils.makeCustomSpecificationsMapping(specificationsForKind),
    "exhaleDoxygenStdin": textwrap.dedent(f'''
                          GENERATE_XML            = YES
                          XML_PROGRAMLISTING      = YES
                          INPUT                   = "../../../source/kraken/ChaosEngine"
                          INPUT                  += "../../../source/kraken/draw"
                          INPUT                  += "../../../source/kraken/krakernel"
                          INPUT                  += "../../../source/kraken/kraklib"
                          INPUT                  += "../../../source/kraken/luxo"
                          INPUT                  += "../../../source/kraken/python/kpy"
                          INPUT                  += "../../../source/kraken/server"
                          INPUT                  += "../../../source/kraken/universe"
                          INPUT                  += "../../../source/kraken/wm"
                          INPUT                  += "../../../wabi/base/arch/docs"
                          INPUT                  += "../../../wabi/base/gf/docs"
                          INPUT                  += "../../../wabi/base/js/docs"
                          INPUT                  += "../../../wabi/base/plug/docs"
                          INPUT                  += "../../../wabi/base/tf/docs"
                          INPUT                  += "../../../wabi/base/trace/docs"
                          INPUT                  += "../../../wabi/base/vt/docs"
                          INPUT                  += "../../../wabi/base/work/docs"
                          INPUT                  += "../../../wabi/imaging/glf/docs"
                          INPUT                  += "../../../wabi/imaging/hd/docs"
                          INPUT                  += "../../../wabi/imaging/hdSt/docs"
                          INPUT                  += "../../../wabi/imaging/hdx/docs"
                          INPUT                  += "../../../wabi/imaging/hio/docs"
                          INPUT                  += "../../../wabi/usd/ar/docs"
                          INPUT                  += "../../../wabi/usd/kind/docs"
                          INPUT                  += "../../../wabi/usd/ndr/docs"
                          INPUT                  += "../../../wabi/usd/pcp/docs"
                          INPUT                  += "../../../wabi/usd/sdf/docs"
                          INPUT                  += "../../../wabi/usd/sdr/docs"
                          INPUT                  += "../../../wabi/usd/usd/docs"
                          INPUT                  += "../../../wabi/usd/usdGeom/docs"
                          INPUT                  += "../../../wabi/usd/usdHydra/docs"
                          INPUT                  += "../../../wabi/usd/usdLux/docs"
                          INPUT                  += "../../../wabi/usd/usdMedia/docs"
                          INPUT                  += "../../../wabi/usd/usdMtlx/docs"
                          INPUT                  += "../../../wabi/usd/usdPhysics/docs"
                          INPUT                  += "../../../wabi/usd/usdRender/docs"
                          INPUT                  += "../../../wabi/usd/usdRi/docs"
                          INPUT                  += "../../../wabi/usd/usdShade/docs"
                          INPUT                  += "../../../wabi/usd/usdSkel/docs"
                          INPUT                  += "../../../wabi/usd/usdUI/docs"
                          INPUT                  += "../../../wabi/usd/usdUtils/docs"
                          INPUT                  += "../../../wabi/usd/usdVol/docs"
                          INPUT                  += "../../../wabi/usdImaging/usdAppUtils/docs"
                          INPUT                  += "../../../wabi/usdImaging/usdviewq/docs"
                          EXTRACT_LOCAL_CLASSES   = NO
                          HIDE_UNDOC_CLASSES      = YES
                          HIDE_SCOPE_NAMES        = YES
                          SHOW_INCLUDE_FILES      = NO
                          GENERATE_TODOLIST       = NO
                          GENERATE_TESTLIST       = NO
                          GENERATE_BUGLIST        = NO
                          QUIET                   = YES
                          WARN_IF_UNDOCUMENTED    = NO
                          WARN_IF_DOC_ERROR       = NO
                          RECURSIVE               = YES
                          SOURCE_BROWSER          = YES
                          USE_HTAGS               = NO
                          HTML_DYNAMIC_SECTIONS   = YES
                          GENERATE_TREEVIEW       = YES
                          EXT_LINKS_IN_WINDOW     = NO
                          GENERATE_LATEX          = NO
                          ENABLE_PREPROCESSING    = YES
                          MACRO_EXPANSION         = YES
                          EXPAND_ONLY_PREDEF      = YES
                          EXTERNAL_GROUPS         = NO
                          EXTERNAL_PAGES          = NO
                          INCLUDE_PATH            = "{KRAKEN_ROOT_DIRECTORY}"
                          INCLUDE_PATH           += "{SVN_INCLUDE_ROOT}"
                          INCLUDE_PATH           += "{SYS_INCLUDE_ROOT}"
                          PREDEFINED             += doxygen
                          PREDFINED              += "TF_DECLARE_PUBLIC_TOKENS"
                          PREDEFINED             += "TF_DEFINE_STACKED"
                          PREDEFINED             += "WABI_NAMESPACE_BEGIN"
                          PREDEFINED             += "WABI_NAMESPACE_END"
                          PREDEFINED             += "RAPIDJSON_NAMESPACE_BEGIN"
                          PREDEFINED             += "RAPIDJSON_NAMESPACE_END"
                          PREDEFINED             += "WABI_NS"
                          PREDEFINED             += "ANCHOR_API"
                          PREDEFINED             += "GARCH_API"
                          PREDEFINED             += "HGIGL_API"
                          PREDEFINED             += "HGIVULKAN_API"
                          PREDEFINED             += "KRAKEN_SLOTS"
                          PREDEFINED             += "KRAKEN_SIGNALS"
                          PREDEFINED             += "CODE_EDITOR_API"
                          PREDEFINED             += "KRAKEN_NODE_API"
                          PREDEFINED             += "HGIMETAL_API"
                          PREDEFINED             += "HIO_API"
                          PREDEFINED             += "PXOSD_API"
                          PREDEFINED             += "KIND_API"
                          PREDEFINED             += "NDR_API"
                          PREDEFINED             += "SDROSL_API"
                          PREDEFINED             += "SDF_API"
                          PREDEFINED             += "SDR_API"
                          PREDEFINED             += "USDMTLX_API"
                          PREDEFINED             += "PCP_API"
                          PREDEFINED             += "ARCH_API"
                          PREDEFINED             += "USD_API"
                          PREDEFINED             += "GF_API"
                          PREDEFINED             += "CAMERAUTIL_API"
                          PREDEFINED             += "HGI_API"
                          PREDEFINED             += "USDUTILS_API"
                          PREDEFINED             += "USDVOL_API"
                          PREDEFINED             += "USDIMAGING_API"
                          PREDEFINED             += "KRAKEN_IO_API"
                          PREDEFINED             += "KRAKEN_PYTHON_API"
                          PREDEFINED             += "KRAKEN_UNIVERSE_API"
                          PREDEFINED             += "KRAKEN_WM_API"
                          PREDEFINED             += "KRAKEN_KERNEL_API"
                          PREDEFINED             += "KRAKEN_LIB_API"
                          PREDEFINED             += "KRAKEN_SPACE_API"
                          PREDEFINED             += "DRAW_GEOM_API"
                          PREDEFINED             += "ENGINE_ETCHER_API"
                          PREDEFINED             += "DRAW_GEOM_API"
                          PREDEFINED             += "USD_CLIPS_API"
                          PREDEFINED             += "USDSKEL_API"
                          PREDEFINED             += "USDGEOM_API"
                          PREDEFINED             += "USDRI_API"
                          PREDEFINED             += "USDSHADE_API"
                          PREDEFINED             += "USDHYDRA_API"
                          PREDEFINED             += "USDUI_API"
                          PREDEFINED             += "USDUTILS_API"
                          PREDEFINED             += "USDSKELIMAGING_API"
                          PREDEFINED             += "USDABC_API"
                          PREDEFINED             += "USDLUX_API"
                          PREDEFINED             += "PCP_API"
                          PREDEFINED             += "PCPNODEREF_API"
                          PREDEFINED             += "PCPPRIMINDEX_API"
                          PREDEFINED             += "AR_API"
                          PREDEFINED             += "VT_API"
                          PREDEFINED             += "TRACE_API"
                          PREDEFINED             += "TRACELITE_API"
                          PREDEFINED             += "PLUG_API"
                          PREDEFINED             += "JS_API"
                          PREDEFINED             += "TF_API"
                          PREDEFINED             += "WORK_API"
                          PREDEFINED             += "HF_API"
                          PREDEFINED             += "HD_API"
                          PREDEFINED             += "HDX_API"
                          PREDEFINED             += "HDST_API"
                          PREDEFINED             += "GLF_API"
                          PREDEFINED             += "USDIMAGING_API"
                          PREDEFINED             += "USDIMAGINGGL_API"
                          FILE_PATTERNS           = *.dox
                          FILE_PATTERNS          += *.h
                          FILE_PATTERNS          += *.md
                          EXCLUDE_SYMBOLS         = Usd_*
                          EXCLUDE_SYMBOLS        += UsdGeom_*
                          EXCLUDE_SYMBOLS        += UsdRi_*
                          EXCLUDE_SYMBOLS        += UsdShade_*
                          EXCLUDE_SYMBOLS        += UsdHydra_*
                          EXCLUDE_SYMBOLS        += UsdUI_*
                          EXCLUDE_SYMBOLS        += UsdUtils_*
                          EXCLUDE_SYMBOLS        += UsdAbc_*
                          EXCLUDE_SYMBOLS        += Sdf_*
                          EXCLUDE_SYMBOLS        += Pcp_*
                          EXCLUDE_SYMBOLS        += PcpNodeRef_*
                          EXCLUDE_SYMBOLS        += PcpPrimIndex_*
                          EXCLUDE_SYMBOLS        += Ar_*
                          EXCLUDE_SYMBOLS        += Vt_*
                          EXCLUDE_SYMBOLS        += Tracelite_*
                          EXCLUDE_SYMBOLS        += Plug_*
                          EXCLUDE_SYMBOLS        += Js_*
                          EXCLUDE_SYMBOLS        += Tf_*
                          EXCLUDE_SYMBOLS        += Work_*
                          EXCLUDE_SYMBOLS        += Arch_*
                          EXCLUDE_SYMBOLS        += Hf_*
                          EXCLUDE_SYMBOLS        += Hd_*
                          EXCLUDE_SYMBOLS        += Hdx_*
                          EXCLUDE_SYMBOLS        += HdSt_*
                          EXCLUDE_SYMBOLS        += Glf_*
                          EXCLUDE_SYMBOLS        += UsdImaging_*
                          EXCLUDE_SYMBOLS        += UsdImagingGL_*
                          EXCLUDE_PATTERNS       += */pch/*
                      '''),
    ############################################################################
    # HTML Theme specific configurations.                                      #
    ############################################################################
    # Fix broken Sphinx RTD Theme 'Edit on GitHub' links
    # Search for 'Edit on GitHub' on the FAQ:
    #     http://exhale.readthedocs.io/en/latest/faq.html
    "pageLevelConfigMeta": ":github_url: https://github.com/Wabi-Studios/Kraken",
    ############################################################################
    # Main library page layout configuration.                                  #
    ############################################################################
    "afterTitleDescription": textwrap.dedent(u'''
        Developer reference and home of the Kraken project -- redefining animation
        composition, collaborative workflows, simulation engines, skeletal rigging
        systems, and look development from storyboard to final render. Built on
        the underlying software architecture provided by Pixar, and extended to
        meet the ever-growing needs of both artists and production pipelines. It
        is with this strong core foundation, that we may begin to solve the most
        challenging issues the world of modern graphics demands, and push the
        framework for composition & design into the future.

        .. note::
          This is the developer reference for the C++ and Python API of Kraken. It
          is subject to change frequently, as different iterations are necessary
          to find the correct approach.
    '''),
    ############################################################################
    # Individual page layout configuration.                                    #
    ############################################################################
    "contentsTitle": "Page Contents",
    "kindsWithContentsDirectives": ["file", "namespace"],
    ############################################################################
    "verboseBuild": True
}

primary_domain = 'cpp'

highlight_language = 'cpp'

source_suffix = '.rst'

master_doc = 'index'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = 'en'

# -- Options for HTML output -------------------------------------------------

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
templates_path = ['_templates']
html_static_path = ['_static']

html_last_updated_fmt = "%a, %b %d, %Y"

def setup(app):
    app.add_css_file("main_stylesheet.css")
