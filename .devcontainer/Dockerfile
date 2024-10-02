FROM mcr.microsoft.com/devcontainers/base:jammy

COPY --chmod=700 install_dependencies.pl install_dependencies.pl
COPY --chmod=700 lame.pl lame.pl

RUN apt update
RUN apt install perl -y
RUN ./install_dependencies.pl

ENTRYPOINT ./lame.pl