#!/bin/perl

# run all of the test suites... 
my $PATH_TO_TEST_DIRECTORY="build_scripts/tests";
my $LOG_LEVEL = 0;
my $DEFINES = "";

# yeah but argparse would have still been good...
for (my $i = 0; $i < scalar(@ARGV); $i++) {
	if ($ARGV[$i] eq "-p" || $ARGV[$i] eq "--path") {
        $PATH_TO_TEST_DIRECTORY = $ARGV[++$i];
	} elsif ($ARGV[$i] eq "-l" || $ARGV[$i] eq "--log") {
        $LOG_LEVEL = $ARGV[++$i];
	} elsif ($ARGV[$i] eq "-D") {
        $DEFINES .= $ARGV[++$i];
	}
}
my @TEST_SUITES = split(' ', `ls $PATH_TO_TEST_DIRECTORY`);
my @TEST_SUITES_TO_SKIP = ("http_server_test.sh", "enew_test.sh");

# SUMMARY
print("\t\t\tTest Summary\n");

print("\nTEST SUITES DETECTED:\n");
foreach(@TEST_SUITES) {
    print("\t$_\n");
}
print("\nTEST SUITES TO SKIP:\n");
foreach(@TEST_SUITES_TO_SKIP) {
    print("\t$_\n");
}

my $DELIMETER= "\n\n@###################################################################\n";

my %passed_map = ();
my %failed_map = ();
foreach (@TEST_SUITES) {
    print($DELIMETER);

    my $test = $_;
    my $skip = 0;
    foreach (@TEST_SUITES_TO_SKIP) {
        if ($test eq $_) {
            $skip = 1;
        }
    }
    if (!$skip) {
        my $CMD = "$PATH_TO_TEST_DIRECTORY/$test -l $LOG_LEVEL $DEFINES";
        print("~Executing test:\n\t$CMD\n");
        my $result_code = system($CMD);
        if ($result_code) {
            $failed_map{$test} = $result_code;
        } else {
            $passed_map{$test} = $result_code;
        }
    } else {
        print("~Skipping $test\n");
    }
}

print($DELIMETER);
print("\t\t\tResult Summary\n");
if (scalar(%passed_map)) {
    print("Passed Tests\n");
    foreach (keys %passed_map) {
        print("\t$_ with code $passed_map{$_}\n");
    }
}

if (scalar(%failed_map)) {
    print("Failed Tests\n");
    foreach (keys %failed_map) {
        print("\t$_ with code $failed_map{$_}\n");
    }
}