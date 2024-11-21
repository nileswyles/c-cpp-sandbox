#!/bin/perl

my $PATH_TO_FILE = "";
for (my $i = 0; $i < scalar(@ARGV); $i++) {
	if ($ARGV[$i] eq "-p" || $ARGV[$i] eq "--path") {
        $PATH_TO_FILE = $ARGV[++$i];
	} elsif ($ARGV[$i] eq "-l" || $ARGV[$i] eq "--log") {
        $LOG_LEVEL = $ARGV[++$i];
	}
}
if ($PATH_TO_FILE eq "") {
    print("This file is required.");
    exit(1);
}
open(my $fh, "<", $PATH_TO_FILE);
print("Test function calls: \n");
while (<$fh>) {
    if ($_ =~ /\/\/.*void (.*)\(TestArg \* .*\).*\{/) {
    } else {
        # TODO:
        #   don't include if inline commented. (negative lookbehind isn't working and I don't care to figure that out)
        if ($_ =~ /(?<!\/\/).*void (.*)\(TestArg \* .*\).*\{/) {
            print("t.addTest($1);\n");
        }
    }
}
print("\n\n");
print("Test function declarations: \n");
open(my $fh, "<", $PATH_TO_FILE);
while (<$fh>) {
    if ($_ =~ /\/\/.*void (.*)\(TestArg \* .*\).*\{/) {
    } else {
        if ($_ =~ /(?<!\/\/).*void (.*)\(TestArg \* .*\).*\{/) {
            print("static void $1(TestArg * t);\n");
        }
    }
}