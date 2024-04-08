# - - - - - - - - - - - - - - - - - - - - - - - -
#
# Makefile pro Loxie
# Compila a linguagem
# 
# Uso (rode pelo CMD no mesmo diretório do Makefile):
# make -f Makefile [all|fresh|clean|reformat|document] [RELEASE="Y"]  \
#                  [PRINT_CODE="Y"] [STACK_TRACE="Y"] [STRESS_GC="Y"] \
#                  [LOG_GC="Y"]
#
# Alvos:
# - all: Compila tudo;
# - fresh: Limpa os executáveis, objetos, reformata, documenta e compila.
#          Basicamente compila do zero;
# - clean: Limpa os executáveis e objetos. CUIDADO! Pode apagar coisar
#          indesejadas se usado de forma incorreta;
# - reformat: Reformata o código com clang-format. clang-format precisa
#             estar no seu PATH;
# - document: Documenta o código com doxygen. doxygen precisa estar no seu
#             PATH.
#
# Variáveis:
# - RELEASE: Compila com otimizações (e sem debug symbols);
# - PRINT_CODE: Imprime os opcodes gerados durante a compilação;
# - STACK_TRACE: Imprime o estado da pilha + opcodes durante a interpretação;
# - STRESS_GC: Tenta limpar o lixo todo o tempo possível;
# - LOG_GC: Imprime o que o coletor de lixo está fazendo atualmente.
#
# - - - - - - - - - - - - - - - - - - - - - - - -

# -- Pré-compilação --

# Compilador
CC := gcc
LD := gcc

CFLAGS := -Wall -Wextra -pedantic

# Caminhos
BASE := $(CURDIR)

SRC := $(BASE)/src
INC := $(BASE)/inc
OUT := $(BASE)/out
OBJ := $(BASE)/obj
DOC := $(BASE)/doc

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

.PHONY: all clean reformat document fresh

# Compila
all: $(OBJS)
	@echo
	@echo Linking $@
	@echo ...
	@echo
	$(LD) $(OBJS) -o $(OUT)/loxiec.exe
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

# Reformat
# Reformata o código usando clang-format
reformat:
	clang-format $(SRC)/*.c -style=file -i
	clang-format $(INC)/*.h -style=file -i

# Document
# Compila a documentação usando o doxygen
document:
	doxygen $(DOC)/doxyfile
	
# Fresh
# Limpa os objetos/exe, reformat o código, documenta e compila de novo
fresh: clean reformat document all

