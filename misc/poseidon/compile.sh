#!/bin/sh

if [ -n "$1" ] ; then
	if [ "$1" = "help" ] ; then
		echo -e "Usage:\n  $0 [binary]\n"
		exit 0
	fi
	binary="$1"
else
	binary="poseidon"
fi

ccache gcc -Wall -pedantic -ggdb -o $binary poseidon.c
if [ $? -eq 0 ] ; then
	echo "Finished. You can run the bot by ./$binary"
fi
