#include "emu6502.h"
#include "nes.h"
#include <ctype.h>
#include <raylib.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

FS_DECLARE_BSTSET(uint16_t, word, NULL);


sig_atomic_t g_var = {0x00};

void sig_handler(int signo) {
	if (signo == SIGINT)
		g_var = SIGINT;
}

void	set_term_settings(void)
{
	struct termios term;

	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag &= ~(ICANON);
	term.c_lflag &= ~ECHO;
	tcsetattr(fileno(stdin), TCSANOW, &term);
}

void	enable_echo(void)
{
	struct termios term;

	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag |= ECHO;
	tcsetattr(fileno(stdin), TCSANOW, &term);
}

void	disable_echo(void)
{
	struct termios term;

	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag &= ~ECHO;
	tcsetattr(fileno(stdin), TCSANOW, &term);
}

void	reset_term_settings(void)
{
	struct termios term;

	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag |= ICANON;
	tcsetattr(fileno(stdin), TCSANOW, &term);
}

void	load_program(t_cpu *cpu, const char *path)
{
	struct stat statbuf;

	stat(path, &statbuf);
	size_t size = statbuf.st_size;
	if (size > 0x10000)
		return ;

	int fd = open(path, O_RDONLY);
	// uint8_t *buf = malloc(size);
	// read(fd, buf, size);
	// memcpy(&cpu->memory[0x8000], &buf[16], 0x4000);
	// memcpy(&cpu->memory[0xC000], &buf[16], 0x4000);
	// free(buf);
	read(fd, cpu->memory, size);
	// read(fd, &cpu->memory[10], size);
	close(fd);
}

uint8_t read_byte_input(char *prompt)
{
	char buf[3] = {};
	int i = 0;
	printf("%s", prompt);
	fflush(stdout);

	enable_echo();
	while (i < 2)
	{
		char c = tolower(getchar());
		if (!strchr("0123456789abcdef", c))
			continue ;
		buf[i++] = c;
	}
	disable_echo();
	return (uint8_t)strtol(buf, NULL, 16);
}

uint16_t read_word_input(char *prompt)
{
	char buf[5] = {};

	int i = 0;
	printf("%s", prompt);
	fflush(stdout);

	enable_echo();
	while (i < 4)
	{
		char c = tolower(getchar());
		if (!strchr("0123456789abcdef", c))
			continue ;
		buf[i++] = c;
	}
	// printf("\n%s\n", buf);
	disable_echo();
	long num = strtol(buf, NULL, 16);
	// printf("%04zX\n", num);
	uint16_t ret = (uint16_t)num;
	// printf("%04X\n", ret);
	return ret;
}

void	run_until_addr(t_cpu *cpu, uint16_t addr)
{
	while (cpu->pc != addr)
	{
		uint16_t old_pc = cpu->pc;
		uint8_t opcode = read_byte(cpu, cpu->pc);
		const t_instruct *instr = get_instruction(opcode);
		execute_instr(cpu, instr);
		if (cpu->pc == old_pc)
			break ;
	}
}

static inline void	run_until_breakpoint(t_nes *nes, BSTSet_word *breakpoints)
{
	uint16_t old_pc = 0xFFFF;
	g_var = 0;
	// while (nes->cpu.pc != old_pc && g_var != SIGINT)
	while (g_var != SIGINT && !WindowShouldClose())
	{
		old_pc = nes->cpu.pc;
		// if (instr->instruction > NOP)
		// 	break ;
		nes_step(nes);
		// printf("\e[2J\e[H");
		// print_debug_view(cpu, old_pc);
		if (bstset_word_contains(breakpoints, nes->cpu.pc))
			break ;
	}
	g_var = 0;
}

