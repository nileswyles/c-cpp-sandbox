#!/bin/sh

BENCH=0
LOG_LEVEL=0
SORT_FILE="nlognsort"
while true; do
	case "$1" in
		-b|--bench) BENCH=1; shift 1 ;; 
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-s|--sort) SORT_FILE="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES -D$2"; shift 2 ;;
		# --) shift; break ;;
		*) break;;
	esac
done

DEFINES=$DEFINES"
-DGLOBAL_LOGGER_LEVEL=$LOG_LEVEL
-DLOGGER_LEVEL=$LOG_LEVEL
"

ROOT_DIR="."

SRC_FILES="
$ROOT_DIR/cpping/$SORT_FILE.cpp
"

mkdir $ROOT_DIR/out 2> /dev/null
TEST_PATH=$ROOT_DIR/out/$SORT_FILE.out
GRAPH_OUT=$ROOT_DIR/out/$SORT_FILE.gif
rm $TEST_PATH
rm $GRAPH_OUT

# obtained through `pkg-config libgvc --cflags` 
# -I/usr/include/graphviz
# -lgvc -lcgraph -lcdt
g++ $SRC_FILES -I/usr/include/graphviz -lgvc -lcgraph -lcdt -std=c++20 -iquote $ROOT_DIR/cpping -iquote $ROOT_DIR/lib $DEFINES -o $TEST_PATH
# if [BENCH -eq 1]; then
# 	while true; do
# 		exec $TEST_PATH -K dot -Tgif -o$GRAPH_OUT
# 	done
# else;
	exec $TEST_PATH -K dot -Tgif -o$GRAPH_OUT
# fi