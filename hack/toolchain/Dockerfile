FROM robotalks/tbus-toolchain:latest
RUN apt-get -y update && \
    apt-get -y install \
        yasm nasm \
        gcc g++ \
        ninja \
        automake \
        autogen \
        bzip2 \
        libtool \
        bison \
        flex && \
    apt-get -y clean && \
    curl -sSL https://cmake.org/files/v3.6/cmake-3.6.1-Linux-x86_64.tar.gz | tar -C /usr/local --strip-components=1 -zx
