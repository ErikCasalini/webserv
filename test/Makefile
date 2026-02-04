#################
# ALIASES/FLAGS #
#################
CXX := c++
CXXFLAGS := -std=c++98 -g3 -Wall -Werror -Wextra -MMD
# -Wconversion -Wsign-conversion

#########
# FILES #
#########
SRC_DIR := ../src
LIB_DIR := ../src/lib
BUILD_DIR := build

SRC_COMMON := lib_test.cpp

vpath %.cpp $(SRC_DIR):$(LIB_DIR)

SRC_REQUEST_PARSER := $(SRC_COMMON) \
					  test_request_parser.cpp \
					  request_parser.cpp \
					  http_types.cpp \
					  cctype_cast.cpp
OBJ_REQUEST_PARSER := $(SRC_REQUEST_PARSER:%.cpp=$(BUILD_DIR)/%.o)

SRC_LIB_CCTYPE_CAST := $(SRC_COMMON) \
					  test_lib_cctype_cast.cpp \
					  cctype_cast.cpp
OBJ_LIB_CCTYPE_CAST := $(SRC_LIB_CCTYPE_CAST:%.cpp=$(BUILD_DIR)/%.o)

SRC_RESPONSE := $(SRC_COMMON) \
				test_Response.cpp \
				Response.cpp \
				http_types.cpp
OBJ_RESPONSE := $(SRC_RESPONSE:%.cpp=$(BUILD_DIR)/%.o)

RUNNERS := run_test_request_parser \
		   run_test_lib_cctype_cast \
		   run_test_Response

DEP_FILES := $(OBJ_REQUEST_PARSER:.o=.d) \
			 $(OBJ_LIB_CCTYPE_CAST:.o=.d) \
			 $(OBJ_RESPONSE:.o=.d)

###########
# TARGETS #
###########
.PHONY: all clean fclean re run gdb

all: $(BUILD_DIR) $(RUNNERS)

run_all = @for i in $(1); do echo "\033[1m$$i:\033[0m"; ./$$i; done
run: all 
	@$(call run_all,$(RUNNERS))

gdb: CXX := g++
gdb: CXXFLAGS := -std=c++98 -ggdb -Wall -Werror -Wextra -MMD
gdb: all

# Only print a message if actually removing files/folders
rm_wrapper = rm -r $(1) 2>/dev/null && echo $(2) || true
clean:
	@$(call rm_wrapper,$(BUILD_DIR),"remove $(BUILD_DIR)/")
fclean: clean
	@$(call rm_wrapper,$(RUNNERS),"remove $(RUNNERS)")

re: fclean all

-include $(DEP_FILES)

#########
# RULES #
#########
run_test_request_parser: $(OBJ_REQUEST_PARSER)
	$(CXX) $(CXXFLAGS) $^ -o $@
run_test_lib_cctype_cast: $(OBJ_LIB_CCTYPE_CAST)
	$(CXX) $(CXXFLAGS) $^ -o $@
run_test_Response: $(OBJ_RESPONSE)
	$(CXX) $(CXXFLAGS) $^ -o $@

# TESTING:
# - skip the call to read_socket() in Request::parse()
$(BUILD_DIR)/%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) -DTESTING -c $< -o $@

$(BUILD_DIR):
	mkdir $@
