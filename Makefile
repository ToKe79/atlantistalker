# Atlantis Makefile :-)
# ===========================================================================

CC        = ccache gcc
PROJECT   = runtime/atlantis
VERSION   = $(PROJECT).version
XMLLIBS   = $(shell xml2-config --libs)
XMLCFLAGS = $(shell xml2-config --cflags)
LIBS      = -lmysqlclient -ldl -rdynamic -Wl,--version-script=$(VERSION) $(XMLLIBS)
CFLAGS    = -Wall $(XMLCFLAGS)
DEPENDS   = make.depend
SRCS      = ${wildcard *.c}
TODO      = $(patsubst %.c,%.o,$(SRCS))
DEFINES   =

ifeq ($(HAVE_CONFIG), 1)
DEFINES += -DHAVE_CONFIG
endif

ifeq ($(DEBUG), 1)
CFLAGS += -ggdb
endif

ifeq ($(MOREWARN), 1)
CFLAGS += -Wextra -Wformat=2 -Wmisleading-indentation
endif

.PHONY: clean libs

$(PROJECT): $(TODO) $(DEPENDS) $(VERSION)
	$(CC) $(CFLAGS) $(DEFINES) -o $(PROJECT) $(TODO) $(LIBS)

$(VERSION): atl-libapi.o
	@echo "Creating version script $(VERSION)"
	@(echo '{'; echo 'global:'; nm atl-libapi.o | grep ' api_' | cut -d ' ' -f 3 | sed 's/.*/\t\0;/' ; echo 'local: *;'; echo '};') > $(VERSION)

libs: libs/liblog.so

libs/liblog.so: libs/lib_log.c
	$(CC) $(CFLAGS) $(DEFINES) -fPIC -shared $< -o $@

clean:
	@echo "Deleting all object files..."
	@rm -vf *.o libs/*.so $(DEPENDS)
	@echo "Done!"

%.o:
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

$(DEPENDS):
	@echo "Building dependencies"
	@gcc -E -MM $(SRCS) > $(DEPENDS)

-include $(DEPENDS)
