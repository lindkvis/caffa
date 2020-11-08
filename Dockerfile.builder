FROM arm32v7/ubuntu
ENV CC=/usr/bin/gcc CXX=/usr/bin/g++
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt install -yq build-essential git qtbase5-dev qtdeclarative5-dev libssl-dev cmake
