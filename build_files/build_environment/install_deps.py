from __future__ import print_function

from distutils.spawn import find_executable
from distutils.dir_util import copy_tree

from pathlib import Path

import argparse
import textwrap
import codecs
import contextlib
import ctypes
import datetime
import distutils
import fnmatch
import glob
import locale
import multiprocessing
import os
import platform
import re
import shlex
import shutil
import subprocess
import sys
import sysconfig
import tarfile
import zipfile
import rarfile

from sysconfig import get_paths as gp

if sys.version_info.major >= 3:
    from urllib.request import urlopen
else:
    from urllib2 import urlopen

# Helpers for printing output
verbosity = 1

def Print(msg):
    if verbosity > 0:
        print(msg)

def PrintWarning(warning):
    if verbosity > 0:
        print("WARNING:", warning)

def PrintStatus(status):
    if verbosity >= 1:
        print("STATUS:", status)

def PrintInfo(info):
    if verbosity >= 2:
        print("INFO:", info)

def PrintCommandOutput(output):
    if verbosity >= 3:
        sys.stdout.write(output)

def PrintError(error):
    if verbosity >= 3 and sys.exc_info()[1] is not None:
        import traceback
        traceback.print_exc()
    print ("ERROR:", error)

# Helpers for determining platform
def Windows():
    return platform.system() == "Windows"
def Linux():
    return platform.system() == "Linux"
def MacOS():
    return platform.system() == "Darwin"

global INSTALL_DIR
global SOURCE_DIR
global BUILD_DIR

if Linux():
    import distro
    INSTALL_DIR = "../../../lib/linux_centos7_x86_64"
    SOURCE_DIR = "../../../lib/linux_centos7_x86_64/build_env/source"
    BUILD_DIR = "../../../lib/linux_centos7_x86_64/build_env/build"

if MacOS():
    INSTALL_DIR = "../../../lib/apple_darwin_x86_64"
    SOURCE_DIR = "../../../lib/apple_darwin_x86_64/build_env/source"
    BUILD_DIR = "../../../lib/apple_darwin_x86_64/build_env/build"

def Python3():
    return sys.version_info.major == 3

def GetLocale():
    return sys.stdout.encoding or locale.getdefaultlocale()[1] or "UTF-8"

def GetCommandOutput(command):
    """Executes the specified command and returns output or None."""
    try:
        return subprocess.check_output(
            shlex.split(command),
            stderr=subprocess.STDOUT).decode(GetLocale(), 'replace').strip()
    except subprocess.CalledProcessError:
        pass
    return None

def GetXcodeDeveloperDirectory():
    """Returns the active developer directory as reported by 'xcode-select -p'.
    Returns None if none is set."""
    if not MacOS():
        return None

    return GetCommandOutput("xcode-select -p")

def GetVisualStudioCompilerAndVersion():
    """Returns a tuple containing the path to the Visual Studio compiler
    and a tuple for its version, e.g. (14, 0). If the compiler is not found
    or version number cannot be determined, returns None."""
    if not Windows():
        return None

    msvcCompiler = find_executable('cl')
    if msvcCompiler:
        # VisualStudioVersion environment variable should be set by the
        # Visual Studio Command Prompt.
        match = re.search(
            r"(\d+)\.(\d+)",
            os.environ.get("VisualStudioVersion", ""))
        if match:
            return (msvcCompiler, tuple(int(v) for v in match.groups()))
    return None

def IsVisualStudioVersionOrGreater(desiredVersion):
    if not Windows():
        return False

    msvcCompilerAndVersion = GetVisualStudioCompilerAndVersion()
    if msvcCompilerAndVersion:
        _, version = msvcCompilerAndVersion
        return version >= desiredVersion
    return False

def IsVisualStudio2022OrGreater():
    VISUAL_STUDIO_2022_VERSION = (17, 0)
    return IsVisualStudioVersionOrGreater(VISUAL_STUDIO_2022_VERSION)

def IsVisualStudio2019OrGreater():
    VISUAL_STUDIO_2019_VERSION = (16, 0)
    return IsVisualStudioVersionOrGreater(VISUAL_STUDIO_2019_VERSION)

def IsVisualStudio2017OrGreater():
    VISUAL_STUDIO_2017_VERSION = (15, 0)
    return IsVisualStudioVersionOrGreater(VISUAL_STUDIO_2017_VERSION)

def GetVisualStudioDirectories():
    if IsVisualStudio2022OrGreater():
        INSTALL_DIR = "../../../lib/win64_vc17"
        SOURCE_DIR = "../../../lib/win64_vc17/build_env/source"
        BUILD_DIR = "../../../lib/win64_vc17/build_env/build"
    elif IsVisualStudio2019OrGreater():
        INSTALL_DIR = "../../../lib/win64_vc16"
        SOURCE_DIR = "../../../lib/win64_vc16/build_env/source"
        BUILD_DIR = "../../../lib/win64_vc16/build_env/build"
    elif IsVisualStudio2017OrGreater():
        INSTALL_DIR = "../../../lib/win64_vc15"
        SOURCE_DIR = "../../../lib/win64_vc15/build_env/source"
        BUILD_DIR = "../../../lib/win64_vc15/build_env/build"
    else:
        INSTALL_DIR = "../../../lib/win64_vcUNKNOWN"
        SOURCE_DIR = "../../../lib/win64_vcUNKNOWN/build_env/source"
        BUILD_DIR = "../../../lib/win64_vcUNKNOWN/build_env/build"
    return (INSTALL_DIR, SOURCE_DIR, BUILD_DIR)

def GetPythonInfo():
    """Returns a tuple containing the path to the Python executable, shared
    library, and include directory corresponding to the version of Python
    currently running. Returns None if any path could not be determined.
    This function is used to extract build information from the Python 
    interpreter used to launch this script. This information is used
    in the Boost and USD builds. By taking this approach we can support
    having USD builds for different Python versions built on the same
    machine. This is very useful, especially when developers have multiple
    versions installed on their machine, which is quite common now with 
    Python2 and Python3 co-existing.
    """
    # First we extract the information that can be uniformly dealt with across
    # the platforms:
    pythonExecPath = sys.executable
    pythonVersion = sysconfig.get_config_var("py_version_short")  # "2.7"
    pythonVersionNoDot = sysconfig.get_config_var("py_version_nodot") # "27"

    # Lib path is unfortunately special for each platform and there is no
    # config_var for it. But we can deduce it for each platform, and this
    # logic works for any Python version.
    def _GetPythonLibraryFilename():
        if Windows():
            return "python" + pythonVersionNoDot + ".lib"
        elif Linux():
            return sysconfig.get_config_var("LDLIBRARY")
        elif MacOS():
            return "libpython" + pythonVersion + ".dylib"
        else:
            raise RuntimeError("Platform not supported")

    pythonIncludeDir = sysconfig.get_config_var("INCLUDEPY")
    if Windows():
        pythonBaseDir = sysconfig.get_config_var("base")
        pythonLibPath = os.path.join(pythonBaseDir, "libs",
                                        _GetPythonLibraryFilename())
    elif Linux():
        pythonLibDir = sysconfig.get_config_var("LIBDIR")
        pythonMultiarchSubdir = sysconfig.get_config_var("multiarchsubdir")
        if pythonMultiarchSubdir:
            pythonLibDir = pythonLibDir + pythonMultiarchSubdir
        pythonLibPath = os.path.join(pythonLibDir,
                                        _GetPythonLibraryFilename())
    elif MacOS():
        pythonBaseDir = sysconfig.get_config_var("base")
        pythonLibPath = os.path.join(pythonBaseDir, "lib",
                                        _GetPythonLibraryFilename())
    else:
        raise RuntimeError("Platform not supported")

    if Windows():
        python_version_no_dot=39
        python_version=3.9
        python_dir     = "{installed_libs_dir}/python/{py_ver}".format(installed_libs_dir=context.libInstDir, py_ver=python_version_no_dot)
        python_include = os.path.join(python_dir, "include")
        python_lib     = os.path.join(python_dir, "libs")
        python_exe     = os.path.join(python_dir, "bin", "python.exe")

        python_info = {"python_include": python_include.replace('\\', '/'),
                    "python_lib": python_lib.replace('\\', '/'),
                    "python_exe": python_exe.replace('\\', '/'),
                    "python_version": python_version}
        return python_info
    else:
        return (pythonExecPath, pythonLibPath, pythonIncludeDir, pythonVersion)


def GetCPUCount():
    try:
        return multiprocessing.cpu_count()
    except NotImplementedError:
        return 1

