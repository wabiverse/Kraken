
.. Keep this document short & concise,
   linking to external resources instead of including content in-line.
   See 'release/text/readme.html' for the end user read-me.


******
Kraken
******

.. image:: https://img.shields.io/github/v/release/Wabi-Studios/Kraken?include_prereleases
   :target: https://github.com/Wabi-Studios/Kraken/releases/latest
   :alt: GitHub release (latest SemVer including pre-releases)

.. image:: https://github.com/Wabi-Studios/Kraken/actions/workflows/cmake_macos_arm64.yml/badge.svg
   :target: https://github.com/Wabi-Studios/Kraken/actions/workflows/cmake_macos_arm64.yml
   :alt: Build Status for arch macOS arm64

.. image:: https://github.com/Wabi-Studios/Kraken/actions/workflows/build_docs_macos_arm64.yml/badge.svg
   :target: https://docs.wabi.foundation
   :alt: Documentation

.. figure:: https://www.dropbox.com/s/0fcfeo5u3q18ryh/kraken-logo.png?raw=1
   :scale: 50 %
   :align: center

*Animators, Developers, Filmmakers, & Artists. This Belongs to You.*

    Documentation is currently **experimental**, so please don't mind the dust over at docs.wabi.foundation_.
    Top level pixar docs can now be found here_, eventually, we can get most pages looking like this_.
    
    .. _this: https://docs.wabi.foundation/api/page_page_tf_MallocTag.html#page_tf__malloc_tag_1MallocTagAddingTags
    .. _here: https://docs.wabi.foundation/api/wabi_api_root.html
    .. _docs.wabi.foundation: https://docs.wabi.foundation

Home of the Kraken -- The free and open source 3D animation platform redefining
animation composition, collaborative workflows, simulation engines, skeletal
rigging systems, and look development from storyboard to final render. Built on
the underlying software architecture provided by Pixar, and extended to meet the
ever-growing needs of both artists and production pipelines. It is with this strong
core foundation, that we may begin to solve the most challenging issues the world
of modern graphics demands, and push the framework for composition & design into
the future.

Join the community: https://discord.gg/5PYrUu4hqa


Phoenix
========
The OpenSubdiv-based real-time render engine of the 21st century.

.. figure:: https://www.dropbox.com/s/zs38x71vou8raje/image_processing20220110-15733-visqpw.png?raw=1
   :scale: 50 %
   :align: center


File Format
============
The one and only -- **Universal** file format. The first of it's kind,
that addresses the need to robustly and scalably interchange and augment
arbitrary 3D scenes that may be composed from many elemental assets.

.. figure:: https://www.dropbox.com/s/w0ul2nvda4dckg4/kraken-fileformat.png?raw=1
   :scale: 50 %
   :align: center


Automated Builds
================
All commits result in a ready-to-install Kraken package, just visit the 
actions_ tab and select your operating system. The "build artifact" linked
at the bottom of each successful workflow will download it directly. Currently
only **macOS on Apple Silicon (M1, M2, ...)** is supported, with Windows and Linux soon to follow.

.. figure:: https://www.dropbox.com/s/m6yd9qczayeia55/automated_ci_macos.png?raw=1
   :scale: 50 %
   :align: center

.. _actions: https://github.com/Wabi-Studios/Kraken/actions/workflows/cmake_macos_arm64.yml


Static Python Bindings
======================
Kraken has an embedded Python interpreter which is loaded when Kraken is started and stays active while Kraken is running. The internal Kraken module **kpy** statically binds both the (``kpy.context`` ``kpy.data`` ``kpy.app``, ...) API and all the Pixar **USD** python bindings within the same python module with substantial crossover between the two APIs at it's core. This is **USD** as a *runtime*, and is the framework for which all core development is founded upon. On the outset it might not look like much as this is still early days -- but you can imagine something similiar to Blender's **bpy** python module, with the main difference being the "Scene" or in this context, the "Stage", having *total* control over the underlying API schematics and/or opinions. A digital content creation suite *which evolves to the content* - rather than the content having to adapt to the needs of a platform.

.. figure:: https://www.dropbox.com/s/1yka8wfqtl07m2z/Screenshot%202022-07-23%20at%2012.38.28%20AM.png?raw=1
   :scale: 50 %
   :align: center


Credits
-------
- Blender Foundation: https://blender.org
- Pixar: https://graphics.pixar.com

License
-------
Kraken uses the GNU General Public License, which describes the rights
to distribute or change the code.

Please read this file for the full license.
https://www.gnu.org/licenses/gpl-3.0.txt

Apart from the GNU GPL, Kraken is not available under other licenses.


|
| *We speak in pixel, and we come in peace.*