void	debug_loop(t_nes *nes)
{
	uint16_t addrbuf;
	BSTSet_word breakpoints = {};
	signal(SIGINT, sig_handler);
	while (!WindowShouldClose())
	{
		uint16_t orig_pc = nes->cpu.pc;
		char c = 0;
		while (c != '\n' && c != 'n')
		{
			printf("\e[2J\e[H\e[32;1m<<< FETCH <<<\e[m\n");
			printf("-------------\n");
			print_debug_view(&nes->cpu, orig_pc);
			c = tolower(getchar());
			switch (c) {
				case ('y'):
					nes->cpu.y = read_byte_input("Y = ");
					break;
				case ('x'):
					nes->cpu.x = read_byte_input("X = ");
					break;
				case ('a'):
					nes->cpu.a = read_byte_input("A = ");
					break;
				// case ('p'):
				// 	nes.cpu.pc = read_byte_input("PC = ");
				// 	break;
				case ('s'):
					nes->cpu.sp = read_byte_input("SP = ");
					break;
				case ('b'):
					bstset_word_insert(&breakpoints, read_word_input("\e[31;1mBREAK\e[m: "));
					// _bstset_word_print(breakpoints.tree, 7);
					// printf("contains 0x3373: %d\n", bstset_word_contains(&breakpoints, 0x3373));
					// getchar();
					break;
				case ('p'):
					addrbuf = read_word_input("\e[32;1mPRINT\e[m: ");
					printf("\n\e[32:1m%04X\e[m: 0x%02X\n", addrbuf, read_byte(&nes->cpu, addrbuf));
					printf("Press any key to continue...\n");
					getchar();
					continue ;
				case ('c'):
					run_until_breakpoint(nes, &breakpoints);
					orig_pc = nes->cpu.pc;
					continue ;
				case ('q'):
					bstset_word_clear(&breakpoints, NULL);
					return;
				default:
					break;
			}
		}

		nes_step(nes);
		printf("\e[2J\e[H\e[31;1m>>> EXECUTE >>>\e[m\n");
		printf("-------------\n");
		print_debug_view(&nes->cpu, orig_pc);
		getchar();
		// nes.cpu.pc += instr->n_bytes;
		// if (nes.cpu.pc == orig_pc)
		// 	break;
	}
	printf("PC: %04X\n", nes->cpu.pc);

}

int	main(int argc, char **argv)
{
	// draw_palette();
	// test_tile_fetch();
	// exit(0);
	t_nes nes = {};
	uint8_t	mem[0x10000];
	nes.cpu.logfd = open("output.log", O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);

	nes.cpu.memory = mem;
	nes.cpu.memsize = 0x10000;
	nes.cpu.sp = 0xFF;

	printf("mem: %ld\n", sizeof(mem));

	if (argc > 2)
		return 1;

	// load_program(&nes.cpu, argv[1]);
	// setup_default_pagetable(&nes.cpu);
	// t_cart *cart = read_nes(argv[1]);
	// printf("cart: %p\n", cart);
	// exit(1);
	// nes.cpu.pc = 0x400;

	init_nes(&nes);
	init_raylib(&nes);
	t_cart *cart = read_nes_cart(argv[1]);
	if (cart == NULL)
		return (printf("Error loading cartridge\n"), 1);
	nes_load_cartridge(&nes, cart);
	// draw_tile(&nes.ppu, 0, 0, 0);
	// draw_pattern_table(&nes.ppu, 0, 0, 0);
	// draw_pattern_table(&nes.ppu, 1, 128, 0);
	// UpdateTexture(nes.ppu.screen_tex, nes.ppu.screenbuf);
	// BeginDrawing();
	// DrawTextureEx(nes.ppu.screen_tex, (Vector2){0, 0}, 0, SCALING, WHITE);
	// EndDrawing();

	set_term_settings();
	// bstset_word_insert(&breakpoints, 0x3373);
	
	nes.cpu.status = FLAG_E;
	nes.cpu.sp = 0xFF;
	nes.cpu.cycles = 0;
	exec_hardware_interrupt(&nes.cpu, 0xFFFC);
	while (!WindowShouldClose())
		nes_step(&nes);
	// debug_loop(&nes);
END:
	free_cart(nes.cart);
	free(nes.ppu.screenbuf);
	reset_term_settings();
	close(nes.cpu.logfd);
	printf("cycles: %ld\n", nes.cpu.cycles);
	CloseWindow();
}
