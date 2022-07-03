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
RUN apt install python3.10 -y
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.10 1
RUN update-alternatives --set python /usr/bin/python3.10
RUN apt install python3-pip -y
RUN apt install doxygen -y
RUN apt install graphviz -y

# -------------------------------------------------------------------------------------------------------------------------------
# Install Python packages.

RUN pip3 install sphinx
RUN pip3 install breathe
RUN pip3 install sphinx-rtd-theme
RUN pip3 install exhale
RUN pip3 install sphinxcontrib-napoleon

# -------------------------------------------------------------------------------------------------------------------------------
# Clone latest Kraken.

RUN if [[ -d "kraken/" ]]; then rm -r kraken/; fi
RUN mkdir kraken/
ADD "https://www.random.org/cgi-bin/randbyte?nbytes=10&format=h" skipcache
RUN git clone https://github.com/Wabi-Studios/Kraken.git kraken/

# -------------------------------------------------------------------------------------------------------------------------------
# Grab the kraken version from the kraken version file (only place where version is ever hardcoded).

RUN KRAKEN_VERSION_XXX="$(grep 'KRAKEN_VERSION' kraken/source/kraken/krakernel/KKE_version.h | cut -d\  -f2)"
RUN KRAKEN_VERSION_PATCH="$(grep 'KRAKEN_VERSION_PATCH' kraken/source/kraken/krakernel/KKE_version.h | cut -d\  -f2)"
RUN KRAKEN_VERSION_MAJOR="$((KRAKEN_VERSION_XXX / 100))"
RUN KRAKEN_VERSION_MINOR="$((KRAKEN_VERSION_XXX % 100))"
RUN KRAKEN_VERSION="$KRAKEN_VERSION_MAJOR.$KRAKEN_VERSION_MINOR.$KRAKEN_VERSION_PATCH"
RUN KRAKEN_VERSION_DECIMAL="$KRAKEN_VERSION_MAJOR.$KRAKEN_VERSION_MINOR"

# -------------------------------------------------------------------------------------------------------------------------------
# Build the docs.

RUN cd kraken && make doc_all

# -------------------------------------------------------------------------------------------------------------------------------
# Done.

RUN echo "                                          "
RUN echo "  _ _ _ _____ _____ _____                 "
RUN echo " | | | |  _  | __  |     |                "
RUN echo " | | | |     | __ -|-   -|                "
RUN echo " |_____|__|__|_____|_____|                "
RUN echo "     Animation Studios                    "
RUN echo "                                          "
RUN echo "  Kraken ::: v$KRAKEN_VERSION_DECIMAL"
RUN echo "                                          "