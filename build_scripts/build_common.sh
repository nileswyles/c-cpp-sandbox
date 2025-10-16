#!/bin/bash

if [ -z $WYLESLIBS_WORKSPACE_ROOT_DIR ]; then
	# typically run scripts from the c-cpp-sandbox workspace root directory
	WYLESLIBS_WORKSPACE_ROOT_DIR=`pwd`
fi

if [ -z $SCRIPTS_DIR ]; then
	export SCRIPTS_DIR="$WYLESLIBS_WORKSPACE_ROOT_DIR/.devcontainer/linux_goodness/scripts"
fi

NAME="program"
LOG_LEVEL=0
LD_FLAGS=""
LIB_SEARCH_PATHS=""
HEADER_SEARCH_PATHS=""
DEFINES=""
SRC_FILES=""
RUN_FROM="$WYLESLIBS_WORKSPACE_ROOT_DIR"
OUTPUT_DIR="$WYLESLIBS_WORKSPACE_ROOT_DIR/out"
OPTIMIZATION="-O3"
DEBUG=""
PROGRAM_ARG=""
while true; do
	case "$1" in
        -n|--name) NAME="$2"; shift 2 ;;
		--log) LOG_LEVEL="$2"; shift 2 ;;
		-l) LD_FLAGS="$LD_FLAGS -l $2"; shift 2 ;;
		-l*) LD_FLAGS="$LD_FLAGS $1"; shift 1 ;;
		-L) LIB_SEARCH_PATHS="$LIB_SEARCH_PATHS-L $2 "; shift 2 ;;
		-L*) LIB_SEARCH_PATHS="$LIB_SEARCH_PATHS$1 "; shift 1 ;;
		-I|--include) HEADER_SEARCH_PATHS="$HEADER_SEARCH_PATHS-I $2 "; shift 2 ;;
		-I*) HEADER_SEARCH_PATHS="$HEADER_SEARCH_PATHS$1 "; shift 1 ;;
		-D) DEFINES="$DEFINES -D $2"; shift 2 ;;
		-D*) DEFINES="$DEFINES $1"; shift 1 ;;
		-s|--source) SRC_FILES="$SRC_FILES $2"; shift 2 ;;
		-r|--run-from) RUN_FROM="$2"; shift 2 ;;
		-o|--output-dir) OUTPUT_DIR="$2"; shift 2 ;;
		-O) OPTIMIZATION="-O$2"; shift 2 ;;
		-Og) DEBUG="-g"; OPTIMIZATION="-Og" shift ;;
		-O*) OPTIMIZATION="$1 "; shift ;;
		-g) DEBUG="-g"; OPTIMIZATION="-Og" shift ;;
		# --) echo "PROGRAM ARG: $@"; break ;;
		*) PROGRAM_ARG=$@; break;;
	esac
done

# see logger.h for log level values
# TODO: remember to remove need to set LOGGER_LEVEL... this was a stop-gap to support legacy stuff, need to update all files.
#		And actually don't need to set GLOBAL_LOGGER_LEVEL in each file.
DEFINES=$DEFINES"
-D GLOBAL_LOGGER_LEVEL=$LOG_LEVEL
-D LOGGER_LEVEL=$LOG_LEVEL"
DEFINES=`echo "$DEFINES" | tr "\n" " "`

# Standardize this
HEADER_SEARCH_PATHS="$HEADER_SEARCH_PATHS
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/algo
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/datastructures
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/estream
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/file
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/memory
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/parser/csv
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/parser/json
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/parser/keyvalue
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/parser/multipart
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/threads
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/web
-I $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/web/http"
HEADER_SEARCH_PATHS=`echo "$HEADER_SEARCH_PATHS" | tr "\n" " "`

if [ $(cut -b 1 - <<< $OUTPUT_DIR) != "/" ]; then
	OUTPUT_DIR="$(pwd)/$OUTPUT_DIR"
fi
mkdir $OUTPUT_DIR 2> /dev/null
PROGRAM_PATH="$OUTPUT_DIR/$NAME.out"
rm $PROGRAM_PATH 2> /dev/null

if [ -z $CXX_COMPILER ]; then
	CXX_COMPILER="g++"
fi

REMOVED_WARNING_FLAGS="-Wno-unused-variable
-Wno-unused-parameter
-Wno-unused-function
-Wno-reorder
-Wno-deprecated-declarations
-Wno-format
-Wno-overloaded-virtual
-Wno-maybe-uninitialized
-Wno-unused-result"
REMOVED_WARNING_FLAGS=`echo "$REMOVED_WARNING_FLAGS" | tr "\n" " "`

echo "
~Build: "
BUILD_CMD="$CXX_COMPILER $DEBUG $HEADER_SEARCH_PATHS $LIB_SEARCH_PATHS $DEFINES $LD_FLAGS $OPTIMIZATION -std=c++20 -Wall $REMOVED_WARNING_FLAGS $SRC_FILES -o $PROGRAM_PATH"
perl -e 'my $cmd = "$ARGV[0]"; my $buf = ""; my @s = split(/ /, $cmd); foreach(@s) { if ($_ eq "-I" || $_ eq "-D" || $_ eq "-o" ) { $buf = "$_ "; } elsif (length($_)) { print ("\t$buf$_\n"); $buf = ""; }  }' "$BUILD_CMD"
$BUILD_CMD

echo "
~Executing Program: "
EXEC_CMD="cd $RUN_FROM && $PROGRAM_PATH $PROGRAM_ARG"
echo "$PROGRAM_PATH $PROGRAM_ARG
"
bash -c "$EXEC_CMD"