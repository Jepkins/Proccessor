CC = g++
CFLAGS = -c -Wall -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual\
-Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat=2 -Winline\
-Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo\
-Wstrict-overflow=2 -Wsuggest-override -Wswitch-default -Wundef -Wunreachable-code -Wunused -Wvariadic-macros\
-Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -Wsuggest-override\
-Wlong-long -fopenmp -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer\
-Wlarger-than=8192 -Wstack-protector -fPIE -Werror=vla -MP -MMD

BUILD_DIR = build

PROC_DIR = tester
ASM_DIR = asm
COMMON_DIR = common

COMMON_OBJECTS = $(shell $(MAKE) -s -C $(COMMON_DIR) curr=$(COMMON_DIR) build='$(BUILD_DIR)' echo_objects)
PROC_OBJECTS = $(shell $(MAKE) -s -C $(PROC_DIR) curr=$(PROC_DIR) build='$(BUILD_DIR)' echo_objects)
ASM_OBJECTS   = $(shell $(MAKE) -s -C $(ASM_DIR) curr=$(ASM_DIR)   build='$(BUILD_DIR)' echo_objects)
COMMON_INCLUDES = $(shell $(MAKE) -s -C $(COMMON_DIR) curr=$(COMMON_DIR) echo_includes)

LDFLAGS += $(shell $(MAKE) -s -C $(COMMON_DIR) curr=$(COMMON_DIR) echo_libs)
EXECUTABLE_DIR = $(BUILD_DIR)/executable

PROC_EXEC = $(EXECUTABLE_DIR)/proc.exe
ASM_EXEC = $(EXECUTABLE_DIR)/asm.exe

.PHONY: all common proc asm

all: clean proc asm

proc: $(PROC_EXEC)
	@echo Successfully remade $<

asm: $(ASM_EXEC)
	@echo Successfully remade $<

common: $(COMMON_OBJECTS)

$(PROC_EXEC): $(COMMON_OBJECTS) $(PROC_OBJECTS)
	@echo making $@
	@mkdir -p $(@D)
	$(CC) $^ -o $@ $(LDFLAGS)

$(ASM_EXEC): $(COMMON_OBJECTS) $(ASM_OBJECTS)
	@echo making $@
	@mkdir -p $(@D)
	@$(CC) $^ -o $@ $(LDFLAGS)

$(COMMON_OBJECTS):
	@echo $(shell $(MAKE) -s -C $(COMMON_DIR) curr=$(COMMON_DIR) comp='$(CC)' flags='$(CFLAGS)' build='$(BUILD_DIR)')

$(PROC_OBJECTS):
	@echo $(shell $(MAKE) -s -C $(PROC_DIR) curr=$(PROC_DIR) comp='$(CC)' flags='$(CFLAGS)' build='$(BUILD_DIR)' common_incs='$(COMMON_INCLUDES)')

$(ASM_OBJECTS):
	@echo $(shell $(MAKE) -s -C $(ASM_DIR) curr=$(ASM_DIR) comp='$(CC)' flags='$(CFLAGS)' build='$(BUILD_DIR)' common_incs='$(COMMON_INCLUDES)')

clean:
	@rm -rf -d $(BUILD_DIR)

echo:
	@echo $(COMMON_OBJECTS)
	@echo $(PROC_OBJECTS)
	@echo $(ASM_OBJECTS)
	@echo $(COMMON_INCLUDES)
