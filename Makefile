
CC = $(CROSS)gcc
CFLAGS += -c -Wall -g -O0 -I$(APP_INC_DIR)
#LDFLAGS = $(APP_LIB_DIR)/libipc.a $(APP_LIB_DIR)/libixml.a -lpthread

# -print-search-dirs
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE=recorder

.PHONY : all install clean

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	@echo "---------- Build ${EXECUTABLE} application"
	-@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	@echo "---------- Build $<"
	-@$(CC) $(CFLAGS) $< -o $@
	

install:
	@echo -e ${BLUE}'Installing monitor...'${WHITE}
	install recorder $(APP_OUT_DIR)/bin

clean:
	@echo "---------- Remove ${EXECUTABLE} application"
	@$ if [ -e ${EXECUTABLE} ]; then rm ${EXECUTABLE}; fi;
	@echo "---------- Remove object files"
	@$ rm -f *.o
	
