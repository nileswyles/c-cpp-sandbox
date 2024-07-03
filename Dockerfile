FROM mcr.microsoft.com/devcontainers/base:jammy

COPY --chmod=700 build_scripts/install_dependencies.pl install_dependencies.pl
COPY --chmod=700 build_scripts/lame.pl lame.pl

RUN apt update
RUN apt install perl -y
RUN ./install_dependencies.pl

ENTRYPOINT ./lame.pl