#include "emu6502.h"
#include <ctype.h>
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
	uint8_t *buf = malloc(size);
	read(fd, buf, size);
	memcpy(&cpu->memory[0x8000], &buf[16], 0x4000);
	memcpy(&cpu->memory[0xC000], &buf[16], 0x4000);
	// read(fd, &cpu->memory[10], size);
	free(buf);
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

void	run_until_breakpoint(t_cpu *cpu, BSTSet_word *breakpoints)
{
	uint16_t old_pc = 0xFFFF;
	while (cpu->pc != old_pc)
	{
		old_pc = cpu->pc;
		uint8_t opcode = read_byte(cpu, cpu->pc);
		const t_instruct *instr = get_instruction(opcode);
		execute_instr(cpu, instr);
		if (bstset_word_contains(breakpoints, cpu->pc))
			break ;
	}
}

int	main(int argc, char **argv)
{
	t_cpu	cpu = {0};
	uint8_t	mem[0x10000];

	cpu.memory = mem;
	cpu.memsize = 0x10000;
	cpu.sp = 0xFF;

	printf("mem: %ld\n", sizeof(mem));

	if (argc > 2)
		return 1;

	load_program(&cpu, argv[1]);
	// uint16_t reset_vec = read_word(&cpu, 0xFFFC);
	// printf("Reset vector: %04X\n", reset_vec);
	// getchar();
	cpu.pc = 0xC000;


	set_term_settings();
	BSTSet_word breakpoints;
	// bstset_word_insert(&breakpoints, 0x3373);
	// run_until_addr(&cpu, 0x3373);
	while (true)
	{
		uint16_t orig_pc = cpu.pc;
		char c = 0;
		while (c != '\n' && c != 'n')
		{
			printf("\e[2J\e[H\e[32;1m<<< FETCH <<<\e[m\n");
			printf("-------------\n");
			print_debug_view(&cpu, orig_pc);
			c = tolower(getchar());
			switch (c) {
				case ('y'):
					cpu.y = read_byte_input("Y = ");
					break;
				case ('x'):
					cpu.x = read_byte_input("X = ");
					break;
				case ('a'):
					cpu.a = read_byte_input("A = ");
					break;
				// case ('p'):
				// 	cpu.pc = read_byte_input("PC = ");
				// 	break;
				case ('s'):
					cpu.sp = read_byte_input("SP = ");
					break;
				case ('b'):
					bstset_word_insert(&breakpoints, read_word_input("\e[31;1mBREAK\e[m: "));
					// _bstset_word_print(breakpoints.tree, 7);
					// printf("contains 0x3373: %d\n", bstset_word_contains(&breakpoints, 0x3373));
					getchar();
					break;
				case ('c'):
					run_until_breakpoint(&cpu, &breakpoints);
					orig_pc = cpu.pc;
					continue ;
				case ('q'):
					goto END;
				default:
					break;
			}
		}

		uint8_t opcode = read_byte(&cpu, cpu.pc);
		const t_instruct *instr = get_instruction(opcode);
		execute_instr(&cpu, instr);
		printf("\e[2J\e[H\e[31;1m>>> EXECUTE >>>\e[m\n");
		printf("-------------\n");
		print_debug_view(&cpu, orig_pc);
		getchar();
		// cpu.pc += instr->n_bytes;
		// if (cpu.pc == orig_pc)
		// 	break;
	}
	// printf("PC: %04X\n", cpu.pc);
END:
	bstset_word_clear(&breakpoints, NULL);
	reset_term_settings();
}
