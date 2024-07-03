#!/bin/sh

SORT_FILE="nlognsort"
while true; do
	case "$1" in
		-s|--sort) SORT_FILE="$2"; shift 2 ;;
		# --) shift; break ;;
		*) break;;
	esac
done

ROOT_DIR="."

SRC_FILES="
$ROOT_DIR/cpping/$SORT_FILE.cpp
"

mkdir $ROOT_DIR/out
TEST_PATH=$ROOT_DIR/out/$SORT_FILE.out
GRAPH_OUT=$ROOT_DIR/out/$SORT_FILE.gif
rm $TEST_PATH
rm $GRAPH_OUT

# obtained through `pkg-config libgvc --cflags` 
# -I/usr/include/graphviz
# -lgvc -lcgraph -lcdt
g++ $SRC_FILES -I/usr/include/graphviz -lgvc -lcgraph -lcdt -std=c++20 -iquote $ROOT_DIR/cpping -o $TEST_PATH
exec $TEST_PATH -K dot -Tgif -o$GRAPH_OUT