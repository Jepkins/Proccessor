CURRENT_DIR = $(curr)
CC = $(comp)
CFLAGS = $(flags)
BUILD_DIR = $(build)/$(CURRENT_DIR)
COMMON_INCLUDES = $(addprefix -I../, $(common_incs))

SRC_DIR = source
INC_DIR = includes

SOURCES = $(wildcard $(addsuffix *.cpp, $(SRC_DIR)/))
OBJECTS = $(addprefix $(BUILD_DIR)/, $(SOURCES:.cpp=.o))
DEPS = $(addsuffix .d, $(basename $(OBJECTS)))

CFLAGS += $(addprefix -I, $(INC_DIR)) $(COMMON_INCLUDES)

-include $(DEPS)

all: $(addprefix ../, $(OBJECTS))

../$(BUILD_DIR)/%.o: %.cpp
	@echo making $(@:../%=%)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) $< -o $@

echo_objects:
	@echo $(OBJECTS)

echo:
	@echo $(COMMON_INCLUDES)
