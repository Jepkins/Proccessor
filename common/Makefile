CURRENT_DIR = $(curr)
CC = $(comp)
CFLAGS = $(flags)
BUILD_DIR = $(build)/$(CURRENT_DIR)

LIBS =

ifeq ($(d_debug),true)
STACK_LIBS += stack_debug.a
else
STACK_LIBS += stack_release.a
endif
LIB_INCS += -L ./$(CURRENT_DIR)/stack $(addprefix -l:, $(STACK_LIBS))

SUBDIRS = $(wildcard */)

SOURCES = $(wildcard $(addsuffix *.cpp, $(SUBDIRS)))
OBJECTS = $(addprefix $(BUILD_DIR), $(SOURCES:.cpp=.o))
DEPS = $(addsuffix .d, $(basename $(OBJECTS)))

LOCAL_INCLUDES = $(addprefix -I, $(SUBDIRS))
GLOBAL_INCLUDES = $(addprefix $(CURRENT_DIR)/, $(SUBDIRS))
CFLAGS += $(LOCAL_INCLUDES)

all: $(addprefix ../, $(OBJECTS))

-include $(DEPS)

../$(BUILD_DIR)%.o: %.cpp
	@echo making $(@:../%=%)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

echo_objects:
	@echo $(OBJECTS)

echo_includes:
	@echo $(GLOBAL_INCLUDES)

echo_libs:
	@echo $(LIB_INCS)







