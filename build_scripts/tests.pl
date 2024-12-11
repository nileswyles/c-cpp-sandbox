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
        $DEFINES .= "-D $ARGV[++$i] ";
	}
}
my @TEST_SUITES = split(' ', `ls $PATH_TO_TEST_DIRECTORY`);
my @TEST_SUITES_TO_SKIP = ("http_server_test.sh", "enew_test.sh", "matrix_test.sh", "csv_test.sh");

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

my $timeout = 60; 
my %EXTENDED_TIMES = ("etasker_test.sh" => $timeout * 10);

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
        my $CMD = "$PATH_TO_TEST_DIRECTORY/$test --log $LOG_LEVEL $DEFINES";
        my $actual_timeout = int($timeout);
        if ($EXTENDED_TIMES{$test} > $actual_timeout) {
            $actual_timeout = int($EXTENDED_TIMES{$test});
        }
        print("~Executing test:\n\t$CMD"."with timeout: $actual_timeout\n");
        my $time_before = time;
        my $result = system("etimeout/bin/etimeout $actual_timeout $CMD");
        my $result_code = $?;
        print("\n~Finished executing test.\n");
        # print("system result $result vs result code $result_code\n");

        my $time_elapsed = time - $time_before;
        my $mins = int($time_elapsed / 60);
        my $seconds = $time_elapsed - ($mins * 60);
        # here because timeout...
        print("\nTime elapsed: \n$mins Minutes and $seconds Seconds \n");

        if ($result != 0) {
            $failed_map{$test} = int($result_code);
        } else {
            $passed_map{$test} = int($result_code);
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