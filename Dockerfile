FROM ubuntu:jammy

# mounts repo root to /build folder in docker container...
# docker run -v ./:/build
# docker run --mount type=bind,src="$(pwd)",target=/src
RUN --mount=type=bind,target=/build,rw=true

WORKDIR /build

RUN apt update
# RUN "apt install perl"
# RUN ./build_scripts/install_deps.pl

# old way
# keep running and use docker exec command to build...
#   as opposed to ARG (build time defined..., does that mean it rebuilds entire container image?) 
#   and ENTRYPOINT (running/stopping each time)
# ENTRYPOINT "bash"

# new way
ARG BUILD_CMD
RUN echo $BUILD_CMD
RUN $BUILD_CMD