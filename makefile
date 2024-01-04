# Name of file
NAME = watch_dog

# Path to directory
DIR_PATH = /home/matias/git/projects/WatchDog

# The compiler : gcc for C program :
CC = gcc

# Compiler flags :
CFLAGS = -ansi -pedantic-errors -Wall -Wextra -lpthread -lrt -fPIC -c

# Valgrind
VALGRIND = valgrind --leak-check=yes --track-origins=yes -s

# Debugtest
DEBUG = gdb -tui

#Remove
RM = rm -rf

# Path to header
PATH_TO_HEADER = -I$(DIR_PATH)/include/

# User main file
USER_MAIN = $(DIR_PATH)/test/$(NAME)_test.c

# main file
MAIN = $(DIR_PATH)/src/wd_$(NAME).c

# User o main file
O_USER_MAIN = $(DIR_PATH)/lib/$(NAME)_test.o

# Main file
O_MAIN = $(DIR_PATH)/lib/wd_$(NAME).o

# Shared Lib names
SO_NAME = $(DIR_PATH)/dist/libwatchdog.so

# Shared lib path
PATH_TO_SO = $(DIR_PATH)/dist

# Source files, headers, Object files

SRC = $(DIR_PATH)/src/$(NAME).c
O_SRC = $(DIR_PATH)/lib/$(NAME).o
HEADER = $(DIR_PATH)/include/$(NAME).h

EXTERNAL_O_SRC = $(DIR_PATH)/lib/wd_heap.o
EXTERNAL_SRC = $(DIR_PATH)/src/wd_heap.c
EXTERNAL_HEADER = $(DIR_PATH)/include/wd_heap.h

EXTERNAL_O_SRC_1 = $(DIR_PATH)/lib/wd_heap_pq.o
EXTERNAL_SRC_1 = $(DIR_PATH)/src/wd_heap_pq.c
EXTERNAL_HEADER_1 = $(DIR_PATH)/include/wd_heap_pq.h

EXTERNAL_O_SRC_2 = $(DIR_PATH)/lib/wd_heap_scheduler.o
EXTERNAL_SRC_2 = $(DIR_PATH)/src/wd_heap_scheduler.c
EXTERNAL_HEADER_2 = $(DIR_PATH)/include/wd_heap_scheduler.h

EXTERNAL_O_SRC_3 = $(DIR_PATH)/lib/wd_vector.o
EXTERNAL_SRC_3 = $(DIR_PATH)/src/wd_vector.c
EXTERNAL_HEADER_3 = $(DIR_PATH)/include/wd_vector.h

EXTERNAL_O_SRC_4 = $(DIR_PATH)/lib/wd_task.o
EXTERNAL_SRC_4 = $(DIR_PATH)/src/wd_task.c
EXTERNAL_HEADER_4 = $(DIR_PATH)/include/wd_task.h

EXTERNAL_O_SRC_5 = $(DIR_PATH)/lib/wd_unique_identifier.o
EXTERNAL_SRC_5 = $(DIR_PATH)/src/wd_unique_identifier.c
EXTERNAL_HEADER_5 = $(DIR_PATH)/include/wd_unique_identifier.h

# The build target executable
TARGET = $(DIR_PATH)/dist/watchdog.out

# The usertest build target executable
TEST_TARGET = $(DIR_PATH)/test/watchdog_test.out

# C Files of the project
C_FILES = $(SRC) $(EXTERNAL_SRC) $(EXTERNAL_SRC_1) $(EXTERNAL_SRC_2) $(EXTERNAL_SRC_3) $(EXTERNAL_SRC_4) $(EXTERNAL_SRC_5)

# O Files of the project
O_FILES = $(O_SRC) $(EXTERNAL_O_SRC) $(EXTERNAL_O_SRC_1) $(EXTERNAL_O_SRC_2) $(EXTERNAL_O_SRC_3) $(EXTERNAL_O_SRC_4) $(EXTERNAL_O_SRC_5)

