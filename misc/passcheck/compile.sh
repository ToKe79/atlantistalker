#!/bin/sh
CC="ccache gcc"
PROJECT="passcheck"
LIBS="-lmysqlclient -ldl -rdynamic $(xml2-config --libs)"
CCFLAGS="-Wall -ggdb -pedantic -I. $(xml2-config --cflags)"
IN="passcheck.c"

${CC} ${CCFLAGS} ${IN} -o ${PROJECT} ${LIBS}
