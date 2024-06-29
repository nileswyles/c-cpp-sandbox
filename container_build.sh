#/bin/sh

docker build -t c-cpp-sandbox:latest -f Dockerfile .

# docker run --mount type=bind,src="$(pwd)",target=/src
docker container rm -f c-cpp-sandbox-build
RESPONSE=`docker run --mount type=bind,src="$(pwd)",target=/build --name c-cpp-sandbox-build -d c-cpp-sandbox:latest 2>&1`
# if [ $? -ne 0 ]; 
# then
#     # if not "already created" error, don't bother starting if already created because there's a reason. Fix root cause.
#     echo "ERROR RUNNING DOCKER CONTAINER... REMOVING"
#     docker container rm c-cpp-sandbox-build
#     echo "ATTEMPTING TO RUN DOCKER CONTAINER AGAIN"
#     docker run --mount type=bind,src="$(pwd)",target=/build --name c-cpp-sandbox-build -d c-cpp-sandbox:latest
# fi
echo "EXECUTING THE FOLLOWING COMMAND IN THE DOCKER CONTAINER: $@"
# # exec command on running container
# BUILD_SCRIPT=$1; shift
docker exec -it c-cpp-sandbox-build bash -c "cd /build && $@"