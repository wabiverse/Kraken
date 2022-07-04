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
  'github_user': 'Wabi-Studios',
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
    "rootFileName":          "wabi_api_root.rst",
    "doxygenStripFromPath":  "../../../",
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

        .. figure:: https://www.dropbox.com/s/705rku14oqy8b9g/piper-banner.jpg?raw=1


        =================================
        Universal Scene Description (USD)
        =================================

        **USD is a system for authoring, composing, and reading hierarchically organized scene description.**

        USD comprises a set of modules that scalably encode and interchange static and 
        time-sampled 3D geometry and shading data between Digital Content Creation applications. 
        Domain-specific schema modules define the geometry and shading encoding atop USD's 
        domain-agnostic core.

        Continue reading about USD's purpose and overall architecture, or jump directly into the 
        core of the USD API :ref:`page_usd_page_front`.


        .. _Usd_Overview:

        --------------------
        Overview and Purpose
        --------------------

        In addition to addressing the interchange problem, USD also provides powerful
        mechanisms for large-scale collaboration and context-dependent asset refinement
        anywhere in a production pipeline. For example: **layers** enable artists in different 
        departments to all work simultaneously on the same "thing", whether that be a singular 
        asset, an aggregate asset, a sequence, or a shot.

        - **variants** enable asset creators to package up an enumerated set of variations, which can be selected and re-selected by downstream artists in consuming contexts.  Using variants in combination with the **inherits** composition operator, a downstream artist can introduce  *new* variations for all instances of an asset, in a particular (set, sequence, shot, etc.) context. Abilities like this can dramatically reduce the number of pipeline-stalling "asset fix requests" needed to support downstream artists.

        This set of documents describes the concrete Usd software package, its APIs,
        and their effective use. In contrast, the forthcoming
        *Composition Compendium* centers on USD composition's detailed semantics.

        Following is an overview of the architecture, followed by information for
        :ref:`Usd_Quickstart` "getting started with USD", and some
        :ref:`Usd_Background` "background on how the USD project came to be."


        .. _Usd_ArchitecturalOverview: 
        
        Architectural Overview
        ======================

        The USD repository is divided into four "core" packages, plus 3rd-party
        application plugins and extras:

        - **base** Contains application-agnostic, foundation modules, upon which most Pixar software builds, including possible future open-source projects.

        - **usd** Contains all the modules directly related to authoring, reading, and composing universal scene description. Defines the plugin interfaces by which USD can be extended and adapted.

        - **imaging** Contains the embeddable Hydra hardware renderer as well as other low-level, imaging-related services.

        - **usdImaging** Ties usd and hydra together by providing an adapter to Hydra that reads data directly and efficiently from a UsdStage. Defines a plugin interface by which the adapter can be extended, and provides *usdview*, the standalone graphical USD inspector tool.

        - **third_party** Contains USD plugins for DCC applications.

        - **extras** Contains a collection of tutorial material, sample code, and small example USD assets.

        To author and consume USD, you need only the *base* and *usd* packages.
        Most of the open-source third-party plugins, however, do make use of
        *usdImaging* for fast preview, and as part of the answer to the question of
        what application XXX should do when it is asked to import a USD prim type of
        which it has no corresponding native representation.  (Answer: it creates
        a proxy representation that delegates preview to usdImaging, and presents the
        prim's properties for inspection and overriding.)

        Following is a deeper indexing of the four packages.


        .. _Usd_Package_Base: 
        
        The "base" package
        ------------------

        - :ref:`page_arch_page_front` module centralizes functionality that must be implemented differently for different operating systems or architectures, so that we can avoid littering the entire code-base with conditionally-compiled code.

        - :ref:`page_tf_page_front` module is a catch-all for low-level, commonly used services developed at Pixar, including facilities for memory tracking, error reporting and debugging, string utilities, boost python wrapping aids, threading/synchronization tools, registry and singleton management, smart pointers, and "observer pattern" support, which we call "notification".

        - :ref:`page_gf_page_front` module provides Pixar's equivalent to imath, and provides many of the types enumerated in Vt.  It also provides other foundational graphics abstractions like frusta and a camera model. Given unlimited resources we would have replaced our use of the linear algebra components in Gf with imath, but Gf's use in Pixar's code-base is substantial and its API's do not map one-to-one to imath.

        - :ref:`page_js_page_front` module provides a thin API wrapper on top of the `RapidJSON <http://rapidjson.org/>`_ package, which is what our plugin system uses for multi-threaded plugin discovery.

        - :ref:`page_trac_page_front` module provides an interface for embeddable performance-profiling tagging.

        - :ref:`page_vt_page_front` module provides many of the concrete types that Sdf is able to recognize and serialize.  It provides a copy-on-write array-type, VtArray, which is used for all array types in Usd, and an efficient type-erasure wrapper class, VtValue, that also provides datatype conversion facilities and support for unboxing python objects from boost python.  VtValue is supported in all Usd API for getting and setting values.

        - :ref:`page_work_page_front` module provides a thin abstraction layer on top of Intel's TBB (Thread Building Blocks), and is leveraged extensively in Usd core and higher-level services for multi-threading.

        - :ref:`page_plug_page_front` module provides the organization and access API for all plugins.  It identifies plugins by the existence of a plugInfo.json manifest file, which enumerates the number and kinds of plugins provided by a module.


        .. _Usd_Package_Usd: 
        
        The "usd" package
        -----------------

        The USD *package* begins with the low-level modules for path resolution,
        scene description serialization, and composition, upon which the "core" Usd
        module relies.  The USD package is rounded out by a set of schema and utility
        modules that layer on top of the core.  In low-to-high order, the modules are:

        - :ref:`page_ar_page_front` module defines the abstract interface for USD's asset resolution plugin, so that clients can author asset references in their USD files that make sense to their asset management systems.  It also provides a "fallback resolver" that is active when no site-level plugin has been provided; the fallback resolver provides basic search-path based resolution.

        - :ref:`page_kind_page_front` module provides a simple, site-extensible token-based typing system, which USD uses, for example, to identify and classify types of "models".

        - :ref:`page_sdf_page_front` module defines the Usd data model, namely: prims, attributes, relationships, meta-data, as well as the concrete types that attributes can possess. Sdf also provides the key abstraction SdfLayer, which represents a file of data subscribing to the Usd data model, and provides low-level data authoring and extraction API's. SdfLayer also has an associated plugin mechanism known as SdfFileFormat that allows any reasonable file format to be dynamically translated into Usd; it is via this mechanism that we added a binary encoding to the pre-existing ASCII format for Usd, as well as how we support referencing and reading of Alembic files.

        - :ref:`page_pcp_page_front` module implements the composition logic at the heart of USD; Pcp efficiently builds and caches an "index cache" that the Usd scenegraph uses to determine which prims need to be populated, and where to look for values for any particular property in a multi-layer aggregation of assets.

        - :ref:`page_usd_page_front` module builds the USD scene-graph (UsdStage), and provides the primary API's for authoring and reading composed scene description. The Usd module and the concepts it contains are meant to be fairly generic, allowing many different domain-specific schemas to be built over it, as the USD package does for geometry, shading, etc.  We make an exception for two particular concepts/schemas, "Model" and "AssetInfo", which are defined in the core Usd module, because they are extremely valuable for organizing scene description, and other core behaviors are built on top of the concept of Model.

        - :ref:`page_usdGeom_page_front` The primary graphics-supporting schema module for DCC interchange, UsdGeom provides geometric primitives (meshes, curves, patches, xforms, etc.), as well as a Camera schema and "Primvars" that encode UV's and various user-defined fields over a primitive.  The majority of the schema classes are produced entirely via the *usdGenSchema* code generator included with the core.

        - :ref:`page_usdShade_page_front` Schema module that defines Looks, Shaders, connectible Parameters, with API for building shading networks and user-facing "Look Interfaces" that allow shading networks to be shared (via instancing) by multiple Looks.  UsdShade also prescribes how geometry is *bound* to Looks.  With just the objects in UsdShade one can encode completely generic shading; however, it also allows renderer-specific shading schemas to be built on top of it, and a Look can host the network "terminals" for any number of renderer schemas.

        - :ref:`page_usdRi_page_front` module for data targeted at Pixar's RenderMan specification. Includes shading schemas for both RSL and RIS shading systems, as well as a "catch all" RiStatements schema for encoding various RenderMan concepts/directives, such as attributes, options, coordinate systems, etc.

        - :ref:`page_usdUtils_page_front` module provides a number of utilities and conventions that have proven useful in a USD-based pipeline.


        .. _Usd_Package_Imaging: 
        
        The "imaging" package
        ---------------------

        - **Garch** : The Graphics Architecture module provides abstractions for architecture-specific functionality that may pull in imaging-related dependencies.

        - **Glf** : The GL Foundations module provides access to textures, ptextures and GL resources (such as draw targets). It also provides some basic glsl shaders used by hdSt.

        - **Hio** : The Hydra Resource I/O module provides resource loaders used by Hydra, such as Pixar's shader container format (glslfx). It will eventually house the infrastructure for image loading that is currently parked in Glf.

        - **CameraUtil** The Camera Utilities module provides a small but important set of common camera-related computations that don't really belong in the core GfCamera class.

        - **PxOsd** : The Pixar OpenSubdiv module provides some higher-level utilities on top of OpenSubdiv that may, one day make it into OpenSubdiv, but are still "cooking".  Hydra interacts with OpenSubdiv through pxOsd.

        - **Hd** : The Hydra module provides the Hydra renderer.  Hydra uses OpenSubdiv for tessellating subdivision meshes, and supports general programmable glsl shaders, and multiple render passes.

        - **Hdx** : The Hydra Extensions module provides higher-level functionality on top of Hydra, such as organization and packaging for particular kinds of multiple render-pass tasks.


        .. _Usd_Package_UsdImaging: 
        
        The "usdImaging" package
        ------------------------

        - :ref:`page_usdAppUtils_page_front` : A library that provides a number of utilities and common functionality for applications that view and/or record images of UsdStages.

        - **UsdImaging** : An adapter to Pixar's modern, high-performance GL rendering engine *Hydra* (which in turn leverages OpenSubdiv) that provides preview imaging (including streaming playback) for any UsdStage encoded with UsdGeom and UsdShade.  UsdImaging takes full advantage of scenegraph instancing in USD, and uses multiple threads to extract data from a UsdStage and marshal it into Hd objects.

        - **Usdviewq** : A python package that provides the controller and GUI elements used by usdview.  The elements are structured for ease of embedding in other python applications, also.


        .. _Usd_PythonSupport: 
        
        Python Support, numpy, etc.
        ===========================

        Usd includes python wrapping for all core classes and generated schema
        classes. Each module described in the last section, plus the core Usd
        module, are independently importable. Each module includes in-python
        help/documentation extracted from the doxygen docs for the module.  We have
        chosen, initially, to keep the python API as true to the C++ API as possible,
        with one notable exception.

        - **Some underlying modules support more pythonic API's.** Some of the foundational modules shared with Presto, such as Sdf, provide a more pythonic API in which member-variable-like-things become properties in python. We have hesitated to deploy this level of pythonification at higher levels of the system because SdfLayer is the only container that truly owns its data: all the primary Usd-level classes represent restricted views on data that resides in one-or-more SdfLayers, and don't possess any member variables/properties other than those that serve as temporary caches.

        USD's central array type, used to hold all array-valued scene description types,
        is VtArray. The python wrapping for VtArray supports python's "buffer protocol",
        so VtArray 's returned by USD to python can be trivially accessed like and
        converted to native python containers, as well as numpy arrays.  Further,
        USD methods that expect a VtArray input argument should always successfully
        convert native python containers and numpy arrays whose underlying element-type
        is compatible.


        .. _Usd_Quickstart: 
        
        Quickstart!
        ===========

        Please visit `the USD Documentation Hub <http://openusd.org>`_ where
        you will find tutorials, FAQs, and other supporting material.


        .. _Usd_Background USD: 
        
        What's the Point, and Why Isn't it Alembic?
        ===========================================

        The outward-facing goal of Universal Scene Description is to take the next
        step in DCC application data interchange *beyond* what is encodable in the
        ground-breaking Alembic interchange package.  The chief component of that
        step is the ability to encode, interchange, and edit entire scenes with the
        ability to share variation and repeated/instanced scene data across shots and
        sequences, by providing robust asset-level (but not restricted to
        asset-level) file-referencing with sparse override capabilities.
        Additionally, USD provides a handful of other "composition operators" and
        concepts that target:

        - Encoding asset variation and preserving the ability to switch variants late in the pipeline.

        - Scale to scenes of unlimited complexity by deferring reading of heavy property data until it is needed (as in Alembic), and also by providing composition semantics that allow deferred (and reversible) loading and composition of arbitrary amounts of scene description without sacrificing the ability to perform robust dependency analysis between prims in a scene. (See the discussion of :ref:`Usd_Payloads` "payloads" for more information).

        The USD project also developed under a high priority inward-facing mandate to
        simplify and unify Pixar's binary cache-based geometry package (TidScene)
        with its ASCII, animation and rigging scene description and composition
        system (Presto core). This mandate required that the advanced rigging
        concepts and datatypes of Presto be layerable on top of a reduced-featureset,
        shared (with USD) core. Given the time and resource constraints, and
        necessity to not massively disrupt our in-use codebase, we chose to largely
        retain our existing data model, while looking to Alembic as a guide for many
        schema decisions along the way. While it is unlikely Pixar will attempt to
        expand the Alembic API and core to encompass USD composition, asset resolution,
        and necessary plugin mechanisms, we are committed to providing a USD
        "file format" plugin that allows the USD core and toolset to consume and author
        Alembic files as if they were native USD files (obviously writing cannot always
        produce an equally meaningful Alembic file because composition operators cannot
        be represented in Alembic).


        .. _Usd_Related:

        Related Pages
        =============

        - `Pixar USD Home Page <http://openusd.org>`_


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
