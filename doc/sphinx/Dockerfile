# -------------------------------------------------------------------------------------------------------------------------------
# Grab Ubuntu.

FROM ubuntu:20.04

# -------------------------------------------------------------------------------------------------------------------------------
# Update apt packages and install prereqs.

RUN apt update
RUN apt upgrade -y
RUN apt install software-properties-common -y
RUN apt install git -y

# -------------------------------------------------------------------------------------------------------------------------------
# Install Python & set as default.

RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt install python3.9 -y
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.9 1
RUN update-alternatives --set python /usr/bin/python3.9
RUN apt install python3-pip -y
RUN apt install doxygen -y
RUN apt install graphviz -y

# -------------------------------------------------------------------------------------------------------------------------------
# Install Python packages (& our custom fork of Exhale).

RUN pip3 install Sphinx
RUN pip3 install breathe
RUN pip3 install sphinx-rtd-theme
RUN pip3 install https://storage.googleapis.com/dependency_links/Exhale-1.3.0.tar.gz
RUN pip3 install sphinxcontrib-napoleon

# -------------------------------------------------------------------------------------------------------------------------------
# Clone latest KRAKEN, here in the cloud, we refer to it as the chaos engine.

RUN if [[ -d "chaos_engine/" ]]; then rm -r chaos_engine/; fi
RUN mkdir chaos_engine/
ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache
RUN git clone https://github.com/Wabi-Studios/KRAKEN.git chaos_engine/

# -------------------------------------------------------------------------------------------------------------------------------
# Grab the kraken version from the kraken version file (only place where version is ever hardcoded).

RUN KRAKEN_VERSION_XXX="$(grep 'KRAKEN_VERSION' chaos_engine/source/kraken/krakernel/KKE_version.h | cut -d\  -f2)"
RUN KRAKEN_VERSION_PATCH="$(grep 'KRAKEN_VERSION_PATCH' chaos_engine/source/kraken/krakernel/KKE_version.h | cut -d\  -f2)"
RUN KRAKEN_VERSION_MAJOR="$((KRAKEN_VERSION_XXX / 100))"
RUN KRAKEN_VERSION_MINOR="$((KRAKEN_VERSION_XXX % 100))"
RUN KRAKEN_VERSION="$KRAKEN_VERSION_MAJOR.$KRAKEN_VERSION_MINOR.$KRAKEN_VERSION_PATCH"
RUN KRAKEN_VERSION_DECIMAL="$KRAKEN_VERSION_MAJOR.$KRAKEN_VERSION_MINOR"

# -------------------------------------------------------------------------------------------------------------------------------
# Build the docs ::: This takes a long time - there is alot of source code to go through KRAKEN + USD
#                    and breathe <-> exhale work in unison to build beautiful documentation in fully
#                    automated fashion. The alternative would be to create each of these .rst files
#                    by hand, and the manual effort to do that would require a full time job in and
#                    of itself. So for the sake of sanity (and accuracy with the latest codebase) --
#                    this is how it is to be done...
#                    NOTES:
#                    - Don't break my f#@%ing docs! >:(
#                    - Edit the config in source/conf.py if
#                      * You add a new *_API (DLL IMPORT/EXPORT)
#                      * You add any code not parsable by doxygen:
#                          Exclude the whole file from documentation build  
#
# - Exhale 'Sorry :(' messages - okay       (why we have a private fork, project is dead/stale)
# - Breathe python segfaults   - not okay!! (I don't want to fork it, it is an active project.)

RUN cd chaos_engine/doc/sphinx && make html

# -------------------------------------------------------------------------------------------------------------------------------
# Godspeed.

RUN echo "                                          "
RUN echo "  _ _ _ _____ _____ _____                 "
RUN echo " | | | |  _  | __  |     |                "
RUN echo " | | | |     | __ -|-   -|                "
RUN echo " |_____|__|__|_____|_____|                "
RUN echo "     Animation Studios                    "
RUN echo "                                          "
RUN echo "  Chaos Engine ::: v$KRAKEN_VERSION_DECIMAL"
RUN echo "                                          "