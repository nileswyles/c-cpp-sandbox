DEFINES=""
LOG_LEVEL=0
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		*) break;;
	esac
done

g++ etimeout/main.cpp -iquote lib -Wall -D LOGGER_LEVEL=$LOG_LEVEL $DEFINES-o etimeout/out/etimeout