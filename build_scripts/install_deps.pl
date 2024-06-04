#!/usr/bin/perl
# because wtf even is awk/cut...

my $VERSION = `lsb_release -c`;

if ($VERSION =~ /.+:\s+(.+)/) {
    if ($1 ne "jammy") {
        print("Invalid version of ubuntu: " . $1);
        exit;
    }
}

system("sudo apt update");

system("sudo apt install libssl-dev");
system("sudo apt install libcrypto-dev");

system("sudo apt install -y postgresql-common");
system("sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh");
