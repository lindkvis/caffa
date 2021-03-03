FROM lindkvis/arm32v7_ubuntu_builder:latest
RUN mkdir /usr/src/caffa
ADD . /usr/src/caffa
WORKDIR /usr/src/caffa
RUN ls
# This actually fails first time due to a cmake-bug with qemu and needs to be rerun.
RUN cmake -S . -B build -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armv7
RUN cd build && make && make install