def Run(cmd, logCommandOutput = True):
    """Run the specified command in a subprocess."""
    PrintInfo('Running "{cmd}"'.format(cmd=cmd))

    with codecs.open("log.txt", "a", "utf-8") as logfile:
        logfile.write(datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
        logfile.write("\n")
        logfile.write(cmd)
        logfile.write("\n")

        # Let exceptions escape from subprocess calls -- higher level
        # code will handle them.
        if logCommandOutput:
            p = subprocess.Popen(shlex.split(cmd), stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT)
            while True:
                l = p.stdout.readline().decode(GetLocale(), 'replace')
                if l:
                    logfile.write(l)
                    PrintCommandOutput(l)
                elif p.poll() is not None:
                    break
        else:
            p = subprocess.Popen(shlex.split(cmd))
            p.wait()

    if p.returncode != 0:
        # If verbosity >= 3, we'll have already been printing out command output
        # so no reason to print the log file again.
        if verbosity < 3:
            with open("log.txt", "r") as logfile:
                Print(logfile.read())
        raise RuntimeError("Failed to run '{cmd}'\nSee {log} for more details."
                           .format(cmd=cmd, log=os.path.abspath("log.txt")))

@contextlib.contextmanager
def CurrentWorkingDirectory(dir):
    """Context manager that sets the current working directory to the given
    directory and resets it to the original directory when closed."""
    curdir = os.getcwd()
    os.chdir(dir)
    try: yield
    finally: os.chdir(curdir)

def CopyFilesHeaders(src, dest):
    """Copy files like shutil.copy, but src may be a glob pattern."""
    ignore_func = lambda d, files: [f for f in files if (Path(d) / Path(f)).is_file() and not f.endswith('.h')]
    try:
        shutil.copytree(src, dest, ignore=ignore_func)
    except:
        pass

def CopyFiles(context, src, dest):
    """Copy files like shutil.copy, but src may be a glob pattern."""
    filesToCopy = glob.glob(src)
    if not filesToCopy:
        raise RuntimeError("File(s) to copy {src} not found".format(src=src))

    instDestDir = os.path.join(context.libInstDir, dest)
    for f in filesToCopy:
        # print("Copying {file} to {destDir}\n".format(file=f, destDir=instDestDir))
        try:
            shutil.copy(f, instDestDir)
        except:
            pass

def CopyDirectory(context, srcDir, destDir):
    """Copy directory like shutil.copytree."""
    instDestDir = os.path.join(context.libInstDir, destDir)
    if os.path.isdir(instDestDir):
        shutil.rmtree(instDestDir)

    PrintCommandOutput("Copying {srcDir} to {destDir}\n"
                       .format(srcDir=srcDir, destDir=instDestDir))
    shutil.copytree(srcDir, instDestDir)

def CleanInstall(context, dependency):
    dependencypath = Path(context.libInstDir) / dependency
    if dependencypath.exists() and dependencypath.is_dir():
        shutil.rmtree(dependencypath)
    os.mkdir(dependencypath)

    binpath = Path(dependencypath) / 'bin'
    if binpath.exists() and binpath.is_dir():
        shutil.rmtree(binpath)
    os.mkdir(binpath)

    libpath = Path(dependencypath) / 'lib'
    if libpath.exists() and libpath.is_dir():
        shutil.rmtree(libpath)
    os.mkdir(libpath)

    incpath = Path(dependencypath) / 'include'
    if incpath.exists() and incpath.is_dir():
        shutil.rmtree(incpath)
    os.mkdir(incpath)

def FormatMultiProcs(numJobs, generator):
    tag = "-j"
    if generator:
        if "Visual Studio" in generator:
            tag = "/M:"
        elif "Xcode" in generator:
            tag = "-j "

    return "{tag}{procs}".format(tag=tag, procs=numJobs)

def RunCMake(context, force, extraArgs = None):
    """Invoke CMake to configure, build, and install a library whose
    source code is located in the current working directory."""
    # Create a directory for out-of-source builds in the build directory
    # using the name of the current working directory.
    srcDir = os.getcwd()
    libInstDir = (context.libInstDir)
    buildDir = os.path.join(context.buildDir, os.path.split(srcDir)[1])
    if force and os.path.isdir(buildDir):
        shutil.rmtree(buildDir)

    if not os.path.isdir(buildDir):
        os.makedirs(buildDir)

    generator = context.cmakeGenerator

    # On Windows, we need to explicitly specify the generator to ensure we're
    # building a 64-bit project. (Surely there is a better way to do this?)
    # TODO: figure out exactly what "vcvarsall.bat x64" sets to force x64
    if generator is None and Windows():
        if IsVisualStudio2022OrGreater():
            generator = "Visual Studio 17 2022"
        elif IsVisualStudio2019OrGreater():
            generator = "Visual Studio 16 2019"
        else:
            generator = "Visual Studio 15 2017 Win64"

    if generator is not None:
        generator = '-G "{gen}"'.format(gen=generator)

    if IsVisualStudio2022OrGreater() or IsVisualStudio2019OrGreater():
        generator = generator + " -A x64"

    toolset = context.cmakeToolset
    if toolset is not None:
        toolset = '-T "{toolset}"'.format(toolset=toolset)

    # On MacOS, enable the use of @rpath for relocatable builds.
    osx_rpath = None
    if MacOS():
        osx_rpath = "-DCMAKE_MACOSX_RPATH=ON"

    # We use -DCMAKE_BUILD_TYPE for single-configuration generators
    # (Ninja, make), and --config for multi-configuration generators
    # (Visual Studio); technically we don't need BOTH at the same
    # time, but specifying both is simpler than branching
    config= "Release"

    with CurrentWorkingDirectory(buildDir):
        Run('cmake '
            '-DCMAKE_INSTALL_PREFIX="{libInstDir}" '
            '-DCMAKE_PREFIX_PATH="{libInstDir}" '
            '-DCMAKE_BUILD_TYPE={config} '
            '{osx_rpath} '
            '{generator} '
            '{toolset} '
            '{extraArgs} '
            '"{srcDir}"'
            .format(libInstDir=libInstDir,
                    depsInstDir=context.libInstDir,
                    config=config,
                    srcDir=srcDir,
                    osx_rpath=(osx_rpath or ""),
                    generator=(generator or ""),
                    toolset=(toolset or ""),
                    extraArgs=(" ".join(extraArgs) if extraArgs else "")))
        Run("cmake --build . --config {config} --target install -- {multiproc}"
            .format(config=config,
                    multiproc=FormatMultiProcs(context.numJobs, generator)))

def GetCMakeVersion():
    """
    Returns the CMake version as tuple of integers (major, minor) or
    (major, minor, patch) or None if an error occured while launching cmake and
    parsing its output.
    """

    output_string = GetCommandOutput("cmake --version")
    if not output_string:
        PrintWarning("Could not determine cmake version -- please install it "
                     "and adjust your PATH")
        return None

    # cmake reports, e.g., "... version 3.14.3"
    match = re.search(r"version (\d+)\.(\d+)(\.(\d+))?", output_string)
    if not match:
        PrintWarning("Could not determine cmake version")
        return None

    major, minor, patch_group, patch = match.groups()
    if patch_group is None:
        return (int(major), int(minor))
    else:
        return (int(major), int(minor), int(patch))

def PatchFile(filename, patches, multiLineMatches=False):
    """Applies patches to the specified file. patches is a list of tuples
    (old string, new string)."""
    if multiLineMatches:
        oldLines = [open(filename, 'r').read()]
    else:
        oldLines = open(filename, 'r').readlines()
    newLines = oldLines
    for (oldString, newString) in patches:
        newLines = [s.replace(oldString, newString) for s in newLines]
    if newLines != oldLines:
        PrintInfo("Patching file {filename} (original in {oldFilename})..."
                  .format(filename=filename, oldFilename=filename + ".old"))
        shutil.copy(filename, filename + ".old")
        open(filename, 'w').writelines(newLines)

def InstallDependency(src, dest):
    src = context.libInstDir + src
    dest = context.libInstDir + dest
    try:
        os.remove(dest)
    except:
        if os.path.exists(dest):
            shutil.rmtree(dest)

    os.makedirs(os.path.dirname(dest), exist_ok=True)
    try:
        copy_tree(src, dest)
    except:
        shutil.copyfile(src, dest)

def DownloadFileWithCurl(url, outputFilename):
    # Don't log command output so that curl's progress
    # meter doesn't get written to the log file.
    Run("curl {progress} -L -o {filename} {url}".format(
        progress="-#" if verbosity >= 2 else "-s",
        filename=outputFilename, url=url),
        logCommandOutput=False)

def DownloadFileWithPowershell(url, outputFilename):
    # It's important that we specify to use TLS v1.2 at least or some
    # of the downloads will fail.
    cmd = "powershell [Net.ServicePointManager]::SecurityProtocol = \
            [Net.SecurityProtocolType]::Tls12; \"(new-object \
            System.Net.WebClient).DownloadFile('{url}', '{filename}')\""\
            .format(filename=outputFilename, url=url)

    Run(cmd,logCommandOutput=False)

def DownloadFileWithUrllib(url, outputFilename):
    r = urlopen(url)
    with open(outputFilename, "wb") as outfile:
        outfile.write(r.read())

def DownloadURL(url, context, force, dontExtract = None):
    """Download and extract the archive file at given URL to the
    source directory specified in the context.

    dontExtract may be a sequence of path prefixes that will
    be excluded when extracting the archive.

    Returns the absolute path to the directory where files have
    been extracted."""
    with CurrentWorkingDirectory(context.srcDir):
        # Extract filename from URL and see if file already exists.
        filename = url.split("/")[-1]

        if os.path.exists(filename):
            os.remove(filename)

        if os.path.exists(filename):
            PrintInfo("{0} already exists, skipping download".format(os.path.abspath(filename)))
        else:
            PrintInfo("Downloading {0} to {1}"
                      .format(url, os.path.abspath(filename)))

            # To work around occasional hiccups with downloading from websites
            # (SSL validation errors, etc.), retry a few times if we don't
            # succeed in downloading the file.
            maxRetries = 5
            lastError = None

            # Download to a temporary file and rename it to the expected
            # filename when complete. This ensures that incomplete downloads
            # will be retried if the script is run again.
            tmpFilename = filename + ".tmp"
            if os.path.exists(tmpFilename):
                os.remove(tmpFilename)

            for i in range(maxRetries):
                try:
                    context.downloader(url, tmpFilename)
                    break
                except Exception as e:
                    PrintCommandOutput("Retrying download due to error: {err}\n"
                                       .format(err=e))
                    lastError = e
            else:
                errorMsg = str(lastError)
                if "SSL: TLSV1_ALERT_PROTOCOL_VERSION" in errorMsg:
                    errorMsg += ("\n\n"
                                 "Your OS or version of Python may not support "
                                 "TLS v1.2+, which is required for downloading "
                                 "files from certain websites. This support "
                                 "was added in Python 2.7.9."
                                 "\n\n"
                                 "You can use curl to download dependencies "
                                 "by installing it in your PATH and re-running "
                                 "this script.")
                raise RuntimeError("Failed to download {url}: {err}"
                                   .format(url=url, err=errorMsg))

            shutil.move(tmpFilename, filename)

        # Open the archive and retrieve the name of the top-most directory.
        # This assumes the archive contains a single directory with all
        # of the contents beneath it.
        archive = None
        rootDir = None
        members = None
        try:
            if tarfile.is_tarfile(filename):
                archive = tarfile.open(filename)
                rootDir = archive.getnames()[0].split('/')[0]
                if dontExtract != None:
                    members = (m for m in archive.getmembers()
                               if not any((fnmatch.fnmatch(m.name, p)
                                           for p in dontExtract)))
            elif zipfile.is_zipfile(filename):
                archive = zipfile.ZipFile(filename)
                rootDir = archive.namelist()[0].split('/')[0]
                if dontExtract != None:
                    members = (m for m in archive.getnames()
                               if not any((fnmatch.fnmatch(m, p)
                                           for p in dontExtract)))
            elif rarfile.is_rarfile(filename):
                archive = rarfile.RarFile(filename)
                rootDir = archive.namelist()[0].split('/')[0]
                if dontExtract != None:
                    members = (m for m in archive.getnames()
                               if not any((fnmatch.fnmatch(m, p)
                                           for p in dontExtract)))

            else:
                raise RuntimeError("unrecognized archive file type")

            with archive:
                extractedPath = os.path.abspath(rootDir)
                if force and os.path.isdir(extractedPath):
                    shutil.rmtree(extractedPath)

                if os.path.isdir(extractedPath):
                    PrintInfo("Directory {0} already exists, skipping extract".format(extractedPath))
                else:
                    PrintInfo("Extracting archive to {0}".format(extractedPath))

                    # Extract to a temporary directory then move the contents
                    # to the expected location when complete. This ensures that
                    # incomplete extracts will be retried if the script is run
                    # again.
                    tmpExtractedPath = os.path.abspath("extract_dir")
                    if os.path.isdir(tmpExtractedPath):
                        shutil.rmtree(tmpExtractedPath)

                    archive.extractall(tmpExtractedPath, members=members)

                    shutil.move(os.path.join(tmpExtractedPath, rootDir),
                                extractedPath)
                    shutil.rmtree(tmpExtractedPath)

                return extractedPath
        except Exception as e:
            # If extraction failed for whatever reason, assume the
            # archive file was bad and move it aside so that re-running
            # the script will try downloading and extracting again.
            shutil.move(filename, filename + ".bad")
            raise RuntimeError("Failed to extract archive {filename}: {err}".format(filename=filename, err=e))

############################################################
# 3rd-Party Dependencies

AllDependencies = list()
AllDependenciesByName = dict()

class Dependency(object):
    def __init__(self, name, installer, *files):
        self.name = name
        self.installer = installer
        self.filesToCheck = files

        AllDependencies.append(self)
        AllDependenciesByName.setdefault(name.lower(), self)

    def Exists(self, context):
        return all([os.path.isfile(os.path.join(context.libInstDir, f))
                    for f in self.filesToCheck])

class PythonDependency(object):
    def __init__(self, name, getInstructions, moduleNames):
        self.name = name
        self.getInstructions = getInstructions
        self.moduleNames = moduleNames

    def Exists(self, context):
        # If one of the modules in our list imports successfully, we are good.
        for moduleName in self.moduleNames:
            try:
                pyModule = __import__(moduleName)
                return True
            except:
                pass

        return False

def AnyPythonDependencies(deps):
    return any([type(d) is PythonDependency for d in deps])

############################################################
# zlib

if Linux() or MacOS():
    zlib_verify = "include/zlib.h"
elif Windows():
    zlib_verify = "zlib/include/zlib.h"

ZLIB_URL = "https://zlib.net/zlib1211.zip"

def InstallZlib(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(ZLIB_URL, context, force)):
        RunCMake(context, force, buildArgs)

        if Windows():
            CleanInstall(context, "zlib")

            shutil.copyfile(context.libInstDir + "/bin/zlib.dll", context.libInstDir + "/zlib/bin/zlib.dll")
            shutil.copyfile(context.libInstDir + "/lib/zlib.lib", context.libInstDir + "/zlib/lib/zlib.lib")
            shutil.copyfile(context.libInstDir + "/lib/zlibstatic.lib", context.libInstDir + "/zlib/lib/zlibstatic.lib")
            shutil.copyfile(context.libInstDir + "/include/zconf.h", context.libInstDir + "/zlib/include/zconf.h")
            shutil.copyfile(context.libInstDir + "/include/zlib.h", context.libInstDir + "/zlib/include/zlib.h")

ZLIB = Dependency("zlib", InstallZlib, zlib_verify)

############################################################
# Vulkan

if Linux():
    VULKAN_URL = "https://storage.googleapis.com/dependency_links/vulkansdk-linux-x86_64-1.2.176.1.tar.gz"
elif MacOS():
    VULKAN_URL = "https://sdk.lunarg.com/sdk/download/1.2.176.1/mac/vulkansdk-macos-1.2.176.1.dmg"
else:
    VULKAN_URL = "https://sdk.lunarg.com/sdk/download/1.2.176.1/windows/VulkanSDK-1.2.176.1-Installer.exe"


def InstallVulkan(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(VULKAN_URL, context, force)):

        vkSrc = os.getcwd()

        if Linux():
            CopyFiles(context,     "{sourceDir}/x86_64/bin/*".format(sourceDir=vkSrc),                 "bin/")
            CopyFiles(context,     "{sourceDir}/x86_64/lib/*".format(sourceDir=vkSrc),                 "lib/")
            # CopyDirectory(context, "{sourceDir}/x86_64/lib/pkgconfig".format(sourceDir=vkSrc),         "lib/pkgconfig")
            CopyDirectory(context, "{sourceDir}/x86_64/include/dxc".format(sourceDir=vkSrc),           "include/dxc")
            CopyDirectory(context, "{sourceDir}/x86_64/include/glslang".format(sourceDir=vkSrc),       "include/glslang")
            CopyDirectory(context, "{sourceDir}/x86_64/include/shaderc".format(sourceDir=vkSrc),       "include/shaderc")
            CopyDirectory(context, "{sourceDir}/x86_64/include/spirv".format(sourceDir=vkSrc),         "include/spirv")
            CopyDirectory(context, "{sourceDir}/x86_64/include/spirv_cross".format(sourceDir=vkSrc),   "include/spirv_cross")
            CopyDirectory(context, "{sourceDir}/x86_64/include/SPIRV-Reflect".format(sourceDir=vkSrc), "include/SPIRV-Reflect")
            CopyDirectory(context, "{sourceDir}/x86_64/include/vk_video".format(sourceDir=vkSrc),      "include/vk_video")
            CopyDirectory(context, "{sourceDir}/x86_64/include/vulkan".format(sourceDir=vkSrc),        "include/vulkan")
            CopyDirectory(context, "{sourceDir}/x86_64/share/vulkan".format(sourceDir=vkSrc),          "share/vulkan")
            CopyDirectory(context, "{sourceDir}/x86_64/etc/vulkan".format(sourceDir=vkSrc),            "etc/vulkan")
            CopyFiles(context,     "{sourceDir}/setup-env.sh".format(sourceDir=vkSrc),                 "setup-env.sh")
        elif MacOS():
            vulkan_mac = subprocess.call("{sourceDir}/vulkansdk-macos-1.2.176.1.dmg".format(sourceDir=vkSrc), stdout=subprocess.PIPE, shell=True)
        else:
            vulkan_win = subprocess.call("{sourceDir}/VulkanSDK-1.2.176.1-Installer.exe".format(sourceDir=vkSrc), stdout=subprocess.PIPE, shell=True)

VULKAN = Dependency("vulkan", InstallVulkan, "include/vulkan/vulkan.h")

############################################################
# Arnold Render

if Linux():
    ARNOLD_URL = "https://storage.googleapis.com/dependency_links/Arnold-6.2.1.0-linux.tgz"
elif MacOS():
    ARNOLD_URL = "https://www.arnoldrenderer.com/arnold/download/product-download/download/?id=3936&token=kaRw3ZFLx2jAk6r1"
else:
    ARNOLD_URL = "https://storage.googleapis.com/dependency_links/Arnold-6.2.1.0-windows.zip"

def extract_arnold_because_hmm(tar_url, extract_path='.'):
    tar = tarfile.open(tar_url, 'r')
    for item in tar:
        tar.extract(item, extract_path)
        if item.name.find(".tgz") != -1 or item.name.find(".tar") != -1:
            extract_arnold_because_hmm(item.name, "./" + item.name[:item.name.rfind('/')])

def InstallArnold(context, force, buildArgs):
    arnoldSrc = context.libInstDir + '/build_env/source/arnold'
    if not os.path.exists(arnoldSrc):
        os.makedirs(arnoldSrc)
    with CurrentWorkingDirectory(arnoldSrc):
        if len(os.listdir(arnoldSrc)) == 0:
            if Linux():
                subprocess.call("wget {} -O {}".format(ARNOLD_URL, arnoldSrc + "/Arnold-6.2.1.0-linux.tgz"), stdout=subprocess.DEVNULL, shell=True)

        extract_arnold_because_hmm(arnoldSrc + "/Arnold-6.2.1.0-linux.tgz")

        copy_tree(arnoldSrc + "/include",       context.libInstDir + "/include")
        copy_tree(arnoldSrc + "/bin",           context.libInstDir + "/bin")
        copy_tree(arnoldSrc + "/license",       context.libInstDir + "/license")
        copy_tree(arnoldSrc + "/materialx",     context.libInstDir + "/materialx")
        copy_tree(arnoldSrc + "/osl",           context.libInstDir + "/osl")
        copy_tree(arnoldSrc + "/plugins",       context.libInstDir + "/plugins")
        copy_tree(arnoldSrc + "/python/arnold", context.libInstDir + "/python/arnold")

ARNOLD = Dependency("arnold", InstallArnold, "include/ai_version.h")

############################################################
# Cycles Render

if Linux():
    verify_cycles = "include/bvh/bvh.h"
    OPENIMAGE_DENOISE_URL = "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.1/oidn-1.4.1.src.tar.gz"
else:
    verify_cycles = "Cycles/include/bvh/bvh.h"
    OPENIMAGE_DENOISE_URL = "https://github.com/OpenImageDenoise/oidn/releases/download/v1.4.1/oidn-1.4.1.src.zip"

CYCLES_URL = "https://storage.googleapis.com/dependency_links/Cycles-1.10-KRAKEN.zip"

def InstallCycles(context, force, buildArgs):
    # -------------------------------------------------------------------------------- OPENIMAGEDENOISE -----
    with CurrentWorkingDirectory(DownloadURL(OPENIMAGE_DENOISE_URL, context, force)):
        if Windows():
            # OpenImageDenoise isn't able to find tbb so we set it here
            buildArgs.append('-DTBB_ROOT={tbb}'.format(tbb=context.libInstDir + "/tbb"))
            # OpenImageDenoise also needs ISPC (high-performance SIMD on the CPU & GPU) to be installed.
            buildArgs.append('-DISPC_EXECUTABLE={ispc}'.format(ispc=context.libInstDir + "/ispc/bin/ispc.exe"))
        RunCMake(context, force, buildArgs)        

    # ----------------------------------------------------------------------------------------- OPENJPEG -----
    with CurrentWorkingDirectory(DownloadURL("https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.4.0.tar.gz", context, force)):
        RunCMake(context, force, buildArgs)

    # CYCLES RENDERER
    with CurrentWorkingDirectory(DownloadURL(CYCLES_URL, context, force)):
        
        if Linux():
            buildArgs.append("-DUSE_OPENGL=ON")
            buildArgs.append("-DOPENEXR_HALF_LIBRARY={}".format(context.libInstDir + "/lib64/libImath.so"))
            buildArgs.append("-DOPENEXR_ILMIMF_LIBRARY={}".format(context.libInstDir + "/lib64/libImath.so"))
        elif Windows():
            buildArgs.append("-DUSE_OPENGL=ON")
            buildArgs.append('-DBOOST_ROOT={boost}'.format(boost=context.libInstDir + "/boost"))
            buildArgs.append('-DTBB_ROOT={tbb}'.format(tbb=context.libInstDir + "/tbb"))
            buildArgs.append('-DOPENVDB_ROOT={vdb}'.format(vdb=context.libInstDir + "/openvdb"))
            buildArgs.append('-DLLVM_ROOT={llvm}'.format(llvm=context.libInstDir + "/llvm"))
            buildArgs.append('-DGLEW_ROOT={glew}'.format(glew=context.libInstDir + "/glew"))
            buildArgs.append('-DGLEW_LIBRARY={glew}'.format(glew=context.libInstDir + "/glew/lib/Release/x64/glew32.lib"))
            buildArgs.append('-DGLEW_INCLUDE_DIR={glew}'.format(glew=context.libInstDir + "/glew/include/GL"))
            buildArgs.append('-DOPENCOLORIO_ROOT={color}'.format(color=context.libInstDir + "/opencolorio"))
            buildArgs.append('-DOPENCOLORIO_LIBRARIES={color}'.format(color=context.libInstDir + "/opencolorio/lib/OpenColorIO_2_0.lib"))
            buildArgs.append('-Dpugixml_ROOT={pugixml_root_path}'.format(pugixml_root_path=context.libInstDir + "/pugixml"))
            buildArgs.append('-DPUGIXML_LIBRARY={pug}'.format(pug=context.libInstDir + "/pugixml/lib/pugixml.lib"))
            buildArgs.append('-DPUGIXML_INCLUDE_DIR={pug}'.format(pug=context.libInstDir + "/pugixml/include"))
            buildArgs.append("-DOPENEXR_HALF_LIBRARY={}".format(context.libInstDir + "/openexr/lib/Imath-3_0.lib"))
            buildArgs.append("-DOPENEXR_ILMIMF_LIBRARY={}".format(context.libInstDir + "/openexr/lib/Imath-3_0.lib"))

        RunCMake(context, force, buildArgs)

        if Linux():
            copy_tree(context.buildDir + "/Cycles-1.10-KRAKEN/bin", context.libInstDir + "/bin")
            copy_tree(context.buildDir + "/Cycles-1.10-KRAKEN/lib", context.libInstDir + "/lib")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/app",                 context.libInstDir + "/include/app")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/blender",             context.libInstDir + "/include/blender")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/bvh",                 context.libInstDir + "/include/bvh")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/device",              context.libInstDir + "/include/device")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/graph",               context.libInstDir + "/include/graph")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/kernel",              context.libInstDir + "/include/kernel")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/render",              context.libInstDir + "/include/render")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/subd",                context.libInstDir + "/include/subd")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/util",                context.libInstDir + "/include/util")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/atomic",      context.libInstDir + "/include/atomic")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/clew",        context.libInstDir + "/include/clew")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/cuew",        context.libInstDir + "/include/cuew")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/libc_compat", context.libInstDir + "/include/libc_compat")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/numaapi",     context.libInstDir + "/include/numaapi")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/sky",         context.libInstDir + "/include/sky")
        elif Windows():
            copy_tree(context.buildDir + "/Cycles-1.10-KRAKEN/bin", context.libInstDir + "/Cycles/bin")
            copy_tree(context.buildDir + "/Cycles-1.10-KRAKEN/lib", context.libInstDir + "/Cycles/lib")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/app",                 context.libInstDir + "/Cycles/include/app")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/blender",             context.libInstDir + "/Cycles/include/blender")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/bvh",                 context.libInstDir + "/Cycles/include/bvh")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/device",              context.libInstDir + "/Cycles/include/device")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/graph",               context.libInstDir + "/Cycles/include/graph")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/kernel",              context.libInstDir + "/Cycles/include/kernel")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/render",              context.libInstDir + "/Cycles/include/render")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/subd",                context.libInstDir + "/Cycles/include/subd")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/src/util",                context.libInstDir + "/Cycles/include/util")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/atomic",      context.libInstDir + "/Cycles/include/atomic")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/clew",        context.libInstDir + "/Cycles/include/clew")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/cuew",        context.libInstDir + "/Cycles/include/cuew")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/libc_compat", context.libInstDir + "/Cycles/include/libc_compat")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/numaapi",     context.libInstDir + "/Cycles/include/numaapi")
            CopyFilesHeaders(context.srcDir + "/Cycles-1.10-KRAKEN/third_party/sky",         context.libInstDir + "/Cycles/include/sky")

