#/bin/sh

# new way
WTF="$@"
echo $WTF
docker build --build-arg BUILD_CMD="$WTF" -t c-cpp-sandbox:latest .

# old way
# build (parts not already built?)
# docker build -t c-cpp-sandbox:latest .
# run (if not already running)
# RESPONSE=`docker run --name c-cpp-sandbox-build -d c-cpp-sandbox:latest 2>&1`
# if [ $? -ne 125 ]; 
# then
#     # if not "already created" error, don't bother starting if already created because there's a reason. Fix root cause.
#     echo $RESPONSE
# fi

# # exec command on running container
# BUILD_SCRIPT=$1; shift
# docker exec -it c-cpp-sandbox-build bash -c "$BUILD_SCRIPT $@"