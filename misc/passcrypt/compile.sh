#!/bin/sh
CC="ccache gcc"
PROJECT="passcrypt"
LIBS="-lmysqlclient -ldl -rdynamic $(xml2-config --libs)"
CCFLAGS="-Wall -ggdb -pedantic -I. $(xml2-config --cflags)"
IN="passcrypt.c"

${CC} ${CCFLAGS} ${IN} -o ${PROJECT} ${LIBS}
