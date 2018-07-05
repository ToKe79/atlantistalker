# Atlantis Makefile :-)
# ===========================================================================

CC	= ccache gcc
PROJECT = runtime/atlantis
LIBS    = -lmysqlclient -ldl -rdynamic -Wl,--version-script=$(PROJECT_VERSION_SCRIPT) $(XMLLIBS)
XMLLIBS	= $(shell xml2-config --libs)
XMLFLAGS= $(shell xml2-config --cflags)
CCFLAGS = -Wall -ggdb -I. $(XMLFLAGS)
DEPENDS = make.depend
SRCS    = ${wildcard *.c}
TODO    = $(patsubst %.c,%.o,$(SRCS))

PROJECT_VERSION_SCRIPT = $(PROJECT).version

.PHONY: clean libs

$(PROJECT): $(TODO) $(DEPENDS) $(PROJECT_VERSION_SCRIPT)
	$(CC) $(CCFLAGS) -o $(PROJECT) $(TODO) $(LIBS)

$(PROJECT_VERSION_SCRIPT): atl-libapi.o
	@echo "Creating version script $(PROJECT_VERSION_SCRIPT)"
	@(echo '{'; echo 'global:'; nm atl-libapi.o | grep ' api_' | cut -d ' ' -f 3 | sed 's/.*/\t\0;/' ; echo 'local: *;'; echo '};') > $(PROJECT_VERSION_SCRIPT)

libs: libs/liblog.so

libs/liblog.so: libs/lib_log.c
	$(CC) $(CCFLAGS) -fPIC -shared $< -o $@

clean:
	@echo -n "Deleting all object files..."
	@rm -f *.o $(PROJECT) $(DEPENDS) $(PROJECT_VERSION_SCRIPT)
	@echo "OK"

%.o:
	$(CC) $(CCFLAGS) -c $< -o $@

$(DEPENDS):
	@echo "Building dependencies"
	@gcc -E -MM $(SRCS) > $(DEPENDS)
	@mkdir -p runtime

-include $(DEPENDS)
