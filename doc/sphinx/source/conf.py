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
import sys
import textwrap

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(1, '../../../release/scripts/modules')

import wpy

print("\nKRAKEN VERSION RELEASE: {}\n{} All Rights Reserved.\n".format(wpy.__version__, wpy.__author__))

version = wpy.__version__.replace('(', '').replace(')', '').replace(', ', '.')
release = version

# on_rtd = os.environ.get('READTHEDOCS', None) == 'True'

# if on_rtd:
#     subprocess.call('cd ..; doxygen', shell=True)

import sphinx_rtd_theme

html_logo = 'outline.svg'
html_theme = "sphinx_rtd_theme"
html_show_sphinx = False;
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

display_version = True;

# -- Project information -----------------------------------------------------

project = 'KRAKEN'
copyright = '2021, Wabi'
author = 'Furby™'

# The full version, including alpha/beta/rc tags
release = 'latest'

html_context = {
  'display_github': True,
  'github_user': 'furby-tm',
  'github_repo': 'KRAKEN',
  'github_version': 'main',
  'theme_navigation_depth': 4,
  'conf_py_path': '/doc/doxygen/source/',
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


KRAKEN_SOURCE_DIRECTORY = os.path.abspath("../../../kraken/source").replace('\\', '/')
WABI_SOURCE_DIRECTORY = os.path.abspath("../../../wabi").replace('\\', '/')

# Temporarily disable documentation build for code,
# broken.
# INPUT                  += {WABI_SOURCE_DIRECTORY}

exhale_args = {
    ############################################################################
    # Main extension setup.                                                    #
    ############################################################################
    "containmentFolder":     "./api",
    "rootFileName":          "kraken_api_root.rst",
    "rootFileTitle":         "KRAKEN Developer Reference",
    "doxygenStripFromPath":  "..",
    ############################################################################
    # Suggested optional arguments.                                            #
    ############################################################################
    "createTreeView":        True,
    "exhaleExecutesDoxygen": True,
    "exhaleDoxygenStdin": textwrap.dedent(f'''
                          GENERATE_XML            = YES
                          XML_PROGRAMLISTING      = YES
                          INPUT                   = {KRAKEN_SOURCE_DIRECTORY}
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
                          INCLUDE_PATH            = {WABI_SOURCE_DIRECTORY + "/../"}
                          INCLUDE_PATH           += {KRAKEN_SOURCE_DIRECTORY}
                          PREDEFINED              = DOXYGEN_SHOULD_SKIP_THIS
                          PREDEFINED             += doxygen
                          PREDFINED              += "TF_DECLARE_PUBLIC_TOKENS(name, x, y)=class name"
                          PREDEFINED             += "TF_DEFINE_STACKED(name, x, y)=class name"
                          PREDEFINED             += "WABI_NAMESPACE_BEGIN"
                          PREDEFINED             += "WABI_NAMESPACE_END"
                          PREDEFINED             += "RAPIDJSON_NAMESPACE_BEGIN"
                          PREDEFINED             += "RAPIDJSON_NAMESPACE_END"
                          PREDEFINED             += "WABI_NS"
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
                          PREDEFINED             += "HDPH_API"
                          PREDEFINED             += "HDPH_API"
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
                          EXCLUDE_SYMBOLS        += HdPh_*
                          EXCLUDE_SYMBOLS        += HdPh_*
                          EXCLUDE_SYMBOLS        += Glf_*
                          EXCLUDE_SYMBOLS        += UsdImaging_*
                          EXCLUDE_SYMBOLS        += UsdImagingGL_*
                          EXCLUDE_PATTERNS        = */gf/*.template.h
                          EXCLUDE_PATTERNS       += */garch/glApi.h
                          EXCLUDE_PATTERNS       += */covalib/CLI_string_utils.h
                          EXCLUDE_PATTERNS       += */sdf/abstractData.h
                          EXCLUDE_PATTERNS       += */usdImaging/adapterRegistry.h
                          EXCLUDE_PATTERNS       += */usdAbc/alembicFileFormat.h
                          EXCLUDE_PATTERNS       += */usdAbc/alembicUtil.h
                          EXCLUDE_PATTERNS       += */hdx/aovInputTask.h
                          EXCLUDE_PATTERNS       += */tf/ostreamMethods.h
                          EXCLUDE_PATTERNS       += */usd/crateDataTypes.h
                          EXCLUDE_PATTERNS       += */nodes/NOD_tokens.h
                          EXCLUDE_PATTERNS       += */krakenfile/miniz.h
                      '''),
    ############################################################################
    # HTML Theme specific configurations.                                      #
    ############################################################################
    # Fix broken Sphinx RTD Theme 'Edit on GitHub' links
    # Search for 'Edit on GitHub' on the FAQ:
    #     http://exhale.readthedocs.io/en/latest/faq.html
    "pageLevelConfigMeta": ":github_url: https://github.com/Wabi-Studios/KRAKEN",
    ############################################################################
    # Main library page layout configuration.                                  #
    ############################################################################
    "afterTitleDescription": textwrap.dedent(u'''
        Developer reference and home of the KRAKEN project -- redefining animation
        composition, collaborative workflows, simulation engines, skeletal rigging
        systems, and look development from storyboard to final render. Built on
        the underlying software architecture provided by Pixar, and extended to
        meet the ever-growing needs of both artists and production pipelines. It
        is with this strong core foundation, that we may begin to solve the most
        challenging issues the world of modern graphics demands, and push the
        framework for composition & design into the future.

        .. note::
          This is the developer reference as it pertains to the C++ codebase of
          KRAKEN. It is subject to change frequently, as different iterations are
          necessary to find the correct approach. Currently, there is an additional
          Python API available -- the Pixar USD module, wabi, which needs further
          testing to assess it's capabilities for use in a suitable scripting and
          plugin writing environment embedded within the scope of a full-scale DCC
          application.
    '''),
    ############################################################################
    # Individual page layout configuration.                                    #
    ############################################################################
    "contentsTitle": "Page Contents",
    "kindsWithContentsDirectives": ["class", "file", "namespace", "struct"],
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

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['../_build',
                    'darkenergy/*.txt',
                    '*.template.h',
                    'garch/glApi.h',
                    'covalib/CLI_string_utils.h',
                    'sdf/abstractData.h',
                    'usdImaging/adapterRegistry.h',
                    'usdAbc/alembicFileFormat.h',
                    'usdAbc/alembicUtil.h',
                    'hdx/aovInputTask.h',
                    'tf/ostreamMethods.h',
                    'usd/crateDataTypes.h',
                    'nodes/NOD_tokens.h',
                    'krakenfile/miniz.h']


# -- Options for HTML output -------------------------------------------------

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
templates_path = ['_templates']
html_static_path = ['_static']

html_last_updated_fmt = "%a, %b %d, %Y"

def setup(app):
    app.add_css_file("main_stylesheet.css")
