#!/bin/bash

# Parse arguments
for ARGUMENT in "$@"
do
	KEY=$(echo $ARGUMENT | cut -f1 -d=)
	KEY_LENGTH=${#KEY}
	VALUE="${ARGUMENT:$KEY_LENGTH+1}"

	export "$KEY"="$VALUE"
done

if [ -z "$CONFIG" ]; then
	CONFIG="relwithdebug"
fi

if [ -z $CC ]; then
	CC=gcc
fi

if [ -z $JOBS ]; then
	JOBS=$(($(nproc) - 1))
fi

echo "==== Options ===="
echo "Configuration: $CONFIG"
echo "Compiler: $CC"
echo "Make Jobs: $JOBS"
echo ""

# (Re)Generate Project Files
echo "==== Genearting Project Files ===="
premake5 vscode --cc=$CC
premake5 gmake2 --cc=$CC
echo ""

# Run make
echo "==== Building All ($CONFIG, $CC) ===="
make -j$JOBS config=$CONFIG

