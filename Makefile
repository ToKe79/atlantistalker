# Atlantis Makefile :-)
# ===========================================================================

CC        = ccache gcc
PROJECT   = runtime/atlantis
VERSION   = $(PROJECT).version
XMLLIBS   = $(shell xml2-config --libs)
XMLCFLAGS = $(shell xml2-config --cflags)
LIBS      = -lmysqlclient -ldl -rdynamic -Wl,--version-script=$(VERSION) $(XMLLIBS)
CFLAGS    = -Wall -ggdb -I. $(XMLCFLAGS)
DEPENDS   = make.depend
SRCS      = ${wildcard *.c}
TODO      = $(patsubst %.c,%.o,$(SRCS))

.PHONY: clean libs

$(PROJECT): $(TODO) $(DEPENDS) $(VERSION)
	$(CC) $(CFLAGS) -o $(PROJECT) $(TODO) $(LIBS)

$(VERSION): atl-libapi.o
	@echo "Creating version script $(VERSION)"
	@(echo '{'; echo 'global:'; nm atl-libapi.o | grep ' api_' | cut -d ' ' -f 3 | sed 's/.*/\t\0;/' ; echo 'local: *;'; echo '};') > $(VERSION)

libs: libs/liblog.so

libs/liblog.so: libs/lib_log.c
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@

clean:
	@echo -n "Deleting all object files..."
	@rm -f *.o $(DEPENDS)
	@echo "OK"

%.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(DEPENDS):
	@echo "Building dependencies"
	@gcc -E -MM $(SRCS) > $(DEPENDS)

-include $(DEPENDS)
