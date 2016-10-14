FROM robotalks/tbus-toolchain:latest
RUN echo 'deb http://emdebian.org/tools/debian/ jessie main' >/etc/apt/sources.list.d/crosstools.list && \
    curl -sSL http://emdebian.org/tools/debian/emdebian-toolchain-archive.key | apt-key add - && \
    dpkg --add-architecture armhf && \
    dpkg --add-architecture armel && \
    dpkg --add-architecture arm64 && \
    apt-get -y update && \
    apt-get -y install \
        yasm nasm \
        gcc g++ gfortran \
        ninja \
        automake \
        autogen \
        bzip2 \
        libtool \
        bison \
        flex \
        libx11-dev \
        crossbuild-essential-armhf \
        crossbuild-essential-armel \
        crossbuild-essential-arm64 \
        gfortran-arm-linux-gnueabihf \
        gfortran-arm-linux-gnueabi \
        gfortran-aarch64-linux-gnu && \
    apt-get -y clean && \
    curl -sSL https://cmake.org/files/v3.6/cmake-3.6.1-Linux-x86_64.tar.gz | tar -C /usr/local --strip-components=1 -zx && \
    go get -v github.com/GeertJohan/go.rice/rice
