#!/usr/bin/env python3
#
# "make update" for all platforms.
#

import argparse
import os
import platform
import shutil
import sys

import make_utils
from make_utils import call, check_output


def print_stage(text):
    print("")
    print(text)
    print("")

# Parse arguments


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--no-cova", action="store_true")
    parser.add_argument("--git-command", default="git")
    return parser.parse_args()


def get_cova_git_root():
    return check_output([args.git_command, "rev-parse", "--show-toplevel"])

# Test if git repo can be updated.
def git_update_skip(args, check_remote_exists=True):
    if make_utils.command_missing(args.git_command):
        sys.stderr.write("git not found, can't update code\n")
        sys.exit(1)

    # Abort if a rebase is still progress.
    rebase_merge = check_output([args.git_command, 'rev-parse', '--git-path', 'rebase-merge'], exit_on_error=False)
    rebase_apply = check_output([args.git_command, 'rev-parse', '--git-path', 'rebase-apply'], exit_on_error=False)
    merge_head = check_output([args.git_command, 'rev-parse', '--git-path', 'MERGE_HEAD'], exit_on_error=False)
    if (
            os.path.exists(rebase_merge) or
            os.path.exists(rebase_apply) or
            os.path.exists(merge_head)
    ):
        return "rebase or merge in progress, complete it first"

    # Abort if uncommitted changes.
    changes = check_output([args.git_command, 'status', '--porcelain', '--untracked-files=no'])
    if len(changes) != 0:
        return "you have unstaged changes"

    # Test if there is an upstream branch configured
    if check_remote_exists:
        branch = check_output([args.git_command, "rev-parse", "--abbrev-ref", "HEAD"])
        remote = check_output([args.git_command, "config", "branch." + branch + ".remote"], exit_on_error=False)
        if len(remote) == 0:
            return "no remote branch to pull from"

    return ""


# Update kraken repository.
def cova_update(args):
    print_stage("Updating Kraken Git Repository")
    call([args.git_command, "pull", "--rebase"])


if __name__ == "__main__":
    args = parse_arguments()
    cova_skip_msg = ""

    # Test if we are building a specific release version.
    branch = make_utils.git_branch(args.git_command)
    tag = make_utils.git_tag(args.git_command)
    release_version = make_utils.git_branch_release_version(branch, tag)

    if not args.no_cova:
        cova_skip_msg = git_update_skip(args)
        if cova_skip_msg:
            cova_skip_msg = "Kraken repository skipped: " + cova_skip_msg + "\n"
        else:
            cova_update(args)
