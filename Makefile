CC = gcc

SRC_DIR := ./src
BUILD_DIR:= ./build
INC_DIR:= ./include

CFLAGS = -Wall -Wextra -I $(INC_DIR) -O0

LINK_FLAGS = -lraylib

DBG_FLAGS =		-g3 \
				# -fsanitize=address,undefined \
				# -pg \

SRC =	$(SRC_DIR)/main.c \
		$(SRC_DIR)/emulator.c \
		$(SRC_DIR)/6502/cpu.c \
		$(SRC_DIR)/6502/instructions.c \
		$(SRC_DIR)/6502/display.c \
		$(SRC_DIR)/nes/cart.c \
		$(SRC_DIR)/nes/mappers.c \
		$(SRC_DIR)/nes/apu.c \
		$(SRC_DIR)/nes/ppu.c \
		$(SRC_DIR)/nes/nes.c \
		$(SRC_DIR)/nes/render.c \
		$(SRC_DIR)/nes/input.c \
		$(SRC_DIR)/nes/saves.c \
		$(SRC_DIR)/nes/audio_stream.c \
		$(SRC_DIR)/nes/menu.c \
		$(SRC_DIR)/6502/instructions_alt.c \
		# $(SRC_DIR)/6502/dump.c \

OBJ = $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRC:.c=.o))

NAME = emu6502

all: $(NAME)

$(NAME): $(BUILD_DIR) $(OBJ)
	$(CC) $(CFLAGS) $(DBG_FLAGS) $(LINK_FLAGS) $(OBJ) -o $(NAME)

$(OBJ): $(BUILD_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $(CFLAGS) $(DBG_FLAGS) -c $^ -o $@

$(BUILD_DIR):
	@mkdir -p $@/6502
	@mkdir -p $@/nes

clean:
	rm -rf build/

fclean: clean
	rm -rf $(NAME)

re: fclean all
.PHONY: all clean fclean re
