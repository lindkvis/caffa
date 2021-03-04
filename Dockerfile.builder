FROM arm32v7/ubuntu:20.04
ENV CC=/usr/bin/gcc CXX=/usr/bin/g++
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt install -yq apt-transport-https ca-certificates gnupg software-properties-common build-essential git qtbase5-dev qtdeclarative5-dev libssl-dev nlohmann-json3-dev libgrpc++-dev libgrpc-dev protobuf-compiler-grpc curl
RUN update-ca-certificates
RUN curl -fsSL https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt install -yq cmake

