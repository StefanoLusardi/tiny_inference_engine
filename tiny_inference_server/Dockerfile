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
COPY . /tie/

FROM base as build
WORKDIR /tie/
RUN python3 /tie/scripts/download_onnxruntime.py --platform linux --package_version 1.10.0
WORKDIR /tie/build/
RUN cmake -G Ninja -D CMAKE_BUILD_TYPE=Release -D UNIT_TESTS=OFF -D BUILD_SERVER=ON -D BUILD_CLIENT=OFF ..
RUN cmake --build . --config Release
RUN cmake --install . --prefix /tie/install

# FROM base as test
# ENV BUILD_TYPE=Release
# WORKDIR /tie/build
# RUN cmake -G Ninja -D CMAKE_BUILD_TYPE=${BUILD_TYPE} -D UNIT_TESTS=ON -D BUILD_SERVER=ON -D BUILD_CLIENT=OFF .. && \
#     cmake --build . --config ${BUILD_TYPE}
# ENTRYPOINT [ "ctest" ]

FROM scratch as final
COPY --from=build /tie/install/ /tie
COPY --from=build /lib64/ld-linux-x86-64.so.2 /lib64/ld-linux-x86-64.so.2
COPY --from=build /lib/x86_64-linux-gnu/libc.so.6 /lib/x86_64-linux-gnu/libc.so.6
COPY --from=build /lib/x86_64-linux-gnu/libdl.so.2 /lib/x86_64-linux-gnu/libdl.so.2
COPY --from=build /lib/x86_64-linux-gnu/libpthread.so.0 /lib/x86_64-linux-gnu/libpthread.so.0
COPY --from=build /lib/x86_64-linux-gnu/libstdc++.so.6 /lib/x86_64-linux-gnu/libstdc++.so.6
COPY --from=build /lib/x86_64-linux-gnu/libgcc_s.so.1 /lib/x86_64-linux-gnu/libgcc_s.so.1
COPY --from=build /lib/x86_64-linux-gnu/libm.so.6 /lib/x86_64-linux-gnu/libm.so.6
COPY --from=build /lib/x86_64-linux-gnu/librt.so.1 /lib/x86_64-linux-gnu/librt.so.1

ENTRYPOINT [ "./tie/server" ]

