################################################################################

# podman build -f Dockerfile_m2.dockerfile -t lib_garble:dev .
# NOTE: it CAN work with Docker but it less than ideal b/c it can not reuse the host's cache
# b/c docker build has no support for volume contrary to podman/buildah
# podman run -it --rm -v $(pwd):/data/ -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY lib_garble:dev --pgarbled_input_path=/data/pgarbled.pb.bin --packmsg_input_path=/data/packmsg.pb.bin

FROM ubuntu:20.04 as builder

WORKDIR /usr/src/app

# DEBIAN_FRONTEND needed to stop prompt for timezone
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    wget unzip xz-utils lsb-release software-properties-common git \
    && rm -rf /var/lib/apt/lists/*

# prereq: install CMake
ENV PATH=$PATH:/opt/cmake/bin/
RUN wget https://github.com/Kitware/CMake/releases/download/v3.22.3/cmake-3.22.3-linux-x86_64.sh && \
    chmod +x cmake-3.22.3-linux-x86_64.sh && \
    mkdir /opt/cmake/ && \
    ./cmake-3.22.3-linux-x86_64.sh --skip-license --prefix=/opt/cmake/ && \
    rm cmake-*.sh && \
    cmake -version

# prereq: install Ninja (ninja-build)
RUN wget https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip && \
    unzip ninja-linux.zip -d /usr/local/bin/ && \
    rm ninja-linux.zip && \
    ninja --version

# prereq: install clang
# https://baykara.medium.com/installing-clang-10-in-a-docker-container-4c24a4538af2
# ENV LLVM_VERSION clang+llvm-13.0.1-x86_64-linux-gnu-ubuntu-18.04
# RUN wget https://github.com/llvm/llvm-project/releases/download/llvmorg-13.0.1/$LLVM_VERSION.tar.xz && \
#     mkdir -p /opt/$LLVM_VERSION && \
#     tar -xf $LLVM_VERSION.tar.xz -C /opt/$LLVM_VERSION && \
#     mkdir -p /opt/llvm && \
#     mv /opt/$LLVM_VERSION/$LLVM_VERSION/* /opt/llvm && \
#     rm $LLVM_VERSION.tar.xz
# cf https://apt.llvm.org/
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 13 && \
    rm -rf /var/lib/apt/lists/* && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-13 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-13 100 && \
    clang --version

################################################################################
# TODO above SHOULD be in a "base image cpp builder"

RUN apt-get update && apt-get install -y \
    libfreetype-dev libboost-filesystem-dev \
    # TEMP: only for the eval_cli
    libx11-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . .

# TODO add support for cmake install(or cpack?)
RUN mkdir -p build && cd build && cmake -GNinja .. && cmake --build .

# MUST also get all the shared libs; using the CMake generated list of libs
# cf https://github.com/Interstellar-Network/cmake/blob/main/export_libs.cmake
# It SHOULD be empty for "lib_garble" but we might as well handle it just in case.
# NOTE: if it fails with cp: will not overwrite just-created '/usr/local/lib/liblibyosys.so' with '/usr/src/app/build/_deps/yosys_fetch-build/liblibyosys.so'
# It probably means you are caching the container target/ by using a volume and there are multiple build dir
# CHECK: find build/ -type d -name "*lib-garble-wrapper*"
# If yes: DELETE the dup
RUN cat $(find build/ -type f -name cmake_generated_libs) | tr " " "\n" |  grep "/usr/src/app/build/.*.so" > list_local_shared_libs && \
    xargs --arg-file=list_local_shared_libs cp --target-directory=/usr/local/lib/ && \
    rm list_local_shared_libs \
    || echo "no shared libs to copy" && touch /usr/local/lib/no_shared_lib_to_copy

################################################################################

FROM ubuntu:20.04

ENV APP_NAME cli_eval_stripped

ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

RUN apt-get update && apt-get install -y \
    # TEMP: only for the eval_cli
    libx11-6 libpng16-16 \
    && rm -rf /var/lib/apt/lists/*

# NOTE if "no shared libs to copy" above; we  MUST add a random file else COPY fails with:
# "copier: stat: ["/usr/local/lib/*.so"]: no such file or directory"
# cf https://stackoverflow.com/questions/31528384/conditional-copy-add-in-dockerfile
COPY --from=builder /usr/local/lib/no_shared_lib_to_copy /usr/local/lib/*.so /usr/local/lib/
COPY --from=builder /usr/src/app/build/tests/$APP_NAME /usr/local/bin/$APP_NAME

ENTRYPOINT  ["/usr/local/bin/cli_eval_stripped"]