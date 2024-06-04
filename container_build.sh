#/bin/sh

# build (parts not already built?)
docker build -t c-cpp-sandbox:latest .

# run (if not already running)
docker run --name c-cpp-sandbox-build

# exec command on running container
BUILD_SCRIPT=$1; shift
docker exec -it c-cpp-sandbox-build bash -c "$BUILD_SCRIPT $@"