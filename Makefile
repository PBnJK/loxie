# - - - - - - - - - - - - - - - - - - - - - - - -
#
# Makefile
# Compila a linguagem
# 
# Uso:
# mingw32-make -f Makefile [all|clean] [DEBUG=Y]
#
# - - - - - - - - - - - - - - - - - - - - - - - -

# -- Pré-compilação --

# Compilador
CC := gcc
LD := gcc

CFLAGS := -Wall -Wextra -pedantic

# Caminhos
BASE := C:/neat-lang

SRC := $(BASE)/src
INC := $(BASE)/inc
OUT := $(BASE)/out
OBJ := $(BASE)/obj

CFLAGS += -I$(INC)

# Códigos-fonte
SRCS := $(wildcard $(SRC)/*.c )
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

ifeq ($(RELEASE),Y)
	CFLAGS += -O3
else
	CFLAGS += -g
endif

ifeq ($(PRINT_CODE),Y)
	CFLAGS += -DDEBUG_PRINT_CODE
endif

ifeq ($(STACK_TRACE),Y)
	CFLAGS += -DDEBUG_TRACE_EXECUTION
endif

ifeq ($(STRESS_GC),Y)
	CFLAGS += -DDEBUG_STRESS_GC
endif

ifeq ($(LOG_GC),Y)
	CFLAGS += -DDEBUG_LOG_GC
endif

# -- Main --

.PHONY: all clean fresh

# Compila
all: $(OBJS)
	@echo Linking $@
	@echo ...
	@echo
	$(LD) $(OBJS) -o $(OUT)/neatc.exe
	@echo
	@echo All done!

$(OBJ)/%.o: $(SRC)/%.c
	@echo Compiling $@
	@echo ...
	@echo
	$(CC) -c $(CFLAGS) $^ -o $@
	@echo
	@echo Done
	@echo
    
# Clean
# rm -rf basicamente
clean:
	$(RM) $(OUT)/*.exe
	$(RM) $(OBJ)/*.o
	clear

# Fresh
# Limpa os objetos/exe e compila de novo
fresh: clean all