CYCLES = Dependency("cycles", InstallCycles, verify_cycles)

############################################################
# Mitsuba Render

def InstallMitsuba(context, force, buildArgs):

    mitsubaSrc = context.libInstDir + '/build_env/source/mitsuba'
    # if not os.path.exists(mitsubaSrc):
    #     os.makedirs(mitsubaSrc)
    # with CurrentWorkingDirectory(mitsubaSrc):
    #     if len(os.listdir(mitsubaSrc)) == 0:
    #         subprocess.call("git clone https://github.com/mitsuba-renderer/mitsuba2.git .", stdout=subprocess.DEVNULL, shell=True)
    #         # subprocess.call("git submodule update --init --recursive", stdout=subprocess.DEVNULL, shell=True)
        
    #     buildArgs.append("-DMTS_ENABLE_GUI=OFF")
    #     buildArgs.append("-DMTS_ENABLE_PYTHON=OFF")

    #     RunCMake(context, force, buildArgs)

MITSUBA = Dependency("mitsuba", InstallMitsuba, "include/bvh/bvh.h")

############################################################
# AMD ProRender

PRORENDER_URL = "https://github.com/GPUOpen-LibrariesAndSDKs/RadeonProRenderSDK/archive/refs/tags/v2.2.3.tar.gz"

def InstallProRender(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(PRORENDER_URL, context, force)):
        if Linux():
            if distro.linux_distribution()[0] == 'Fedora' or distro.linux_distribution()[0] == 'centos':
                copy_tree(context.srcDir + "/RadeonProRenderSDK-2.2.3/RadeonProRender/binCentOS7", context.libInstDir + "/bin")
            else:
                copy_tree(context.srcDir + "/RadeonProRenderSDK-2.2.3/RadeonProRender/binUbuntu18", context.libInstDir + "/bin")
        
        copy_tree(context.srcDir + "/RadeonProRenderSDK-2.2.3/RadeonProRender/inc",      context.libInstDir + "/include")
        copy_tree(context.srcDir + "/RadeonProRenderSDK-2.2.3/RadeonProRender/rprTools", context.libInstDir + "/include/rprTools")

PRORENDER = Dependency("prorender", InstallProRender, "include/RadeonProRender.h")

############################################################
# Pixar Renderman

if Linux():
    rman_verify = "/opt/pixar/RenderManProServer-23.5/include/prmanapi.h"
    RENDERMAN_URL = "https://storage.googleapis.com/dependency_links/rman23.5.tar.gz"
elif MacOS():
    rman_verify = "include/prmanapi.h"
    RENDERMAN_URL = ""
else:
    rman_verify = "include/prmanapi.h"
    RENDERMAN_URL = "https://storage.googleapis.com/dependency_links/RenderMan-24.0.0win.zip"

def InstallRenderman(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(RENDERMAN_URL, context, force)):

        prmanSrc = os.getcwd()
        if Linux():
            # Use 'sudo locate libcrypto.so.1.0.0' && 'sudo locate libssl.so.1.0.0' to find these
            my_crypto = "/snap/core18/1997/usr/lib/x86_64-linux-gnu/libcrypto.so.1.0.0"
            my_ssl    = "/snap/core18/1997/usr/lib/x86_64-linux-gnu/libssl.so.1.0.0"
            # -----------------------------------------------------------------------------------------------------
            prman_crypto = prmanSrc + "/pixar/RenderMan-Installer-ncr-23.5/lib/3rdparty/Qt-5.6.1/lib/libcrypto.so"
            prman_ssl    = prmanSrc + "/pixar/RenderMan-Installer-ncr-23.5/lib/3rdparty/Qt-5.6.1/lib/libssl.so"
            if not os.path.exists(prman_crypto):
                os.symlink(my_crypto, prman_crypto)
            if not os.path.exists(prman_ssl):
                os.symlink(my_ssl, prman_ssl)
            subprocess.call("sudo " + prmanSrc + "/pixar/RenderMan-Installer-ncr-23.5/bin/RenderManInstaller", stdout=subprocess.DEVNULL, shell=True)

RENDERMAN = Dependency("prman", InstallRenderman, rman_verify)

############################################################
# boost

if MacOS():
    BOOST_URL = "https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz"
    BOOST_VERSION_FILE = "include/boost/version.hpp"
elif Linux():
    BOOST_URL = "https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz"
    BOOST_VERSION_FILE = "include/boost/version.hpp"
elif Windows():
    # The default installation of boost on Windows puts headers in a versioned
    # subdirectory, which we have to account for here. In theory, specifying
    # "layout=system" would make the Windows install match Linux/MacOS, but that
    # causes problems for other dependencies that look for boost.
    BOOST_URL = "https://storage.googleapis.com/dependency_links/boost_1_76_0.zip"
    BOOST_VERSION_FILE = "include/boost-1_76/boost/version.hpp"

def InstallBoost_Helper(context, force, buildArgs):
    # Documentation files in the boost archive can have exceptionally
    # long paths. This can lead to errors when extracting boost on Windows,
    # since paths are limited to 260 characters by default on that platform.
    # To avoid this, we skip extracting all documentation.
    #
    # For some examples, see: https://svn.boost.org/trac10/ticket/11677
    dontExtract = ["*/doc/*", "*/libs/*/doc/*"]

    with CurrentWorkingDirectory(DownloadURL(BOOST_URL, context, force)):
        bootstrap = "bootstrap.bat" if Windows() else "./bootstrap.sh"

        if Windows():
            Run('{bootstrap} --prefix="{libInstDir}"'.format(bootstrap=bootstrap, libInstDir=context.libInstDir + "/boost"))
        else:
            Run('{bootstrap} --prefix="{libInstDir}"'.format(bootstrap=bootstrap, libInstDir=context.libInstDir + "/boost"))

        # b2 supports at most -j64 and will error if given a higher value.
        num_procs = min(64, context.numJobs)

        boost_build_options = [
            '--prefix="{libInstDir}"'.format(libInstDir=context.libInstDir),
            '--build-dir="{buildDir}"'.format(buildDir=context.buildDir),
            '-j{procs}'.format(procs=num_procs),
            'address-model=64',
            'link=shared',
            'runtime-link=shared',
            'threading=multi',
            'variant={variant}'.format(variant="release"),
            'debug-symbols=on',
            '--with-atomic',
            '--with-program_options',
            '--with-regex',
            '--with-python',
            '--with-date_time',
            '--with-system',
            '--with-thread',
            '--with-iostreams',
            "--with-filesystem",
            "--with-wave", # For cycles
            '-sNO_BZIP2=1',
        ]

        pythonInfo = GetPythonInfo()

        if Windows():
            boost_user_config_jam = "{buildDir}/boost.user-config.jam".format(buildDir=context.buildDir)
            boost_build_options.append('--user-config={boost_user_config_jam}'.format(boost_user_config_jam=boost_user_config_jam).replace('\\', '/'))
            PYTHON_SHORT_VERSION =  pythonInfo["python_version"]
            PYTHON_BINARY        =  pythonInfo["python_exe"]
            PYTHON_INCLUDE       =  pythonInfo["python_include"]
            PYTHON_LIB           =  pythonInfo["python_lib"]

            with open(boost_user_config_jam, 'w') as patch_boost:
                patch_boost.write('using python : {} : {}\n'.format(PYTHON_SHORT_VERSION, PYTHON_BINARY))
                patch_boost.write(' : {}\n'.format(PYTHON_INCLUDE))
                patch_boost.write(' : {}\n'.format(PYTHON_LIB))
                patch_boost.write(';')
                patch_boost.close()
        else:
            # While other platforms want the complete executable path
            pythonPath = pythonInfo[0]
        
        if not Windows():
            # This is the only platform-independent way to configure these
            # settings correctly and robustly for the Boost jam build system.
            # There are Python config arguments that can be passed to bootstrap 
            # but those are not available in boostrap.bat (Windows) so we must 
            # take the following approach:
            projectPath = 'python-config.jam'
            with open(projectPath, 'w') as projectFile:
                # Note that we must escape any special characters, like 
                # backslashes for jam, hence the mods below for the path 
                # arguments. Also, if the path contains spaces jam will not
                # handle them well. Surround the path parameters in quotes.
                line = 'using python : %s : "%s" : "%s" ;\n' % (pythonInfo[3], 
                        pythonPath.replace('\\', '\\\\'), 
                        pythonInfo[2].replace('\\', '\\\\'))
                projectFile.write(line)
            boost_build_options.append("--user-config=python-config.jam")


        # if force:
        boost_build_options.append("-a")

        if Windows():
            # toolset parameter for Visual Studio documented here:
            # https://github.com/boostorg/build/blob/develop/src/tools/msvc.jam
            if context.cmakeToolset == "v143":
                boost_build_options.append("toolset=msvc-14.3")
            if context.cmakeToolset == "v142":
                boost_build_options.append("toolset=msvc-14.2")
            elif context.cmakeToolset == "v141":
                boost_build_options.append("toolset=msvc-14.1")
            elif context.cmakeToolset == "v140":
                boost_build_options.append("toolset=msvc-14.0")
            elif IsVisualStudio2022OrGreater():
                boost_build_options.append("toolset=msvc-14.3")
            elif IsVisualStudio2019OrGreater():
                boost_build_options.append("toolset=msvc-14.2")
            else:
                boost_build_options.append("toolset=msvc-14.3")

        if MacOS():
            # Must specify toolset=clang to ensure install_name for boost
            # libraries includes @rpath
            boost_build_options.append("toolset=clang")

        # Add on any user-specified extra arguments.
        # boost_build_options += buildArgs

        build_command = "b2" if Windows() else "./b2"
        Run('{build_command} {options} install'.format(build_command=build_command, options=" ".join(boost_build_options)))

