###########
# ALIASES #
###########
MAKE := make
CXX := c++

#########
# FLAGS #
#########
STD_FLAGS :=	-std=c++98
# -Wconversion and -Wsign-conversion check for implicit conversion
# that may result in data loss (bigger to smaller and signed to unsigned).
WARNINGS :=		-Wall -Werror -Wextra -Wconversion -Wsign-conversion
CXXFLAGS :=		$(STD_FLAGS) $(WARNINGS)
CXXFLAGS_DB :=	$(STD_FLAGS) $(WARNINGS) -g3
CXXFLAGS_NE :=	$(STD_FLAGS) -Wall -Wextra -Wconversion -Wsign-conversion -g3
CXXFLAGS_AS :=	$(STD_FLAGS) $(WARNINGS) -g3 \
				-fsanitize=address
CXXFLAGS_US :=	$(STD_FLAGS) $(WARNINGS) -g3 \
				-fsanitize=undefined
DEP_FLAGS := 	-MMD

#########
# FILES #
#########
NAME := webserv

SRC_DIR := src
LIB_DIR := src/lib
vpath %.cpp $(SRC_DIR):$(LIB_DIR)
SRC := request_parser.cpp \
	   cctype_cast.cpp \
	   c_network_exception_wrapper.cpp \
	   ActiveRequests.cpp \
	   ActiveResponses.cpp \
	   EpollEvents.cpp \
	   main_loop.cpp \
	   main.cpp \
	   Sockets.cpp \

BUILD_DIR := build
OBJ := $(SRC:%.cpp=$(BUILD_DIR)/%.o)

DEP_FILES := $(OBJ:.o=.d)

###########
# TARGETS #
###########
.PHONY: all clean fclean re dbg noerr test asan usan

all: $(BUILD_DIR) $(NAME)

dbg: CXXFLAGS = $(CXXFLAGS_DB)
dbg: all
noerr: CXXFLAGS = $(CXXFLAGS_NE)
noerr: all

test:
	./test/test_runner.sh

asan: CXXFLAGS = $(CXXFLAGS_AS)
asan: CXX = clang++
asan: all
usan: CXXFLAGS = $(CXXFLAGS_US)
usan: CXX = clang++
usan: all

# Only print a message if actually removing files/folders
rm_wrapper = rm -r $(1) 2>/dev/null && echo $(2) || true
clean:
	@$(call rm_wrapper,$(BUILD_DIR),"remove $(BUILD_DIR)/")
fclean: clean
	@$(call rm_wrapper,$(NAME),"remove $(NAME)")

re: fclean all

-include $(DEP_FILES)

#########
# RULES #
#########
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(DEP_FLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir $@
