#!/usr/bin/perl

# because wtf even is awk/cut...

# lmao, just roll with it, I guess?
# my $VERSION = `lsb_release -c`;
# if ($VERSION =~ /.+:\s+(.+)/) {
#     if ($1 ne "jammy") {
#         print("Invalid version of ubuntu: " . $1);
#         exit;
#     }
# }

# assumes ubuntu jammy

# important dependencies
# https://packages.ubuntu.com/jammy/libssl-dev - current version of openssl is 3.0.2
#   should be fine if major stays at 3.

# 
system("apt update");

system("apt install -y g++");
system("apt install -y git");

system("apt install -y libssl-dev");
system("apt install -y libcrypto-dev");

system("apt install -y postgresql-common");
system("apt install -y graphviz");
system("apt install -y libgraphviz-dev");
system("/usr/share/postgresql-common/pgdg/apt.postgresql.org.sh");