# List of all headers
HEADERS = $(HEADER) $(EXTERNAL_HEADER) $(EXTERNAL_HEADER_1) $(EXTERNAL_HEADER_2) $(EXTERNAL_HEADER_3) $(EXTERNAL_HEADER_4) $(EXTERNAL_HEADER_5)

.PHONY : run link_test vlg release debug clean

#******************************************************************************

release : CFLAGS += -DNDEBUG -O3
release : clean $(SO_NAME)

#******************************************************************************

debug : CFLAGS += -DDEBUG_ON -g
debug : clean link_test
	clear
	$(DEBUG) $(TEST_TARGET)

#******************************************************************************

run : release link_test
	
	$(TEST_TARGET)

#******************************************************************************

$(SO_NAME) : $(O_FILES) $(HEADERS)
	$(CC) $(PATH_TO_HEADER) -shared -o $(SO_NAME) $(O_FILES)
	$(CC) $(PATH_TO_HEADER) -L$(PATH_TO_SO) -Wl,-rpath=$(PATH_TO_SO) -Wall $(MAIN) -o $(TARGET) -lwatchdog

#******************************************************************************

link_test : $(USER_MAIN) $(SO_NAME)
	$(CC) $(CFLAGS) $(PATH_TO_HEADER) -c $(USER_MAIN) -o $(O_USER_MAIN)
	$(CC) $(PATH_TO_HEADER) -L$(PATH_TO_SO) -Wl,-rpath=$(PATH_TO_SO) $(O_USER_MAIN) -o $(TEST_TARGET) -lwatchdog


#******************************************************************************

vlg : link_test
	clear
	$(VALGRIND) $(TEST_TARGET)

#******************************************************************************

$(O_SRC) : $(SRC) $(HEADER)
	$(CC) $(PATH_TO_HEADER) $(CFLAGS) -c $(SRC) -o $(O_SRC)

$(EXTERNAL_O_SRC) : $(EXTERNAL_SRC) $(EXTERNAL_HEADER)
	$(CC) $(PATH_TO_HEADER) $(CFLAGS) -c $(EXTERNAL_SRC) -o $(EXTERNAL_O_SRC)

$(EXTERNAL_O_SRC_1) : $(EXTERNAL_SRC_1) $(EXTERNAL_HEADER_1)
	$(CC) $(PATH_TO_HEADER) $(CFLAGS) -c $(EXTERNAL_SRC_1) -o $(EXTERNAL_O_SRC_1)

$(EXTERNAL_O_SRC_2) : $(EXTERNAL_SRC_2) $(EXTERNAL_HEADER_2)
	$(CC) $(PATH_TO_HEADER) $(CFLAGS) -c $(EXTERNAL_SRC_2) -o $(EXTERNAL_O_SRC_2)

$(EXTERNAL_O_SRC_3) : $(EXTERNAL_SRC_3) $(EXTERNAL_HEADER_3)
	$(CC) $(PATH_TO_HEADER) $(CFLAGS) -c $(EXTERNAL_SRC_3) -o $(EXTERNAL_O_SRC_3)

$(EXTERNAL_O_SRC_4) : $(EXTERNAL_SRC_4) $(EXTERNAL_HEADER_4)
	$(CC) $(PATH_TO_HEADER) $(CFLAGS) -c $(EXTERNAL_SRC_4) -o $(EXTERNAL_O_SRC_4)

$(EXTERNAL_O_SRC_5) : $(EXTERNAL_SRC_5) $(EXTERNAL_HEADER_5)
	$(CC) $(PATH_TO_HEADER) $(CFLAGS) -c $(EXTERNAL_SRC_5) -o $(EXTERNAL_O_SRC_5)

#******************************************************************************

clean :
	clear
	$(RM) $(TARGET) $(O_FILES) $(SO_NAME) $(TEST_TARGET)

#******************************************************************************