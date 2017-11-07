SRCS = main.c
CC = gcc
PROJECT = webtel
DEPENDS = make.depend
CC_FLAGS = -Wall -pedantic -O3

$(PROJECT): main.o
	$(CC) $(CC_FLAGS) -lmysqlclient main.o -o $(PROJECT)

main.o:
	$(CC) $(CC_FLAGS) -c main.c -o main.o

clean:
	rm -f *.o $(PROJECT) $(DEPENDS)

depend make.depend:
	@$(CC) -E -MM $(SRCS) > $(DEPENDS)

-include $(DEPENDS)

