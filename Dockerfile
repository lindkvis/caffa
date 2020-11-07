FROM arm32v7/ubuntu
RUN DEBIAN_FRONTEND=noninteractive apt install -yq build-essential git qtbase5-dev
WORKDIR /usr/src
# We need to build CMake from scratch, since the release in Ubuntu fails when run through qemu.
RUN git clone https://github.com/Kitware/CMake.git
RUN cd CMake
RUN ./bootstrap && make && sudo make install 
RUN git clone https://github.com/lindkvis/caffa.git
RUN cd caffa
RUN mkdir build
RUN cd build
RUN cmake ..
RUN make -j2
RUN sudo make install
