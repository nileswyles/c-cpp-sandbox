FROM ubuntu:latest
WORKDIR /app
COPY a.out .
ENTRYPOINT ./a.out