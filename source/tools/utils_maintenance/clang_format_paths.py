#!/usr/bin/env python3

import multiprocessing
import os
import sys
import subprocess
import io
import difflib
import fnmatch
import signal

from functools import partial

CLANG_FORMAT_CMD = "clang-format"
VERSION_MIN = (6, 0, 0)

BASE_DIR = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
os.chdir(BASE_DIR)


extensions = (
    ".c", ".cc", ".cpp", ".cxx",
    ".h", ".hh", ".hpp", ".hxx",
    ".m", ".mm", ".osl", ".glsl"
)

extensions_only_retab = (
    ".cmake",
    "CMakeLists.txt",
    ".sh",
)

ignore_files = {
    ""
}


def compute_paths(paths):
    # Optionally pass in files to operate on.
    if not paths:
        paths = (
            "wabi",
            "source",
        )

    if os.sep != "/":
        paths = [f.replace("/", os.sep) for f in paths]
    return paths


def list_files(files, recursive=False, extensions=None, exclude=None):
    if extensions is None:
        extensions = []
    if exclude is None:
        exclude = []

    out = []
    for file in files:
        if recursive and os.path.isdir(file):
            for dirpath, dnames, fnames in os.walk(file):
                fpaths = [os.path.join(dirpath, fname) for fname in fnames]
                for pattern in exclude:
                    # os.walk() supports trimming down the dnames list
                    # by modifying it in-place,
                    # to avoid unnecessary directory listings.
                    dnames[:] = [
                        x for x in dnames
                        if
                        not fnmatch.fnmatch(os.path.join(dirpath, x), pattern)
                    ]
                    fpaths = [
                        x for x in fpaths if not fnmatch.fnmatch(x, pattern)
                    ]
                for f in fpaths:
                    ext = os.path.splitext(f)[1][1:]
                    if ext in extensions:
                        out.append(f)
        else:
            out.append(file)
    return out


def source_files_from_git(paths):
    cmd = ("git", "ls-tree", "-r", "HEAD", *paths, "--name-only", "-z")
    files = subprocess.check_output(cmd).split(b'\0')
    return [f.decode('ascii') for f in files]


def convert_tabs_to_spaces(files):
    for f in files:
        print("TabExpand", f)
        with open(f, 'r', encoding="utf-8") as fh:
            data = fh.read()
            if False:
                # Simple 4 space
                data = data.expandtabs(4)
            else:
                # Complex 2 space
                # because some comments have tabs for alignment.
                def handle(l):
                    ls = l.lstrip("\t")
                    d = len(l) - len(ls)
                    if d != 0:
                        return ("  " * d) + ls.expandtabs(4)
                    else:
                        return l.expandtabs(4)

                lines = data.splitlines(keepends=True)
                lines = [handle(l) for l in lines]
                data = "".join(lines)
        with open(f, 'w', encoding="utf-8") as fh:
            fh.write(data)


def clang_format_version():
    version_output = subprocess.check_output((CLANG_FORMAT_CMD, "-version")).decode('utf-8')
    version = next(iter(v for v in version_output.split() if v[0].isdigit()), None)
    if version is not None:
        version = version.split("-")[0]
        version = tuple(int(n) for n in version.split("."))
    return version


def clang_format_file(file):
    try:
        with io.open(file, 'r', encoding='utf-8') as f:
            original = f.readlines()
    except IOError as exc:
        print("Cannot open {}".format(file))
        return [], ""
        # raise DiffError(str(exc))
    cmd = ["clang-format", "-i", file]
    try:
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
            encoding='utf-8')
    except OSError as exc:
        print("Error")
    proc_stdout = proc.stdout
    proc_stderr = proc.stderr
    outs = list(proc_stdout.readlines())
    errs = list(proc_stderr.readlines())
    proc.wait()
    if proc.returncode:
        return "Nonzero exit status"
    return [], errs
    # return make_diff(file, original, outs), errs
    # return subprocess.check_output(cmd, stderr=subprocess.STDOUT)

def make_diff(file, original, reformatted):
    return list(
        difflib.unified_diff(
            original,
            reformatted,
            fromfile='{}\t(original)'.format(file),
            tofile='{}\t(reformatted)'.format(file),
            n=3))

def bold_red(s):
    return '\x1b[1m\x1b[31m' + s + '\x1b[0m'


def colorize(diff_lines):
    def bold(s):
        return '\x1b[1m' + s + '\x1b[0m'

    def cyan(s):
        return '\x1b[36m' + s + '\x1b[0m'

    def green(s):
        return '\x1b[32m' + s + '\x1b[0m'

    def red(s):
        return '\x1b[31m' + s + '\x1b[0m'

    for line in diff_lines:
        if line[:4] in ['--- ', '+++ ']:
            yield bold(line)
        elif line.startswith('@@ '):
            yield cyan(line)
        elif line.startswith('+'):
            yield green(line)
        elif line.startswith('-'):
            yield red(line)
        else:
            yield line

def print_diff(diff_lines, use_color):
    if use_color:
        diff_lines = colorize(diff_lines)
    sys.stdout.writelines(diff_lines)

def clang_format(files):
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    try:
        signal.SIGPIPE
    except AttributeError:
        # compatibility, SIGPIPE does not exist on Windows
        pass
    else:
        signal.signal(signal.SIGPIPE, signal.SIG_DFL)

    colored_stdout = True
    colored_stderr = True

    # Process in chunks to reduce overhead of starting processes.
    cpu_count = multiprocessing.cpu_count() + 1
    njobs = min(len(files), cpu_count)
    pool = multiprocessing.Pool(njobs)
    it = pool.imap_unordered(partial(clang_format_file), files)
    pool.close()
    while True:
        try:
            outs, errs = next(it)
        except StopIteration:
            break
        except:
            break
        else:
            sys.stderr.writelines(errs)
            if outs == []:
                continue
            print_diff(outs, use_color=colored_stdout)

    if pool:
        pool.join()

def argparse_create():
    import argparse

    # When --help or no args are given, print this help
    usage_text = "Format source code"
    epilog = "This script runs clang-format on multiple files/directories"
    parser = argparse.ArgumentParser(description=usage_text, epilog=epilog)
    parser.add_argument(
        "--expand-tabs",
        dest="expand_tabs",
        default=False,
        action='store_true',
        help="Run a pre-pass that expands tabs "
        "(default=False)",
        required=False,
    )
    parser.add_argument(
        "paths",
        nargs=argparse.REMAINDER,
        help="All trailing arguments are treated as paths."
    )

    return parser


def main():
    version = clang_format_version()
    if version is None:
        print("Unable to detect 'clang-format -version'")
        sys.exit(1)
    if version < VERSION_MIN:
        print("Version of clang-format is too old:", version, "<", VERSION_MIN)
        sys.exit(1)

    args = argparse_create().parse_args()

    use_default_paths = not bool(args.paths)

    paths = compute_paths(args.paths)
    print("Operating on:")
    for p in paths:
        print(" ", p)

    files = [
        f for f in source_files_from_git(paths)
        if f.endswith(extensions)
        if f not in ignore_files
    ]

    # Always operate on all cmake files (when expanding tabs and no paths given).
    files_retab = [
        f for f in source_files_from_git((".",) if use_default_paths else paths)
        if f.endswith(extensions_only_retab)
        if f not in ignore_files
    ]

    if args.expand_tabs:
        convert_tabs_to_spaces(files + files_retab)
    clang_format(files)


if __name__ == "__main__":
    main()
