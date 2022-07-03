# Minimal makefile for Sphinx documentation

OS_NCASE:=$(shell uname -s | tr '[A-Z]' '[a-z]')

ifeq ($(OS_NCASE),darwin)
	SPHINXBUILD:=../../../lib/apple_darwin_arm64/python/bin/sphinx-build
else
	SPHINXBUILD:=sphinx-build
endif


# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SOURCEDIR     = source
BUILDDIR      = _build
APIDIR        = source/api

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile clean

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

clean:
	$(Q)rm -rf "$(BUILDDIR)" && rm -rf "$(APIDIR)"
