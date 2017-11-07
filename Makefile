# Atlantis Makefile :-)
# ===========================================================================

CC	= ccache gcc
PROJECT = atlantis.exe

#
# Nasledujuce 2 riadky sluzia pre kompilaciu pod Window
#

#LIBS	= -L/usr/local/lib/mysql -lmysql
#CCFLAGS = -Wall -DWIN32

#
# Toto uz je pre Linuziqoqoze
#
# LIBS    = -L/usr/local/lib/mysql -lmysqlclient -lccmalloc -ldl
LIBS    = -L /usr/local/mysql/lib/mysql -lmysqlclient -ldl -rdynamic -Wl,--version-script=$(PROJECT_VERSION_SCRIPT) $(XMLLIBS)
XMLLIBS	= $(shell xml2-config --libs)
XMLFLAGS	= $(shell xml2-config --cflags)
CCFLAGS = -Wall -ggdb -I. $(XMLFLAGS)
DEPENDS = make.depend
PROJECT_VERSION_SCRIPT = $(PROJECT).version
SRCS    = ${wildcard *.c}
TODO    = $(patsubst %.c,%.o,$(SRCS))

.PHONY: clean doc libs

$(PROJECT): $(TODO) $(DEPENDS) $(PROJECT_VERSION_SCRIPT)
	$(CC) $(CCFLAGS) -o $(PROJECT) $(TODO) $(LIBS)

$(PROJECT_VERSION_SCRIPT): atl-libapi.o
	@echo "Creating version script $(PROJECT_VERSION_SCRIPT)"
	@(echo '{'; echo 'global:'; nm atl-libapi.o | grep ' api_' | cut -d ' ' -f 3 | sed 's/.*/\t\0;/' ; echo 'local: *;'; echo '};') > $(PROJECT_VERSION_SCRIPT)

libs: libs/liblog.so

libs/liblog.so: libs/lib_log.c
	$(CC) $(CCFLAGS) -shared $< -o $@

clean:
	@echo -n "Deleting all object files..."
	@rm -f *.o $(PROJECT) $(DEPENDS) $(PROJECT_VERSION_SCRIPT)
	@echo "OK"

%.o:
	$(CC) $(CCFLAGS) -c $< -o $@

$(DEPENDS):
	@echo "Building dependencies"
	@gcc -E -MM $(SRCS) > $(DEPENDS)

doc:
	@echo -n "Creating documentation.."
	robodoc --rc robodoc.rc

-include $(DEPENDS)