def InstallBoost(context, force, buildArgs):
    # Boost's build system will install the version.hpp header before
    # building its libraries. We make sure to remove it in case of
    # any failure to ensure that the build script detects boost as a
    # dependency to build the next time it's run.
    try:
        InstallBoost_Helper(context, force, buildArgs)
    except:
        versionHeader = os.path.join(context.libInstDir + "/boost", BOOST_VERSION_FILE)
        if os.path.isfile(versionHeader):
            try: os.remove(versionHeader)
            except: pass
        raise

BOOST = Dependency("boost", InstallBoost, BOOST_VERSION_FILE)

############################################################
# Intel TBB

if Windows():
    tbb_verify = "tbb/include/tbb/tbb.h"
    TBB_URL = "https://storage.googleapis.com/dependency_links/tbb2019_OSS.zip"
else:
    tbb_verify = "include/tbb/tbb.h"
    TBB_URL = "https://github.com/oneapi-src/oneTBB/releases/download/2019_U9/tbb2019_20191006oss_lin.tgz"

def InstallTBB(context, force, buildArgs):
    if Windows():
        InstallTBB_Windows(context, force, buildArgs)
    elif Linux() or MacOS():
        InstallTBB_LinuxOrMacOS(context, force, buildArgs)

def InstallTBB_Windows(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(TBB_URL, context, force)):
        # On Windows, we simply copy headers and pre-built DLLs to
        # the appropriate location.

        # if buildArgs:
        #     PrintWarning("Ignoring build arguments {}, TBB is "
        #                  "not built from source on this platform."
        #                  .format(buildArgs))

        copy_tree(context.buildDir.rsplit('/',1)[0] + "/source/tbb2019_OSS/lib/intel64/vc14", context.libInstDir + "/tbb/lib")
        copy_tree(context.buildDir.rsplit('/',1)[0] + "/source/tbb2019_OSS/bin/intel64/vc14", context.libInstDir + "/tbb/bin")
        copy_tree(context.buildDir.rsplit('/',1)[0] + "/source/tbb2019_OSS/include/serial", context.libInstDir + "/tbb/include/serial")
        copy_tree(context.buildDir.rsplit('/',1)[0] + "/source/tbb2019_OSS/include/tbb", context.libInstDir + "/tbb/include/tbb")

def InstallTBB_LinuxOrMacOS(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(TBB_URL, context, force)):
        # Note: TBB installation fails on OSX when cuda is installed, a
        # suggested fix:
        # https://github.com/spack/spack/issues/6000#issuecomment-358817701
        # if MacOS():
        #     PatchFile("build/macos.inc",
        #             [("shell clang -v ", "shell clang --version ")])
        # # TBB does not support out-of-source builds in a custom location.
        # Run('make -j{procs} {buildArgs}'
        #     .format(procs=context.numJobs,
        #             buildArgs=" ".join(buildArgs)))

        # Install both release and debug builds. USD requires the debug
        # libraries when building in debug mode, and installing both
        # makes it easier for users to install dependencies in some
        # location that can be shared by both release and debug USD
        # builds. Plus, the TBB build system builds both versions anyway.
        copy_tree(context.buildDir.rsplit('/',1)[0] + "/source/tbb2019_20191006oss_lin/tbb2019_20191006oss/lib/intel64/gcc4.8", context.libInstDir + "/lib")
        # CopyFiles(context, "lib/intel64/gcc4.8/*.*", )
        copy_tree(context.buildDir.rsplit('/',1)[0] + "/source/tbb2019_20191006oss_lin/tbb2019_20191006oss/include/serial", context.libInstDir + "/include/serial")
        copy_tree(context.buildDir.rsplit('/',1)[0] + "/source/tbb2019_20191006oss_lin/tbb2019_20191006oss/include/tbb", context.libInstDir + "/include/tbb")

TBB = Dependency("TBB", InstallTBB, tbb_verify)

############################################################
# JPEG

if Windows():
    JPEG_URL = "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.0.zip"
else:
    JPEG_URL = "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/refs/tags/2.1.0.tar.gz"

def InstallJPEG(context, force, buildArgs):
    if Windows():
        InstallJPEG_Turbo(context, force, buildArgs)
    else:
        InstallJPEG_Lib(context, force, buildArgs)

def InstallJPEG_Turbo(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(JPEG_URL, context, force)):
        RunCMake(context, force, buildArgs)

        if Windows():
            InstallDependency("/bin/cjpeg.exe",            "/jpeg/bin/cjpeg.exe")
            InstallDependency("/bin/djpeg.exe",            "/jpeg/bin/djpeg.exe")
            InstallDependency("/bin/jpeg62.dll",           "/jpeg/bin/jpeg62.dll")
            InstallDependency("/bin/jpegtran.exe",         "/jpeg/bin/jpegtran.exe")
            InstallDependency("/bin/rdjpgcom.exe",         "/jpeg/bin/rdjpgcom.exe")
            InstallDependency("/bin/tjbench.exe",          "/jpeg/bin/tjbench.exe")
            InstallDependency("/bin/turbojpeg.dll",        "/jpeg/bin/turbojpeg.dll")
            InstallDependency("/bin/wrjpgcom.exe",         "/jpeg/bin/wrjpgcom.exe")
            InstallDependency("/lib/jpeg.lib",             "/jpeg/lib/jpeg.lib")
            InstallDependency("/lib/jpeg-static.lib",      "/jpeg/lib/jpeg-static.lib")
            InstallDependency("/lib/turbojpeg.lib",        "/jpeg/lib/turbojpeg.lib")
            InstallDependency("/lib/turbojpeg-static.lib", "/jpeg/lib/turbojpeg-static.lib")
            InstallDependency("/include/jconfig.h",        "/jpeg/include/jconfig.h")
            InstallDependency("/include/jerror.h",         "/jpeg/include/jerror.h")
            InstallDependency("/include/jmorecfg.h",       "/jpeg/include/jmorecfg.h")
            InstallDependency("/include/jpeglib.h",        "/jpeg/include/jpeglib.h")
            InstallDependency("/include/turbojpeg.h",      "/jpeg/include/turbojpeg.h")

def InstallJPEG_Lib(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(JPEG_URL, context, force)):
        RunCMake(context, force, buildArgs)
        # Run('./configure --prefix="{libInstDir}" '
        #     '--disable-static --enable-shared '.format(libInstDir=context.libInstDir))
        #     # '{buildArgs}'
        #     # .format(libInstDir=context.libInstDir,
        #     #         buildArgs=" ".join(buildArgs)))
        # Run('make -j{procs} install'
        #     .format(procs=context.numJobs))

JPEG = Dependency("JPEG", InstallJPEG, "include/jpeglib.h")

############################################################
# TIFF

if Windows():
    TIFF_URL = "https://gitlab.com/libtiff/libtiff/-/archive/v4.3.0rc1/libtiff-v4.3.0rc1.zip"
else:
    TIFF_URL = "https://gitlab.com/libtiff/libtiff/-/archive/v4.3.0rc1/libtiff-v4.3.0rc1.tar.gz"

def InstallTIFF(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(TIFF_URL, context, force)):
        # libTIFF has a build issue on Windows where tools/tiffgt.c
        # unconditionally includes unistd.h, which does not exist.
        # To avoid this, we patch the CMakeLists.txt to skip building
        # the tools entirely. We do this on Linux and MacOS as well
        # to avoid requiring some GL and X dependencies.
        #
        # We also need to skip building tests, since they rely on
        # the tools we've just elided.
        PatchFile("CMakeLists.txt",
                   [("add_subdirectory(tools)", "# add_subdirectory(tools)"),
                    ("add_subdirectory(test)", "# add_subdirectory(test)")])

        # The libTIFF CMakeScript says the ld-version-script
        # functionality is only for compilers using GNU ld on
        # ELF systems or systems which provide an emulation; therefore
        # skipping it completely on mac and windows.
        if MacOS() or Windows():
            extraArgs = ["-Dld-version-script=OFF"]
        else:
            extraArgs = []
        extraArgs += buildArgs
        RunCMake(context, force, extraArgs)

        if Windows():
            InstallDependency("/bin/tiff.dll",            "/tiff/bin/tiff.dll")
            InstallDependency("/bin/tiffxx.dll",          "/tiff/bin/tiffxx.dll")
            InstallDependency("/lib/tiff.lib",            "/tiff/lib/tiff.lib")
            InstallDependency("/include/tiff.h",          "/tiff/include/tiff.h")
            InstallDependency("/include/tiffconf.h",      "/tiff/include/tiffconf.h")
            InstallDependency("/include/tiffio.h",        "/tiff/include/tiffio.h")
            InstallDependency("/include/tiffio.hxx",      "/tiff/include/tiffio.hxx")
            InstallDependency("/include/tiffvers.h",      "/tiff/include/tiffvers.h")

TIFF = Dependency("TIFF", InstallTIFF, "include/tiff.h")

############################################################
# PNG

if Windows():
    verify_png="png/include/png.h"
    PNG_URL = "https://github.com/glennrp/libpng/archive/refs/tags/v1.6.35.zip"
else:
    verify_png="include/png.h"
    PNG_URL = "https://github.com/glennrp/libpng/archive/refs/tags/v1.6.35.tar.gz"

def InstallPNG(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(PNG_URL, context, force)):

        if Windows():
            buildArgs = ['-DZLIB_ROOT="{zlibRoot}"'.format(zlibRoot=context.libInstDir + "/zlib")]
            buildArgs.append('-DZLIB_INCLUDE_DIR="{libInst}"'.format(libInst=context.libInstDir + "/zlib/include"))

        RunCMake(context, force, buildArgs)

        if Windows():
            InstallDependency("/bin/libpng16.dll",        "/png/bin/libpng16.dll")
            InstallDependency("/bin/pngfix.exe",          "/png/bin/pngfix.exe")
            InstallDependency("/bin/png-fix-itxt.exe",    "/png/bin/png-fix-itxt.exe")
            InstallDependency("/lib/libpng16.lib",        "/png/lib/libpng16.lib")
            InstallDependency("/lib/libpng16_static.lib", "/png/lib/libpng16_static.lib")
            InstallDependency("/lib/libpng",              "/png/lib/libpng")
            InstallDependency("/include/pnglibconf.h",    "/png/include/pnglibconf.h")
            InstallDependency("/include/pngconf.h",       "/png/include/pngconf.h")
            InstallDependency("/include/png.h",           "/png/include/png.h")
            InstallDependency("/include/libpng16",        "/png/include/libpng16")

PNG = Dependency("PNG", InstallPNG, verify_png)

############################################################
# IlmBase/OpenEXR

if Windows():
    verify_openexr = "openexr/include/OpenEXR/ImfVersion.h"
    OPENEXR_URL = "https://github.com/AcademySoftwareFoundation/openexr/archive/refs/tags/v3.0.1.zip"
    IMATH_URL   = "https://github.com/AcademySoftwareFoundation/Imath/archive/refs/tags/v3.0.1.zip"
else:
    verify_openexr = "include/OpenEXR/ImfVersion.h"
    OPENEXR_URL = "https://github.com/AcademySoftwareFoundation/openexr/archive/refs/tags/v3.0.1.tar.gz"
    IMATH_URL = "https://github.com/AcademySoftwareFoundation/Imath/archive/refs/tags/v3.0.1.tar.gz"

