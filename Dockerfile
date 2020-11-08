FROM lindkvis/arm32v7_ubuntu_builder:0.2
WORKDIR /usr/src
ARG COMMIT=master
RUN git clone https://github.com/lindkvis/caffa.git
WORKDIR /usr/src/caffa
RUN git checkout $COMMIT
#RUN ls && git submodule update --init && mkdir build
# This actually fails first time due to a cmake-bug with qemu and needs to be rerun.
RUN ls
RUN cmake -S . -B build -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armv7 \
    -DQt5_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5 \
    -DQt5Core_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Core/ \
    -DQt5Gui_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Gui \
    -DQt5Widgets_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Widgets   \
    -DQt5Xml_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Xml ; exit 0
RUN cmake -S . -B build -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=armv7 \
    -DQt5_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5 \
    -DQt5Core_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Core/ \
    -DQt5Gui_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Gui \
    -DQt5Widgets_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Widgets   \
    -DQt5Xml_DIR=/usr/lib/arm-linux-gnueabihf/cmake/Qt5Xml
RUN cd build && make && make install
