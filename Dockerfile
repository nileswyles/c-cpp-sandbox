FROM ubuntu:jammy

COPY --chmod=700 build_scripts/install_deps.pl install_deps.pl
COPY --chmod=700 build_scripts/lame.pl lame.pl

RUN apt update
RUN apt install perl -y
RUN ./install_deps.pl

ENTRYPOINT ./lame.pl