def InstallOpenEXR(context, force, buildArgs):
    srcDir = DownloadURL(OPENEXR_URL, context, force)
    # OPENEXR
    exrDir = DownloadURL(OPENEXR_URL, context, force)
    openexrSrcDir = os.path.join(exrDir, "")
    with CurrentWorkingDirectory(openexrSrcDir):
        RunCMake(context, force,
                 ['-DIMATH_PACKAGE_PREFIX="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr"),
                  '-DOPENEXR_PACKAGE_PREFIX="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr")] + buildArgs)
    # IMATH
    mathDir = DownloadURL(IMATH_URL, context, force)
    imathSrcDir = os.path.join(mathDir, "")
    with CurrentWorkingDirectory(imathSrcDir):
        RunCMake(context, force,
                 ['-DIMATH_PACKAGE_PREFIX="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr"),
                  '-DOPENEXR_PACKAGE_PREFIX="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr")] + buildArgs)

    if Windows():
        InstallDependency("/bin/exr2aces.exe",        "/openexr/bin/exr2aces.exe")
        InstallDependency("/bin/exrenvmap.exe",       "/openexr/bin/exrenvmap.exe")
        InstallDependency("/bin/exrheader.exe",       "/openexr/bin/exrheader.exe")
        InstallDependency("/bin/exrmakepreview.exe",  "/openexr/bin/exrmakepreview.exe")
        InstallDependency("/bin/exrmaketiled.exe",    "/openexr/bin/exrmaketiled.exe")
        InstallDependency("/bin/exrmultipart.exe",    "/openexr/bin/exrmultipart.exe")
        InstallDependency("/bin/exrstdattr.exe",      "/openexr/bin/exrstdattr.exe")
        InstallDependency("/bin/Iex-3_0.dll",         "/openexr/bin/Iex-3_0.dll")
        InstallDependency("/bin/IlmThread-3_0.dll",   "/openexr/bin/IlmThread-3_0.dll")
        InstallDependency("/bin/Imath-3_0.dll",       "/openexr/bin/Imath-3_0.dll")
        InstallDependency("/bin/OpenEXR-3_0.dll",     "/openexr/bin/OpenEXR-3_0.dll")
        InstallDependency("/bin/OpenEXRUtil-3_0.dll", "/openexr/bin/OpenEXRUtil-3_0.dll")
        InstallDependency("/lib/Iex-3_0.lib",         "/openexr/lib/Iex-3_0.lib")
        InstallDependency("/lib/IlmThread-3_0.lib",   "/openexr/lib/IlmThread-3_0.lib")
        InstallDependency("/lib/Imath-3_0.lib",       "/openexr/lib/Imath-3_0.lib")
        InstallDependency("/lib/OpenEXR-3_0.lib",     "/openexr/lib/OpenEXR-3_0.lib")
        InstallDependency("/lib/OpenEXRUtil-3_0.lib", "/openexr/lib/OpenEXRUtil-3_0.lib")
        InstallDependency("/include/Imath",           "/openexr/include/Imath")
        InstallDependency("/include/OpenEXR",         "/openexr/include/OpenEXR")

OPENEXR = Dependency("OpenEXR", InstallOpenEXR, verify_openexr)

############################################################
# Ptex

if Windows():
    verify_ptex = "ptex/include/PtexVersion.h"
else:
    verify_ptex = "include/PtexVersion.h"

PTEX_URL = "https://github.com/wdas/ptex/archive/refs/tags/v2.4.0.zip"

def InstallPtex(context, force, buildArgs):

    buildArgs = [
        '-DPTEX_VER="v2.4.0"',
    ]

    if Windows():
        InstallPtex_Windows(context, force, buildArgs)
    else:
        InstallPtex_LinuxOrMacOS(context, force, buildArgs)

def InstallPtex_Windows(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(PTEX_URL, context, force)):
        # Ptex has a bug where the import library for the dynamic library and
        # the static library both get the same name, Ptex.lib, and as a
        # result one clobbers the other. We hack the appropriate CMake
        # file to prevent that. Since we don't need the static library we'll
        # rename that.
        #
        # In addition src\tests\CMakeLists.txt adds -DPTEX_STATIC to the
        # compiler but links tests against the dynamic library, causing the
        # links to fail. We patch the file to not add the -DPTEX_STATIC
        # PatchFile('src\\ptex\\CMakeLists.txt',
        #           [("set_target_properties(Ptex_static PROPERTIES OUTPUT_NAME Ptex)",
        #             "set_target_properties(Ptex_static PROPERTIES OUTPUT_NAME Ptexs)")])
        # PatchFile('src\\tests\\CMakeLists.txt',
        #           [("add_definitions(-DPTEX_STATIC)",
        #             "# add_definitions(-DPTEX_STATIC)")])

        buildArgs.append("-DZLIB_ROOT={}".format(context.libInstDir + "/zlib"))

        RunCMake(context, force, buildArgs)

        InstallDependency("/bin/ptxinfo.exe",         "/ptex/bin/ptxinfo.exe")
        InstallDependency("/lib/Ptex.dll",            "/ptex/lib/Ptex.dll")
        InstallDependency("/lib/Ptex.lib",            "/ptex/lib/Ptex.lib")
        InstallDependency("/lib/Ptexs.lib",           "/ptex/lib/Ptexs.lib")
        InstallDependency("/include/PtexVersion.h",   "/ptex/include/PtexVersion.h")
        InstallDependency("/include/PtexUtils.h",     "/ptex/include/PtexUtils.h")
        InstallDependency("/include/Ptexture.h",      "/ptex/include/Ptexture.h")
        InstallDependency("/include/PtexInt.h",       "/ptex/include/PtexInt.h")
        InstallDependency("/include/PtexHalf.h",      "/ptex/include/PtexHalf.h")

def InstallPtex_LinuxOrMacOS(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(PTEX_URL, context, force)):
        RunCMake(context, force, buildArgs)

PTEX = Dependency("Ptex", InstallPtex, verify_ptex)

############################################################
# BLOSC (Compression used by OpenVDB)

# Using latest blosc since neither the version OpenVDB recommends
# (1.5) nor the version we test against (1.6.1) compile on Mac OS X
# Sierra (10.12) or Mojave (10.14).
if Windows():
    verify_blosc = "blosc/include/blosc.h"
else:
    verify_blosc = "include/blosc.h"

BLOSC_URL = "https://github.com/Blosc/c-blosc/archive/refs/tags/v1.21.0.zip"

def InstallBLOSC(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(BLOSC_URL, context, force)):
        RunCMake(context, force, buildArgs)

        if Windows():
            InstallDependency("/lib/blosc.lib",          "/blosc/lib/blosc.lib")
            InstallDependency("/lib/libblosc.lib",       "/blosc/lib/libblosc.lib")
            InstallDependency("/include/blosc.h",        "/blosc/include/blosc.h")
            InstallDependency("/include/blosc-export.h", "/blosc/include/blosc-export.h")

BLOSC = Dependency("Blosc", InstallBLOSC, verify_blosc)

############################################################
# OpenVDB

# Using version 6.1.0 since it has reworked its CMake files so that
# there are better options to not compile the OpenVDB binaries and to
# not require additional dependencies such as GLFW. Note that version
# 6.1.0 does require CMake 3.3 though.

if Windows():
    check_ovdb="openvdb/include/openvdb/openvdb.h"
else:
    check_ovdb="include/openvdb/openvdb.h"

OPENVDB_URL = "https://github.com/AcademySoftwareFoundation/openvdb/archive/refs/tags/v8.1.0.tar.gz"

def InstallOpenVDB(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(OPENVDB_URL, context, force)):
        extraArgs = [
            '-DOPENVDB_BUILD_PYTHON_MODULE=OFF',
            '-DOPENVDB_BUILD_BINARIES=OFF',
            '-DOPENVDB_BUILD_UNITTESTS=OFF'
        ]

        # Make sure to use boost installed by the build script and not any
        # system installed boost
        extraArgs.append('-DBoost_NO_BOOST_CMAKE=On')
        extraArgs.append('-DBoost_NO_SYSTEM_PATHS=True')

        if Windows():
            extraArgs.append('-DBOOST_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir) + '/boost')
            extraArgs.append('-DBLOSC_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir + '/blosc'))
            extraArgs.append('-DTBB_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir + '/tbb'))
            extraArgs.append('-DTBB_DIR={libInstDir}'.format(libInstDir=context.libInstDir + '/tbb'))
            extraArgs.append('-DILMBASE_ROOT={libInstDir}'.format(libInstDir=context.libInstDir + '/openexr'))
            extraArgs.append('-DZLIB_ROOT={libInstDir}'.format(libInstDir=context.libInstDir + '/zlib'))
        else:
            extraArgs.append('-DBLOSC_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir))
            extraArgs.append('-DTBB_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir))
            extraArgs.append('-DTBB_DIR={libInstDir}'.format(libInstDir=context.libInstDir))
            extraArgs.append('-DBOOST_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir))
            extraArgs.append('-DBOOST_INCLUDE_DIR="{libInstDir}"'.format(libInstDir=context.libInstDir))
            extraArgs.append('-DILMBASE_ROOT={libInstDir}'.format(libInstDir=context.libInstDir))
            extraArgs.append('-DZLIB_ROOT={libInstDir}'.format(libInstDir=context.libInstDir))

        # Add on any user-specified extra arguments.
        extraArgs += buildArgs

        RunCMake(context, force, extraArgs)

        if Windows():
            InstallDependency("/bin/openvdb.dll", "/openvdb/bin/openvdb.dll")
            InstallDependency("/lib/openvdb.lib", "/openvdb/lib/openvdb.lib")
            InstallDependency("/include/openvdb", "/openvdb/include/openvdb")

OPENVDB = Dependency("OpenVDB", InstallOpenVDB, check_ovdb)

############################################################
# OpenImageIO

if Windows():
    verify_oiio = "OpenImageIO/include/OpenImageIO/oiioversion.h"
else:
    verify_oiio = "include/OpenImageIO/oiioversion.h"

OIIO_URL = "https://github.com/OpenImageIO/oiio/archive/Release-2.2.12.0.zip"

def InstallOpenImageIO(context, force, buildArgs):
    if Linux():
        # OPENJPEG
        with CurrentWorkingDirectory(DownloadURL("https://github.com/uclouvain/openjpeg/archive/refs/tags/v2.4.0.tar.gz", context, force)):
            RunCMake(context, force, buildArgs)

    with CurrentWorkingDirectory(DownloadURL(OIIO_URL, context, force)):
        extraArgs = ['-DOIIO_BUILD_TOOLS=OFF',
                     '-DOIIO_BUILD_TESTS=OFF',
                     '-DUSE_PYTHON=OFF',
                     '-DSTOP_ON_WARNING=OFF',
                     '-DOPENEXR_IMF_INTERNAL_NAMESPACE_AUTO_EXPOSE=ON'
        ]

        # OIIO's FindOpenEXR module circumvents CMake's normal library
        # search order, which causes versions of OpenEXR installed in
        # /usr/local or other hard-coded locations in the module to
        # take precedence over the version we've built, which would
        # normally be picked up when we specify CMAKE_PREFIX_PATH.
        # This may lead to undefined symbol errors at build or runtime.
        # So, we explicitly specify the OpenEXR we want to use here.

        extraArgs.append('-DUSE_PTEX=ON')

        # Make sure to use boost installed by the build script and not any
        # system installed boost
        if Windows():
            buildArgs.append('-DBoost_ROOT="{boostRoot}"'.format(boostRoot=context.libInstDir + "/boost"))
            buildArgs.append('-DBOOST_ROOT="{boostRoot}"'.format(boostRoot=context.libInstDir + "/boost"))
            buildArgs.append('-DOpenColorIO_ROOT={}'.format(context.libInstDir + "/opencolorio"))
            buildArgs.append('-DZLIB_ROOT="{zlibRoot}"'.format(zlibRoot=context.libInstDir + "/zlib"))
            extraArgs.append('-DOPENEXR_HOME="{libInstDir}"'.format(libInstDir=context.libInstDir + '/openexr'))
            extraArgs.append('-DJPEGTurbo_ROOT=' + context.libInstDir + '/jpeg')
            extraArgs.append('-DBoost_ROOT=' + context.libInstDir + "/boost")
            extraArgs.append('-DBoost_NO_BOOST_CMAKE=On')
            extraArgs.append('-DBoost_NO_SYSTEM_PATHS=True')
        else:
            extraArgs.append('-DOPENEXR_HOME="{instDir}"'.format(instDir=context.libInstDir))
            extraArgs.append('-DBoost_ROOT="{instDir}"'.format(instDir=context.libInstDir))
            extraArgs.append('-DBoost_NO_BOOST_CMAKE=On')
            extraArgs.append('-DBoost_NO_SYSTEM_PATHS=True')   

        # Remove OIIO Tests
        PatchFile("CMakeLists.txt", [("oiio_add_all_tests()", "# oiio_add_all_tests()")])

        # Fix FindTBB module to point to TBB v2020
        # PatchFile("src/cmake/modules/FindTBB.cmake", [
        #     ('    file(READ "${TBB_INCLUDE_DIRS}/tbb/tbb_stddef.h" _tbb_version_file)',   '    file(READ "${TBB_INCLUDE_DIRS}/oneapi/tbb/version.h" _tbb_version_file)'),
        # ])

        # Fix deprecated Int64 type to uint64_t & missing includes
        PatchFile("src/openexr.imageio/exroutput.cpp", [
            ('    virtual Imath::Int64 tellp() { return m_io->tell(); }',   '    virtual uint64_t tellp() { return m_io->tell(); }'),
            ('    virtual void seekp(Imath::Int64 pos)',                    '    virtual void seekp(uint64_t pos)'),
            ('#include <OpenEXR/ImfDeepScanLineOutputPart.h>',              '#include <OpenEXR/ImfDeepFrameBuffer.h>\n#include <OpenEXR/ImfDeepScanLineOutputPart.h>\n')
        ])
        PatchFile("src/openexr.imageio/exrinput.cpp", [
            ('    virtual Imath::Int64 tellg() { return m_io->tell(); }',   '    virtual uint64_t tellg() { return m_io->tell(); }'),
            ('    virtual void seekg(Imath::Int64 pos)',                    '    virtual void seekg(uint64_t pos)'),
            ('#include <OpenEXR/ImfDeepFrameBuffer.h>',                     '#include <OpenEXR/ImfHeader.h>\n#include <OpenEXR/ImfDeepFrameBuffer.h>\n')
        ])

        # Add on any user-specified extra arguments.
        extraArgs += buildArgs

        RunCMake(context, force, extraArgs)

        if Windows():
            InstallDependency("/bin/OpenImageIO.dll",      "/OpenImageIO/bin/OpenImageIO.dll")
            InstallDependency("/bin/OpenImageIO_Util.dll", "/OpenImageIO/bin/OpenImageIO_Util.dll")
            InstallDependency("/lib/OpenImageIO.lib",      "/OpenImageIO/lib/OpenImageIO.lib")
            InstallDependency("/lib/OpenImageIO_Util.lib", "/OpenImageIO/lib/OpenImageIO_Util.lib")
            InstallDependency("/include/OpenImageIO",      "/OpenImageIO/include/OpenImageIO")

OPENIMAGEIO = Dependency("OpenImageIO", InstallOpenImageIO, verify_oiio)

############################################################
# OpenColorIO

# Use v1.1.0 on MacOS and Windows since v1.0.9 doesn't build properly on
# those platforms.
if Linux():
    test_ocio = "include/OpenColorIO/OpenColorABI.h"
    OCIO_URL = "https://github.com/AcademySoftwareFoundation/OpenColorIO/archive/refs/tags/v2.0.0.tar.gz"
else:
    test_ocio = "opencolorio/include/OpenColorIO/OpenColorABI.h"
    OCIO_URL = "https://github.com/AcademySoftwareFoundation/OpenColorIO/archive/refs/tags/v2.0.0.zip"

def InstallOpenColorIO(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(OCIO_URL, context, force)):
        extraArgs = ['-DOCIO_BUILD_TRUELIGHT=OFF',
                     '-DOCIO_BUILD_APPS=OFF',
                     '-DOCIO_BUILD_NUKE=OFF',
                     '-DOCIO_BUILD_DOCS=OFF',
                     '-DOCIO_BUILD_TESTS=OFF',
                     '-DOCIO_BUILD_PYGLUE=OFF',
                     '-DOCIO_BUILD_JNIGLUE=OFF',
                     '-DOCIO_STATIC_JNIGLUE=OFF',
                     '-DOCIO_BUILD_GPU_TESTS=OFF']

        # The OCIO build treats all warnings as errors but several come up
        # on various platforms, including:
        # - On gcc6, v1.1.0 emits many -Wdeprecated-declaration warnings for
        #   std::auto_ptr
        # - On clang, v1.1.0 emits a -Wself-assign-field warning. This is fixed
        #   in https://github.com/AcademySoftwareFoundation/OpenColorIO/commit/0be465feb9ac2d34bd8171f30909b276c1efa996
        #
        # To avoid build failures we force all warnings off for this build.
        if GetVisualStudioCompilerAndVersion():
            # This doesn't work because CMake stores default flags for
            # MSVC in CMAKE_CXX_FLAGS and this would overwrite them.
            # However, we don't seem to get any warnings on Windows
            # (at least with VS2015 and 2017).
            # extraArgs.append('-DCMAKE_CXX_FLAGS=/w')
            pass
        else:
            extraArgs.append('-DCMAKE_CXX_FLAGS=-w')

        # Add on any user-specified extra arguments.
        extraArgs += buildArgs

        RunCMake(context, force, extraArgs)

        if Windows():
            InstallDependency("/bin/OpenColorIO_2_0.dll", "/opencolorio/bin/OpenColorIO_2_0.dll")
            InstallDependency("/lib/OpenColorIO_2_0.lib", "/opencolorio/lib/OpenColorIO_2_0.lib")
            InstallDependency("/include/OpenColorIO",     "/opencolorio/include/OpenColorIO")

OPENCOLORIO = Dependency("OpenColorIO", InstallOpenColorIO, test_ocio)

############################################################
# OpenSubdiv

if Windows():
    verify_osubdv = "opensubdiv/include/opensubdiv/version.h"
    OPENSUBDIV_URL = "https://github.com/PixarAnimationStudios/OpenSubdiv/archive/refs/tags/v3_4_4.zip"
else:
    verify_osubdv = "include/opensubdiv/version.h"
    OPENSUBDIV_URL = "https://github.com/PixarAnimationStudios/OpenSubdiv/archive/refs/tags/v3_4_4.tar.gz"

def InstallOpenSubdiv(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(OPENSUBDIV_URL, context, force)):
        extraArgs = [
            '-DNO_EXAMPLES=ON',
            '-DNO_TUTORIALS=ON',
            '-DNO_REGRESSION=ON',
            '-DNO_DOC=ON',
            '-DNO_OMP=ON',
            '-DNO_CUDA=ON',
            '-DNO_OPENCL=ON',
            '-DNO_DX=ON',
            '-DNO_TESTS=ON',
            '-DNO_GLEW=ON',
            '-DNO_GLFW=ON',
        ]

        # NOTE: For now, we disable TBB in our OpenSubdiv build.
        # This avoids an issue where OpenSubdiv will link against
        # all TBB libraries it finds, including libtbbmalloc and
        # libtbbmalloc_proxy. On Linux and MacOS, this has the
        # unwanted effect of replacing the system allocator with
        # tbbmalloc.
        extraArgs.append('-DNO_TBB=ON')

        # Add on any user-specified extra arguments.
        extraArgs += buildArgs

        # OpenSubdiv seems to error when building on windows w/ Ninja...
        # ...so just use the default generator (ie, Visual Studio on Windows)
        # until someone can sort it out
        oldGenerator = context.cmakeGenerator
        if oldGenerator == "Ninja" and Windows():
            context.cmakeGenerator = None

        # OpenSubdiv 3.3 and later on MacOS occasionally runs into build
        # failures with multiple build jobs. Workaround this by using
        # just 1 job for now. See:
        # https://github.com/PixarAnimationStudios/OpenSubdiv/issues/1194
        oldNumJobs = context.numJobs
        if MacOS():
            context.numJobs = 1

        try:
            RunCMake(context, force, extraArgs)
            if Windows():
                InstallDependency("/lib/osdCPU.lib",     "/opensubdiv/lib/osdCPU.lib")
                InstallDependency("/lib/osdGPU.lib",     "/opensubdiv/lib/osdGPU.lib")
                InstallDependency("/include/opensubdiv", "/opensubdiv/include/opensubdiv")
        finally:
            context.cmakeGenerator = oldGenerator
            context.numJobs = oldNumJobs

OPENSUBDIV = Dependency("OpenSubdiv", InstallOpenSubdiv, verify_osubdv)

############################################################
# OSL

if Windows():
    verify_osl = "osl/include/OSL/oslversion.h"
    OSL_URL = "https://github.com/AcademySoftwareFoundation/OpenShadingLanguage/archive/refs/tags/v1.11.13.0.zip"
else:
    verify_osl = "include/OSL/oslversion.h"
    OSL_URL = "https://github.com/AcademySoftwareFoundation/OpenShadingLanguage/archive/refs/tags/v1.11.13.0.tar.gz"

def InstallOSL(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(OSL_URL, context, force)):
        extraArgs = [
            '-DCMAKE_CXX_STANDARD=17',
            '-DBUILD_SHARED_LIBS=ON',
            '-DOSL_BUILD_TESTS=OFF',
            '-DOSL_BUILD_MATERIALX=ON',
        ]

        # Disable some harmless debugger code which causes compiler errors in OSL code.
        PatchFile("src/liboslexec/llvm_util.cpp", [
            # ERROR 1 | Safe to remove
            ("    m_builder->SetCurrentDebugLocation(llvm::DebugLoc::get(static_cast<unsigned int>(1),",
             "//    m_builder->SetCurrentDebugLocation(llvm::DebugLoc::get(static_cast<unsigned int>(1),"),
            ("                static_cast<unsigned int>(0), /* column?  we don't know it, may be worth tracking through osl->oso*/",
             "//                static_cast<unsigned int>(0), /* column?  we don't know it, may be worth tracking through osl->oso*/"),
            ("                getCurrentDebugScope()));",
             "//                getCurrentDebugScope()));"),

            # ERROR 2 | Safe to remove
            ("        llvm::DebugLoc debug_location =",
             "//        llvm::DebugLoc debug_location ="),
            ("                llvm::DebugLoc::get(static_cast<unsigned int>(sourceline),",
             "//                llvm::DebugLoc::get(static_cast<unsigned int>(sourceline),"),
            ("                        sp,",
             "//                        sp,"),
            ("                        inlineSite);",
             "//                        inlineSite);"),
            ("        m_builder->SetCurrentDebugLocation(debug_location);",
             "//        m_builder->SetCurrentDebugLocation(debug_location);"),

            # ERROR 3 | Safe to remove
            ("    options.PrintMachineCode = dumpasm();",
             "//    options.PrintMachineCode = dumpasm();"),
        ])

        # DISABLE OSL TESTS
        PatchFile("CMakeLists.txt", [
            ('osl_add_all_tests()',   '#osl_add_all_tests()'),
            
        ])

        if Linux():
            # OSL needs pugixml, so let's grab that too, the precompiled version from Blender should work fine.
            pugi = subprocess.call("svn checkout https://svn.blender.org/svnroot/bf-blender/trunk/lib/linux_centos7_x86_64/pugixml/", stdout=subprocess.PIPE, shell=True)
            linuxArgs = [
                '-Dpugixml_ROOT={pugixml_root_path}'.format(pugixml_root_path=context.libInstDir),
                '-Dpybind11_ROOT=~/.local/lib/python3.9/site-packages/pybind11'
            ]
            extraArgs += linuxArgs

            InstallDependency("/build_env/source/OpenShadingLanguage-1.11.13.0/pugixml/include/pugiconfig.hpp", "/include/pugiconfig.hpp")
            InstallDependency("/build_env/source/OpenShadingLanguage-1.11.13.0/pugixml/include/pugixml.hpp",    "/include/pugixml.hpp")
            InstallDependency("/build_env/source/OpenShadingLanguage-1.11.13.0/pugixml/lib/libpugixml.a",       "/lib/libpugixml.a")
        else:
            # OSL needs LLVM, so let's grab that too, the precompiled version from Blender should work fine.
            # Also seems to be the only version of LLVM that I can find anywhere that will actually allow OSL
            # to compile on MSVC. The 'official' installation from LLVM does not supply any of the CMake configs. 
            # It's missing 90% of the includes, missing 90% of the libraries, blahhh. Just using Blender's
            # For now...
            llvmpath = Path(context.srcDir) / "OpenShadingLanguage-1.11.13.0/llvm"
            if not llvmpath.exists():
                # LLVM (🡱)
                llvm = subprocess.call("svn checkout https://svn.blender.org/svnroot/bf-blender/trunk/lib/win64_vc15/llvm/", stdout=subprocess.PIPE, shell=True)
                copy_tree(context.srcDir + "/OpenShadingLanguage-1.11.13.0/llvm", context.libInstDir + "/llvm")
                # And PugiXML
                pugixml = subprocess.call("svn checkout https://svn.blender.org/svnroot/bf-blender/trunk/lib/win64_vc15/pugixml/", stdout=subprocess.PIPE, shell=True)
                copy_tree(context.srcDir + "/OpenShadingLanguage-1.11.13.0/pugixml", context.libInstDir + "/pugixml")
                # And PyBind11
                pybind11 = subprocess.call("{} -B -m pip install pybind11".format(context.libInstDir + "/python/39/bin/python.exe"), stdout=subprocess.PIPE, shell=True)
            windArgs = [
                '-DBoost_ROOT={boost_root_path}'.format(boost_root_path=context.libInstDir + "/boost"),
                '-DBOOST_ROOT={boost_root_path}'.format(boost_root_path=context.libInstDir + "/boost"),
                '-DOIIO_LIBRARY_PATH={oiio_library_path}'.format(oiio_library_path=context.libInstDir + "/OpenImageIO"),
                '-Dpugixml_ROOT={pugixml_root_path}'.format(pugixml_root_path=context.libInstDir + "/pugixml"),
                '-Dpybind11_ROOT={pybind11_root_path}'.format(pybind11_root_path=context.libInstDir + "/python/39/lib/site-packages/pybind11"),
                '-DLLVM_DIRECTORY={llvm_dir_path}'.format(llvm_dir_path=context.libInstDir + "/llvm"),
                '-DLLVM_LIB_DIR={llvm_library_path}'.format(llvm_library_path=context.libInstDir + "/llvm/lib"),
                '-DLLVM_ROOT={llvm_root_path}'.format(llvm_root_path=context.libInstDir + "/llvm"),
                '-DLLVM_ROOT_DIR={llvm_root_path}'.format(llvm_root_path=context.libInstDir + "/llvm"),
                '-DLLVM_INCLUDES={llvm_includes}'.format(llvm_includes=context.libInstDir + "/llvm/include"),
                '-DBISON_ROOT={bison_dir}'.format(bison_dir=context.libInstDir + "/flex"),
                '-DFLEX_ROOT={flex_dir}'.format(flex_dir=context.libInstDir + "/flex"),
                "-Dpartio_ROOT={partio_dir}".format(partio_dir=context.libInstDir + "/partio"),
            ]
            extraArgs += windArgs

        # Add on any user-specified extra arguments.
        extraArgs += buildArgs

        RunCMake(context, force, extraArgs)

        if Windows():
            InstallDependency("/bin/oslc.exe",        "/osl/bin/oslc.exe")
            InstallDependency("/bin/oslinfo.exe",     "/osl/bin/oslinfo.exe")
            InstallDependency("/bin/oslcomp.dll",     "/osl/bin/oslcomp.dll")
            InstallDependency("/bin/oslexec.dll",     "/osl/bin/oslexec.dll")
            InstallDependency("/bin/oslnoise.dll",    "/osl/bin/oslnoise.dll")
            InstallDependency("/bin/oslquery.dll",    "/osl/bin/oslquery.dll")
            InstallDependency("/lib/osl.imageio.dll", "/osl/lib/osl.imageio.dll")
            InstallDependency("/lib/oslcomp.lib",     "/osl/lib/oslcomp.lib")
            InstallDependency("/lib/oslexec.lib",     "/osl/lib/oslexec.lib")
            InstallDependency("/lib/oslnoise.lib",    "/osl/lib/oslnoise.lib")
            InstallDependency("/lib/oslquery.lib",    "/osl/lib/oslquery.lib")
            InstallDependency("/include/OSL",         "/osl/include/OSL")
            InstallDependency("/share/doc/OSL",       "/osl/share/doc/OSL")
            InstallDependency("/share/OSL",           "/osl/share/OSL")

OSL = Dependency("OSL", InstallOSL, verify_osl)

############################################################
# PyOpenGL

def GetPyOpenGLInstructions():
    return ('PyOpenGL is not installed. If you have pip '
            'installed, run "pip install PyOpenGL" to '
            'install it, then re-run this script.\n'
            'If PyOpenGL is already installed, you may need to '
            'update your PYTHONPATH to indicate where it is '
            'located.')

PYOPENGL = PythonDependency("PyOpenGL", GetPyOpenGLInstructions,
                            moduleNames=["OpenGL"])

############################################################
# PySide

def GetPySideInstructions():
    # For licensing reasons, this script cannot install PySide itself.
    if Windows():
        # There is no distribution of PySide2 for Windows for Python 2.7.
        # So use PySide instead. See the following for more details:
        # https://wiki.qt.io/Qt_for_Python/Considerations#Missing_Windows_.2F_Python_2.7_release
        return ('PySide is not installed. If you have pip '
                'installed, run "pip install PySide" '
                'to install it, then re-run this script.\n'
                'If PySide is already installed, you may need to '
                'update your PYTHONPATH to indicate where it is '
                'located.')
    else:
        return ('PySide2 is not installed. If you have pip '
                'installed, run "pip install PySide2" '
                'to install it, then re-run this script.\n'
                'If PySide2 is already installed, you may need to '
                'update your PYTHONPATH to indicate where it is '
                'located.')

PYSIDE = PythonDependency("PySide", GetPySideInstructions,
                          moduleNames=["PySide", "PySide2"])

############################################################
# HDF5

if Windows():
    HDF5_URL = "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.zip"
else:
    HDF5_URL = "https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.tar.gz"

def InstallHDF5(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(HDF5_URL, context, force)):
        RunCMake(context, force,
                 ['-DBUILD_TESTING=OFF',
                  '-DHDF5_BUILD_TOOLS=OFF',
                  '-DHDF5_BUILD_EXAMPLES=OFF'] + buildArgs)

HDF5 = Dependency("HDF5", InstallHDF5, "include/hdf5.h")

############################################################
# Alembic

if Windows():
    ALEMBIC_URL = "https://github.com/alembic/alembic/archive/refs/tags/1.8.0.zip"
else:
    ALEMBIC_URL = "https://github.com/alembic/alembic/archive/refs/tags/1.8.0.tar.gz"

def InstallAlembic(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(ALEMBIC_URL, context, force)):
        cmakeOptions = ['-DUSE_BINARIES=OFF', '-DUSE_TESTS=OFF']
        # if context.enableHDF5:
            # HDF5 requires the H5_BUILT_AS_DYNAMIC_LIB macro be defined if
            # it was built with CMake as a dynamic library.

        if Windows():
            cmakeOptions += [
                '-DZLIB_ROOT="{zlibRoot}"'.format(zlibRoot=context.libInstDir + "/zlib"),
                '-DUSE_HDF5=ON',
                '-DHDF5_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir + "/hdf5"),
                '-DILMBASE_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr"),
                '-DALEMBIC_ILMBASE_IMATH_LIB="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr/lib/Imath-3_0.lib"),
                '-DALEMBIC_ILMBASE_ILMTHREAD_LIB="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr/lib/IlmThread-3_0.lib"),
                '-DALEMBIC_ILMBASE_IEX_LIB="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr/lib/Iex-3_0.lib"),
                '-DALEMBIC_ILMBASE_HALF_LIB="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr/lib/Imath-3_0.lib"),
                '-DALEMBIC_ILMBASE_INCLUDE_DIRECTORY="{libInstDir}"'.format(libInstDir=context.libInstDir + "/openexr/include/Imath"),
                '-DIMATH_DLL=1',
                '-DCMAKE_CXX_FLAGS="-DH5_BUILT_AS_DYNAMIC_LIB"']
        else:
            cmakeOptions += [
                '-DUSE_HDF5=ON',
                '-DHDF5_ROOT="{libInstDir}"'.format(libInstDir=context.libInstDir),
                '-DCMAKE_CXX_FLAGS="-DH5_BUILT_AS_DYNAMIC_LIB"']
        # else:
        #    cmakeOptions += ['-DUSE_HDF5=OFF']

        # cmakeOptions += buildArgs

        RunCMake(context, force, cmakeOptions)

        if Windows():
            InstallDependency("/lib/Alembic.dll", "/alembic/bin/Alembic.dll")
            InstallDependency("/lib/Alembic.lib", "/alembic/lib/Alembic.lib")
            InstallDependency("/include/Alembic", "/alembic/include/Alembic")

ALEMBIC = Dependency("Alembic", InstallAlembic, "include/Alembic/Abc/Base.h")

############################################################
# Draco

if Windows():
    DRACO_URL = "https://github.com/google/draco/archive/refs/tags/1.4.1.zip"
else:
    DRACO_URL = "https://github.com/google/draco/archive/refs/tags/1.4.1.tar.gz"

def InstallDraco(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(DRACO_URL, context, force)):
        cmakeOptions = ['-DBUILD_USD_PLUGIN=ON']
        cmakeOptions += buildArgs

        # Silly goog forgot to include limits -- PATCHED
        PatchFile("src/draco/io/parser_utils.cc", [
            ('#include <iterator>',              '#include <iterator>\n#include <limits>\n')
        ])
    
        RunCMake(context, force, cmakeOptions)

        if Windows():
            InstallDependency("/bin/draco_encoder.exe", "/draco/bin/draco_encoder.exe")
            InstallDependency("/bin/draco_decoder.exe", "/draco/bin/draco_decoder.exe")
            InstallDependency("/lib/draco.lib",         "/draco/lib/draco.lib")
            InstallDependency("/lib/draco.dll",         "/draco/lib/draco.dll")
            InstallDependency("/include/draco",         "/draco/include/draco")

DRACO = Dependency("Draco", InstallDraco, "include/draco/compression/decode.h")

############################################################
# MaterialX

if Windows():
    verify_mtlx = "MaterialX/include/MaterialXCore/Library.h"
else:
    verify_mtlx = "include/MaterialXCore/Library.h"

MATERIALX_URL = "https://github.com/materialx/MaterialX/releases/download/v1.38.0/MaterialX-1.38.0.zip"

def InstallMaterialX(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(MATERIALX_URL, context, force)):
        # USD requires MaterialX to be built as a shared library on Linux
        # Currently MaterialX does not support shared builds on Windows or MacOS
        cmakeOptions = []
        if Linux():
            cmakeOptions += ['-DMATERIALX_BUILD_SHARED_LIBS=ON']

        cmakeOptions += buildArgs;

        # Silly ILM forgot to include limits -- PATCHED
        PatchFile("source/MaterialXRender/Mesh.cpp", [
            ('#include <map>', '#include <limits>\n#include <map>\n')
        ])
        PatchFile("source/MaterialXRender/GeometryHandler.cpp", [
            ('#include <MaterialXGenShader/Util.h>', '#include <MaterialXGenShader/Util.h>\n\n#include <limits>\n')
        ])

        RunCMake(context, force, cmakeOptions)

        if Windows():
            CleanInstall(context, "MaterialX")
            copy_tree(context.libInstDir + "/include/MaterialXCore",       context.libInstDir + "/MaterialX/include/MaterialXCore")
            copy_tree(context.libInstDir + "/include/MaterialXFormat",     context.libInstDir + "/MaterialX/include/MaterialXFormat")
            copy_tree(context.libInstDir + "/include/MaterialXGenGlsl",    context.libInstDir + "/MaterialX/include/MaterialXGenGlsl")
            copy_tree(context.libInstDir + "/include/MaterialXGenMdl",     context.libInstDir + "/MaterialX/include/MaterialXGenMdl")
            copy_tree(context.libInstDir + "/include/MaterialXGenOsl",     context.libInstDir + "/MaterialX/include/MaterialXGenOsl")
            copy_tree(context.libInstDir + "/include/MaterialXGenShader",  context.libInstDir + "/MaterialX/include/MaterialXGenShader")
            copy_tree(context.libInstDir + "/include/MaterialXRender",     context.libInstDir + "/MaterialX/include/MaterialXRender")
            copy_tree(context.libInstDir + "/include/MaterialXRenderGlsl", context.libInstDir + "/MaterialX/include/MaterialXRenderGlsl")
            copy_tree(context.libInstDir + "/include/MaterialXRenderHw",   context.libInstDir + "/MaterialX/include/MaterialXRenderHw")
            copy_tree(context.libInstDir + "/include/MaterialXRenderOsl",  context.libInstDir + "/MaterialX/include/MaterialXRenderOsl")
            copy_tree(context.libInstDir + "/libraries",                   context.libInstDir + "/MaterialX/libraries")
            copy_tree(context.libInstDir + "/mdl",                         context.libInstDir + "/MaterialX/mdl")
            copy_tree(context.libInstDir + "/resources",                   context.libInstDir + "/MaterialX/resources")
            InstallDependency("/lib/MaterialXCore.lib",       "/MaterialX/lib/MaterialXCore.lib")
            InstallDependency("/lib/MaterialXFormat.lib",     "/MaterialX/lib/MaterialXFormat.lib")
            InstallDependency("/lib/MaterialXGenGlsl.lib",    "/MaterialX/lib/MaterialXGenGlsl.lib")
            InstallDependency("/lib/MaterialXGenMdl.lib",     "/MaterialX/lib/MaterialXGenMdl.lib")
            InstallDependency("/lib/MaterialXGenOsl.lib",     "/MaterialX/lib/MaterialXGenOsl.lib")
            InstallDependency("/lib/MaterialXGenShader.lib",  "/MaterialX/lib/MaterialXGenShader.lib")
            InstallDependency("/lib/MaterialXRender.lib",     "/MaterialX/lib/MaterialXRender.lib")
            InstallDependency("/lib/MaterialXRenderGlsl.lib", "/MaterialX/lib/MaterialXRenderGlsl.lib")
            InstallDependency("/lib/MaterialXRenderHw.lib",   "/MaterialX/lib/MaterialXRenderHw.lib")
            InstallDependency("/lib/MaterialXRenderOsl.lib",  "/MaterialX/lib/MaterialXRenderOsl.lib")

MATERIALX = Dependency("MaterialX", InstallMaterialX, verify_mtlx)

############################################################
# Embree
# For MacOS we use version 3.7.0 to include a fix from Intel
# to build on Catalina.
if MacOS():
    EMBREE_URL = "https://github.com/embree/embree/archive/v3.7.0.tar.gz"
else:
    EMBREE_URL = "https://github.com/embree/embree/archive/refs/tags/v3.12.2.tar.gz"

def InstallEmbree(context, force, buildArgs):
    with CurrentWorkingDirectory(DownloadURL(EMBREE_URL, context, force)):
        extraArgs = [
            '-DEMBREE_TUTORIALS=OFF',
            '-DEMBREE_ISPC_SUPPORT=OFF'
        ]

        if Windows():
            extraArgs.append('-DTBB_ROOT={libInstDir}'.format(libInstDir=context.libInstDir + "/tbb"))
        else:
            extraArgs.append('-DTBB_ROOT={libInstDir}'.format(libInstDir=context.libInstDir))

        extraArgs += buildArgs

        RunCMake(context, force, extraArgs)

        if Windows():
            InstallDependency("/bin/embree3.dll", "/embree/bin/embree3.dll")
            InstallDependency("/lib/embree3.lib", "/embree/lib/embree3.lib")
            InstallDependency("/include/embree3", "/embree/include/embree3")

EMBREE = Dependency("Embree", InstallEmbree, "include/embree3/rtcore.h")

############################################################
# SDL

def InstallSDL(context, force, buildArgs):
#     # SDL
    sdlSrc = context.libInstDir + '/build_env/source/sdl'
#     if not os.path.exists(sdlSrc):
#         os.makedirs(sdlSrc)
#     with CurrentWorkingDirectory(sdlSrc):
#         if len(os.listdir(sdlSrc)) == 0:
#             subprocess.call("git clone https://github.com/libsdl-org/SDL.git .", stdout=subprocess.DEVNULL, shell=True)
#         RunCMake(context, force, buildArgs)

    # SDL IMAGE
    # sdlImgSrc = context.libInstDir + '/build_env/source/sdl_image'
    # if not os.path.exists(sdlImgSrc):
    #     os.makedirs(sdlImgSrc)
    # with CurrentWorkingDirectory(sdlImgSrc):
    #     if len(os.listdir(sdlImgSrc)) == 0:
    #         subprocess.call("git clone https://github.com/libsdl-org/SDL_image.git .", stdout=subprocess.DEVNULL, shell=True)

    #     RunCMake(context, force, buildArgs)

    #     InstallDependency("/build_env/source/sdl_image/SDL_image.h", "/include/SDL2/SDL_image.h")

    #     if Linux():
    #         InstallDependency("/build_env/build/sdl_image/libSDL2_image.so", "/lib/libSDL2_image.so")

SDL = Dependency("sdl", InstallSDL, "include/SDL2/SDL.h")

############################################################
# Install script

print("\n\n")

programDescription = """\
*** KRAKEN -- ENVIRONMENT BUILDER ***

Builds and installs 3rd-party dependencies to a specified location.

LIBRARIES:
The following is a list of libraries that this script
can download and build as needed:

{libraryList}

""".format(
    libraryList="{}".format("".join(["* " + d.name + "\n" for d in AllDependencies])))

parser = argparse.ArgumentParser(description=programDescription, formatter_class=argparse.RawDescriptionHelpFormatter, usage=argparse.SUPPRESS, epilog=textwrap.dedent('''\
                                                                                                                                                ::
                                                                                                                                                ::
                                                                                                                                                :: Kraken. 2021.'''), add_help=False)
parser._optionals.title = "KRAKEN DEPENDENCIES | COMMAND LINE OPTIONS"

group_build = parser.add_mutually_exclusive_group()
group_build.add_argument("--build",      type=str, action="append", dest="force_build", default=[], help="Download and build the specified library")
group_build.add_argument("--build-all",  action="store_true",       dest="force_all", help="Download and build all libraries")

group_cmake = parser.add_mutually_exclusive_group()
group_cmake.add_argument("--generator",  type=str, dest="generator", help="CMake generator to use when building libraries with cmake")
group_cmake.add_argument("--toolset",    type=str, dest="toolset",   help="CMake toolset to use when building libraries with cmake")


group_help = parser.add_mutually_exclusive_group()
group_help.add_argument("--help", action='help', default=argparse.SUPPRESS, help='Show this help message.')

args = parser.parse_args()

class InstallContext:
    def __init__(self, args):

        if Windows():
            INSTALL_DIR = GetVisualStudioDirectories()[0]
            SOURCE_DIR = GetVisualStudioDirectories()[1]
            BUILD_DIR = GetVisualStudioDirectories()[2]

        if MacOS():
            INSTALL_DIR = "../../../lib/apple_darwin_x86_64"
            SOURCE_DIR = "../../../lib/apple_darwin_x86_64/build_env/source"
            BUILD_DIR = "../../../lib/apple_darwin_x86_64/build_env/build"

        # Directory where dependencies will be installed
        self.libInstDir = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), INSTALL_DIR)).replace('\\', '/')

        # Directory where dependencies will be installed
        self.libInstDir = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), INSTALL_DIR)).replace('\\', '/')

        # Directory where dependencies will be downloaded and extracted
        self.srcDir = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), SOURCE_DIR)).replace('\\', '/')

        # Directory where USD and dependencies will be built
        self.buildDir = os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)), BUILD_DIR)).replace('\\', '/')

        # Determine which downloader to use.  The reason we don't simply
        # use urllib2 all the time is that some older versions of Python
        # don't support TLS v1.2, which is required for downloading some
        # dependencies.
        if find_executable("curl"):
            self.downloader = DownloadFileWithCurl
            self.downloaderName = "curl"
        elif Windows() and find_executable("powershell"):
            self.downloader = DownloadFileWithPowershell
            self.downloaderName = "powershell"
        else:
            self.downloader = DownloadFileWithUrllib
            self.downloaderName = "built-in"

        # Number of jobs
        self.numJobs = multiprocessing.cpu_count()

        # Dependencies to build
        self.forceBuild = [dep.lower() for dep in args.force_build]
        self.forceBuildAll = args.force_all

        # CMake generator and toolset
        self.cmakeGenerator = args.generator
        self.cmakeToolset = args.toolset

        self.buildDebug = False

        # - DEPENDENCIES
        self.buildZlib      = any("zlib"      in dep for dep in self.forceBuild)
        self.buildVulkan    = any("vulkan"    in dep for dep in self.forceBuild)
        self.buildSDL       = any("sdl"       in dep for dep in self.forceBuild)
        self.buildArnold    = any("arnold"    in dep for dep in self.forceBuild)
        self.buildCycles    = any("cycles"    in dep for dep in self.forceBuild)
        self.buildProRender = any("prorender" in dep for dep in self.forceBuild)
        self.buildMitsuba   = any("mitsuba"   in dep for dep in self.forceBuild)
        self.buildRenderman = any("prman"     in dep for dep in self.forceBuild)
        self.buildTBB       = any("tbb"       in dep for dep in self.forceBuild)
        self.buildBoost     = any("boost"     in dep for dep in self.forceBuild)
        self.buildPtex      = any("ptex"      in dep for dep in self.forceBuild)
        self.buildBlosc     = any("blosc"     in dep for dep in self.forceBuild)
        self.buildPNG       = any("png"       in dep for dep in self.forceBuild)
        self.buildJPEG      = any("jpeg"      in dep for dep in self.forceBuild)
        self.buildTIFF      = any("tiff"      in dep for dep in self.forceBuild)
        self.buildOpenEXR   = any("openexr"   in dep for dep in self.forceBuild)
        self.buildOpenVDB   = any("openvdb"   in dep for dep in self.forceBuild)
        self.buildEmbree    = any("embree"    in dep for dep in self.forceBuild)
        self.buildHdf5      = any("hdf5"      in dep for dep in self.forceBuild)
        self.buildOIIO      = any("oiio"      in dep for dep in self.forceBuild)
        self.buildOCIO      = any("ocio"      in dep for dep in self.forceBuild)
        self.buildOSD       = any("opensubd"  in dep for dep in self.forceBuild)
        self.buildAlembic   = any("alembic"   in dep for dep in self.forceBuild)
        self.buildDraco     = any("draco"     in dep for dep in self.forceBuild)
        self.buildMaterialX = any("materialx" in dep for dep in self.forceBuild)
        self.buildOSL       = any("osl"       in dep for dep in self.forceBuild)

        self.buildAll       = any("all"       in dep for dep in self.forceBuild)
        self.cleanAll       = any("clean"     in dep for dep in self.forceBuild)

    def GetBuildArguments(self, dep):
        return self.forceBuild

    def ForceBuildDependency(self, dep):
        return self.forceBuildAll or dep.name.lower() in self.forceBuild

