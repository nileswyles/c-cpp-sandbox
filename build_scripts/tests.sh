#!/bin/bash

# run all of the test suites... 
PATH_TO_TEST_DIRECTORY="build_scripts/tests"

LOG_LEVEL=0
while true; do
	case "$1" in
        -p|--path) PATH_TO_TEST_DIRECTORY="$2"; shift 2;;
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES -D$2"; shift 2 ;;
		# --) shift; break ;;
		*) break;;
	esac
done

TEST_SUITES=`ls $PATH_TO_TEST_DIRECTORY`
TEST_SUITES_TO_SKIP="http_server_test.sh enew_test.sh"

# SUMMARY
echo "\t\t\tTEST SUMMARY\n\n"

echo "
TEST SUITES DETECTED:\n\n\t$TEST_SUITES"
echo "
TEST SUITES TO SKIP:\n\n\t$TEST_SUITES_TO_SKIP\n"

DELIMETER="-------------------------------------------------------------------"

for test in $TEST_SUITES; do
    echo $DELIMETER
    SKIP="no"
    for test_to_skip in $TEST_SUITES_TO_SKIP; do
        if [ "$test" = "$test_to_skip" ]; then
            SKIP="yes"
        fi
    done
    if [ "$SKIP" = "no" ]; then
        CMD="$PATH_TO_TEST_DIRECTORY/$test --log $LOG_LEVEL $DEFINES"
        echo "~Executing test:\n\t$CMD"
        `$CMD`
    else
        echo "~Skipping $test"
    fi
done