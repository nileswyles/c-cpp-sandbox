FROM ubuntu:jammy

# mounts repo root to /build folder in docker container...
# docker run -v ./:/build
# docker run --mount type=bind,src="$(pwd)",target=/src
RUN --mount=type=bind,target=/build
WORKDIR /build

RUN "sudo apt update"
RUN "sudo apt install perl"
RUN ./build_scripts/install_deps.pl

# keep running and use docker exec command to build...
#   as opposed to ARG (build time defined..., does that mean it rebuilds entire container image?) and ENTRYPOINT (running/stopping each time)
ENTRYPOINT bash