try:
    context = InstallContext(args)
except Exception as e:
    PrintError(str(e))
    sys.exit(1)

# Augment PATH on Windows so that 3rd-party dependencies can find libraries
# they depend on. In particular, this is needed for building IlmBase/OpenEXR.
extraPaths = []
extraPythonPaths = []
if Windows():
    extraPaths.append(os.path.join(context.libInstDir, "lib"))
    extraPaths.append(os.path.join(context.libInstDir, "bin"))

if extraPaths:
    paths = os.environ.get('PATH', '').split(os.pathsep) + extraPaths
    os.environ['PATH'] = os.pathsep.join(paths)

if extraPythonPaths:
    paths = os.environ.get('PYTHONPATH', '').split(os.pathsep) + extraPythonPaths
    os.environ['PYTHONPATH'] = os.pathsep.join(paths)

# Determine list of dependencies that are required based on options
# user has selected.

requiredDependencies = []

if context.buildZlib:
    requiredDependencies += [ZLIB]

if context.buildVulkan:
    requiredDependencies += [VULKAN]

if context.buildArnold:
    requiredDependencies += [ARNOLD]

if context.buildMitsuba:
    requiredDependencies += [MITSUBA]

if context.buildCycles:
    requiredDependencies += [CYCLES]

if context.buildProRender:
    requiredDependencies += [PRORENDER]

