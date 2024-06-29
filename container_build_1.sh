#/bin/sh

WTF="$@"
echo $WTF
# docker build --build-arg BUILD_CMD="$WTF" -t c-cpp-sandbox:latest . --progress "plain"
# docker build -t c-cpp-sandbox:latest . --progress "plain"
docker build -t c-cpp-sandbox:latest .

# one-shot
docker container rm -f c-cpp-sandbox-build
docker run --mount type=bind,src="$(pwd)",target=/build --workdir /build --entrypoint "$WTF" --name c-cpp-sandbox-build -d c-cpp-sandbox:latest
# docker run -v $(pwd):/build --entrypoint "/build/$WTF" --name c-cpp-sandbox-build -d c-cpp-sandbox:latest
# docker run -v $(pwd):/build --entrypoint "bash" --name c-cpp-sandbox-build -d c-cpp-sandbox:latest