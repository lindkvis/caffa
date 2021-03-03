FROM arm32v7/ubuntu:20.04
ENV CC=/usr/bin/gcc CXX=/usr/bin/g++
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt install -yq build-essential git qtbase5-dev qtdeclarative5-dev libssl-dev nlohmann-json3-dev libgrpc++-dev libgrpc-dev protobuf-compiler-grpc wget
WORKDIR /usr/src/
RUN wget --no-check-certificate https://cmake.org/files/v3.19/cmake-3.19.6.tar.gz
RUN tar xf cmake-3.19.6.tar.gz
WORKDIR /usr/src/cmake-3.19.6
RUN ./configure && make install && cmake --version
