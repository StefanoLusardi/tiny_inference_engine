FROM ubuntu:20.04 as base
RUN apt update -y && \
    apt install -y --no-install-recommends \
    make \
    cmake \
    ninja-build \
    python3.8 \
    pip \
    gcc-9 \
    g++-9 && \
    pip install conan

ENV CC=gcc-9
ENV CXX=g++-9
ENV BUILD_TYPE=Release
COPY . /tie/

FROM base as build
WORKDIR /tie/build
RUN cmake -G Ninja -D CMAKE_BUILD_TYPE=${BUILD_TYPE} -D UNIT_TESTS=OFF -D BUILD_SERVER=OFF -D BUILD_CLIENT=ON .. && \
    cmake --build . --config ${BUILD_TYPE}

# FROM base as test
# WORKDIR /tie/build
# RUN cmake -G Ninja -D CMAKE_BUILD_TYPE=${BUILD_TYPE} -D UNIT_TESTS=ON -D BUILD_SERVER=OFF -D BUILD_CLIENT=ON .. && \
#     cmake --build . --config ${BUILD_TYPE}
# ENTRYPOINT [ "ctest" ]

