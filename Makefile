CC = gcc

SRC_DIR := ./src
BUILD_DIR:= ./build
INC_DIR:= ./include

CFLAGS = -Wall -Wextra -I $(INC_DIR) -O2

DBG_FLAGS =		-g3 \
				-fsanitize=address \

SRC =	$(SRC_DIR)/main.c \
		$(SRC_DIR)/cpu.c \

OBJ = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRC:.c=.o))

NAME = emu6502

all: $(NAME)

$(NAME): $(BUILD_DIR) $(OBJ)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $(OBJ) -o $(NAME)

$(OBJ): $(BUILD_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(DBG_FLAGS) -c $^ -o $@

$(BUILD_DIR):
	@mkdir -p $@

clean:
	rm -rf build/

fclean: clean
	rm -rf $(NAME)

re: fclean all
.PHONY: all clean fclean re