if context.buildRenderman:
    requiredDependencies += [RENDERMAN]

if context.buildTBB:
    requiredDependencies += [TBB]

if context.buildBoost:
    requiredDependencies += [BOOST]

if context.buildHdf5:
    requiredDependencies += [HDF5]

if context.buildAlembic:
    requiredDependencies += [ALEMBIC]

if context.buildDraco:
    requiredDependencies += [DRACO]

if context.buildEmbree:
    requiredDependencies += [EMBREE]

if context.buildPtex:
    requiredDependencies += [PTEX]

if context.buildBlosc:
    requiredDependencies += [BLOSC]

if context.buildPNG:
    requiredDependencies += [PNG]

if context.buildJPEG:
    requiredDependencies += [JPEG]

if context.buildTIFF:
    requiredDependencies += [TIFF]

if context.buildOpenEXR:
    requiredDependencies += [OPENEXR]

if context.buildOpenVDB:
    requiredDependencies += [OPENVDB]

if context.buildOIIO:
    requiredDependencies += [OPENIMAGEIO]

if context.buildOSD:
    requiredDependencies += [OPENSUBDIV]

if context.buildOCIO:
    requiredDependencies += [OPENCOLORIO]

if context.buildMaterialX:
    requiredDependencies += [MATERIALX]

