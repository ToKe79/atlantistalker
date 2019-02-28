#!/bin/sh
CC="ccache gcc"
PROJECT="passhash"
LIBS="-lmysqlclient -ldl -rdynamic $(xml2-config --libs)"
CFLAGS="-Wall -Wextra -Wformat=2 -ggdb -pedantic -I. $(xml2-config --cflags)"
IN="passhash.c"

${CC} ${CFLAGS} ${IN} -o ${PROJECT} ${LIBS}
