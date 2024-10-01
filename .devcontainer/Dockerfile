FROM mcr.microsoft.com/devcontainers/base:jammy

COPY --chmod=700 .devcontainer/install_dependencies.pl install_dependencies.pl
COPY --chmod=700 .devcontainer/lame.pl lame.pl

RUN apt update
RUN apt install perl -y
RUN ./install_dependencies.pl

ENTRYPOINT ./lame.pl