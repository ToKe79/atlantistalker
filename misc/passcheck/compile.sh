#!/bin/sh
CC="ccache gcc"
PROJECT="passcheck"
LIBS="-lmysqlclient -ldl -rdynamic $(xml2-config --libs)"
CFLAGS="-Wall -Wextra -Wformat=2 -ggdb -pedantic -I. $(xml2-config --cflags)"
IN="passcheck.c"

if [ "${HAVE_CONFIG}" = "1" ] ; then
	CFLAGS="-DHAVE_CONFIG ${CFLAGS}"
fi

${CC} ${CFLAGS} ${IN} -o ${PROJECT} ${LIBS}