if context.buildOSL:
    requiredDependencies += [OSL]

if context.buildSDL:
    requiredDependencies += [SDL]

if context.buildAll:
    requiredDependencies += [ZLIB, TBB, BOOST, HDF5, JPEG, TIFF, PNG, PTEX, BLOSC, OPENEXR, ALEMBIC, DRACO, EMBREE, OPENVDB, OPENCOLORIO, OPENIMAGEIO, OPENSUBDIV, MATERIALX, DRACO, OSL, CYCLES, PRORENDER, SDL]

if context.cleanAll:
    if os.path.isdir(context.libInstDir):
        shutil.rmtree(context.libInstDir)
        print("Cleaned {}".format(context.libInstDir))
        sys.exit()

# Assume zlib already exists on Linux platforms and don't build
# our own. This avoids potential issues where a host application
# loads an older version of zlib than the one we'd build and link
# our libraries against.
if Linux():
    if ZLIB in requiredDependencies:
        requiredDependencies.remove(ZLIB)

# Error out if user explicitly specified building usdview without required
# components. Otherwise, usdview will be silently disabled. This lets users
# specify "--no-python" without explicitly having to specify "--no-usdview",
# for instance.
if "--usdview" in sys.argv:
    if not context.buildUsdImaging:
        PrintError("Cannot build usdview when usdImaging is disabled.")
        sys.exit(1)
    if not context.buildPython:
        PrintError("Cannot build usdview when Python support is disabled.")
        sys.exit(1)

dependenciesToBuild = []
for dep in requiredDependencies:
    if context.ForceBuildDependency(dep) or not dep.Exists(context):
        if dep not in dependenciesToBuild:
            dependenciesToBuild.append(dep)

# Verify toolchain needed to build required dependencies
if (not find_executable("g++") and
    not find_executable("clang") and
    not GetXcodeDeveloperDirectory() and
    not GetVisualStudioCompilerAndVersion()):
    PrintError("C++ compiler not found -- please install a compiler")
    sys.exit(1)

if find_executable("python"):
    # Error out if a 64bit version of python interpreter is not found
    # Note: Ideally we should be checking the python binary found above, but
    # there is an assumption (for very valid reasons) at other places in the
    # script that the python process used to run this script will be found.
    isPython64Bit = (ctypes.sizeof(ctypes.c_voidp) == 8)
    if not isPython64Bit:
        PrintError("64bit python not found -- please install it and adjust your"
                   "PATH")
        sys.exit(1)

else:
    PrintError("python not found -- please ensure python is included in your "
               "PATH")
    sys.exit(1)

if find_executable("cmake"):
    # Check cmake requirements
    if Windows():
        # Windows build depend on boost 1.70, which is not supported before
        # cmake version 3.14
        cmake_required_version = (3, 14)
    else:
        cmake_required_version = (3, 12)
    cmake_version = GetCMakeVersion()
    if not cmake_version:
        PrintError("Failed to determine CMake version")
        sys.exit(1)

    if cmake_version < cmake_required_version:
        def _JoinVersion(v):
            return ".".join(str(n) for n in v)
        PrintError("CMake version {req} or later required to build USD, "
                   "but version found was {found}".format(
                       req=_JoinVersion(cmake_required_version),
                       found=_JoinVersion(cmake_version)))
        sys.exit(1)
else:
    PrintError("CMake not found -- please install it and adjust your PATH")
    sys.exit(1)

if PYSIDE in requiredDependencies:
    # The USD build will skip building usdview if pyside2-uic or pyside-uic is
    # not found, so check for it here to avoid confusing users. This list of
    # PySide executable names comes from cmake/modules/FindPySide.cmake
    pyside2Uic = ["pyside2-uic", "python2-pyside2-uic", "pyside2-uic-2.7"]
    found_pyside2Uic = any([find_executable(p) for p in pyside2Uic])
    pysideUic = ["pyside-uic", "python2-pyside-uic", "pyside-uic-2.7"]
    found_pysideUic = any([find_executable(p) for p in pysideUic])
    if not found_pyside2Uic and not found_pysideUic:
        if Windows():
            # Windows does not support PySide2 with Python2.7
            PrintError("pyside-uic not found -- please install PySide and"
                       " adjust your PATH. (Note that this program may be named"
                       " {0} depending on your platform)"
                   .format(" or ".join(pysideUic)))
        else:
            PrintError("pyside2-uic not found -- please install PySide2 and"
                       " adjust your PATH. (Note that this program may be"
                       " named {0} depending on your platform)"
                       .format(" or ".join(pyside2Uic)))
        sys.exit(1)

if JPEG in requiredDependencies:
    # NASM is required to build libjpeg-turbo
    if (Windows() and not find_executable("nasm")):
        PrintError("nasm not found -- please install it and adjust your PATH")
        sys.exit(1)

# Summarize
summaryMsg = """\
BUILD SUMMARY:
  Kraken Dependency Libraries    {libInstDir}
  Kraken Dependency Source       {srcDir}
  Kraken Dependency Build        {buildDir}
  CMake generator               {cmakeGenerator}
  CMake toolset                 {cmakeToolset}
  Downloader                    {downloader}
    Zlib                        {buildZlib}
    Vulkan                      {buildVulkan}
    Arnold                      {buildArnold}
    Cycles                      {buildCycles}
    ProRender                   {buildProRender}
    Mitsuba                     {buildMitsuba}
    Renderman                   {buildRenderman}
    TBB                         {buildTBB}
    Boost                       {buildBoost}
    Ptex                        {buildPtex}
    Blosc                       {buildBlosc}
    PNG                         {buildPNG}
    JPEG                        {buildJPEG}
    TIFF                        {buildTIFF}
    OpenEXR                     {buildOpenEXR}
    OpenVDB                     {buildOpenVDB}
    Embree                      {buildEmbree}
    Hdf5                        {buildHdf5}
    OpenImageIO                 {buildOIIO}
    OpenColorIO                 {buildOCIO}
    OpenSubdiv                  {buildOSD}
    Alembic                     {buildAlembic}
    Draco                       {buildDraco}
    MaterialX                   {buildMaterialX}
    OSL                         {buildOSL}
    SDL                         {buildSDL}

  Dependencies                  {dependencies}""".format(
    libInstDir=context.libInstDir,
    srcDir=context.srcDir,
    buildDir=context.buildDir,
    cmakeGenerator=("Default" if not context.cmakeGenerator else context.cmakeGenerator),
    cmakeToolset=("Default"   if not context.cmakeToolset   else context.cmakeToolset),
    downloader=(context.downloaderName),
    dependencies=("None" if not dependenciesToBuild else ", ".join([d.name for d in dependenciesToBuild])),
    # buildConfig=("Debug" if context.buildDebug else "Release"),

    buildZlib=     ("On" if context.buildZlib       or context.buildAll else "Off"),
    buildVulkan=   ("On" if context.buildVulkan     or context.buildAll else "Off"),
    buildArnold=   ("On" if context.buildArnold     or context.buildAll else "Off"),
    buildCycles=   ("On" if context.buildCycles     or context.buildAll else "Off"),
    buildProRender=("On" if context.buildProRender  or context.buildAll else "Off"),
    buildMitsuba=  ("On" if context.buildMitsuba    or context.buildAll else "Off"),
    buildRenderman=("On" if context.buildRenderman  or context.buildAll else "Off"),
    buildTBB=      ("On" if context.buildTBB        or context.buildAll else "Off"),
    buildBoost=    ("On" if context.buildBoost      or context.buildAll else "Off"),
    buildPtex=     ("On" if context.buildPtex       or context.buildAll else "Off"),
    buildBlosc=    ("On" if context.buildBlosc      or context.buildAll else "Off"),
    buildPNG=      ("On" if context.buildPNG        or context.buildAll else "Off"),
    buildJPEG=     ("On" if context.buildJPEG       or context.buildAll else "Off"),
    buildTIFF=     ("On" if context.buildTIFF       or context.buildAll else "Off"),
    buildOpenEXR=  ("On" if context.buildOpenEXR    or context.buildAll else "Off"),
    buildOpenVDB=  ("On" if context.buildOpenVDB    or context.buildAll else "Off"),
    buildEmbree=   ("On" if context.buildEmbree     or context.buildAll else "Off"),
    buildHdf5=     ("On" if context.buildHdf5       or context.buildAll else "Off"),
    buildOIIO=     ("On" if context.buildOIIO       or context.buildAll else "Off"),
    buildOCIO=     ("On" if context.buildOCIO       or context.buildAll else "Off"),
    buildOSD=      ("On" if context.buildOSD        or context.buildAll else "Off"),
    buildAlembic=  ("On" if context.buildAlembic    or context.buildAll else "Off"),
    buildDraco=    ("On" if context.buildDraco      or context.buildAll else "Off"),
    buildMaterialX=("On" if context.buildMaterialX  or context.buildAll else "Off"),
    buildOSL=      ("On" if context.buildOSL        or context.buildAll else "Off"),
    buildSDL=      ("On" if context.buildSDL        or context.buildAll else "Off"),

    buildAll=("On" if context.buildAll else "Off"),
    cleanAll=("On" if context.cleanAll else "Off"))

Print(summaryMsg)

# if args.dry_run:
#     sys.exit(0)

# Scan for any dependencies that the user is required to install themselves
# and print those instructions first.
pythonDependencies = \
    [dep for dep in dependenciesToBuild if type(dep) is PythonDependency]
if pythonDependencies:
    for dep in pythonDependencies:
        Print(dep.getInstructions())
    sys.exit(1)

# Ensure directory structure is created and is writable.
for dir in [context.libInstDir, context.libInstDir, context.srcDir,
            context.buildDir]:
    try:
        if os.path.isdir(dir):
            testFile = os.path.join(dir, "canwrite")
            open(testFile, "w").close()
            os.remove(testFile)
        else:
            os.makedirs(dir)
    except Exception as e:
        PrintError("Could not write to directory {dir}. Change permissions "
                   "or choose a different location to install to."
                   .format(dir=dir))
        sys.exit(1)

try:
    # Download and install 3rd-party dependencies.
    for dep in dependenciesToBuild:
        PrintStatus("Installing {dep}...".format(dep=dep.name))
        dep.installer(context,
                      buildArgs=context.GetBuildArguments(dep),
                      force=context.ForceBuildDependency(dep))
except Exception as e:
    PrintError(str(e))
    sys.exit(1)

# Done. Print out a final status message.
requiredInPythonPath = set([
    os.path.join(context.libInstDir, "lib", "python")
])
requiredInPythonPath.update(extraPythonPaths)

requiredInPath = set([
    os.path.join(context.libInstDir, "bin")
])
requiredInPath.update(extraPaths)

if Windows():
    requiredInPath.update([
        os.path.join(context.libInstDir, "lib"),
        os.path.join(context.libInstDir, "bin"),
        os.path.join(context.libInstDir, "lib")
    ])

Print("""
Success! Now build Kraken, simply use the 'make' command:""")
