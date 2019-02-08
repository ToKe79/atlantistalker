#!/bin/sh
CC="ccache gcc"
PROJECT="passhash"
LIBS="-lmysqlclient -ldl -rdynamic $(xml2-config --libs)"
CCFLAGS="-Wall -ggdb -pedantic -I. $(xml2-config --cflags)"
IN="passhash.c"

${CC} ${CCFLAGS} ${IN} -o ${PROJECT} ${LIBS}
