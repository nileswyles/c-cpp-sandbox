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

system("apt update");

system("apt install -y g++");
system("apt install -y git");

system("apt install -y libssl-dev");
system("apt install -y libcrypto-dev");

system("apt install -y postgresql-common");
system("apt install -y graphviz");
system("apt install -y libgraphviz-dev");
system("/usr/share/postgresql-common/pgdg/apt.postgresql.org.sh");