#ifndef ADDR_MACROS_H
# define ADDR_MACROS_H

#define END_INSTRUCTION                                                      \
	if (cpu->nmi_pending)                                                    \
	{                                                                        \
		cpu->pending_interrupt = INT_NMI;                                    \
		cpu->nmi_pending = false;                                            \
	}                                                                        \
	if (cpu->irq_pending & !(cpu->status & FLAG_I))                          \
	{                                                                        \
		cpu->pending_interrupt = INT_IRQ;                                    \
		cpu->irq_pending = false;                                            \
	}                                                                        \
	cpu->instr_step = 0;

#define TICK_IMMEDIATE(CORE_MACRO)                                      \
	case (1):                                                           \
		cpu->tmp.data = read_byte(cpu, cpu->pc++);                       \
		CORE_MACRO                                                      \
		END_INSTRUCTION                                                 \
		break;

#define TICK_ZEROPAGE(INSTR_MACRO)                                      \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(2);

#define TICK_ZEROPAGE_X(INSTR_MACRO)                                    \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		read_byte(cpu, cpu->tmp.addr);                                   \
		cpu->tmp.addr = (cpu->tmp.addr + cpu->x) & 0xFF;                  \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(3);

#define TICK_ZEROPAGE_Y(INSTR_MACRO)                                    \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		read_byte(cpu, cpu->tmp.addr);                                   \
		cpu->tmp.addr = (cpu->tmp.addr + cpu->y) & 0xFF;                  \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(3);

#define TICK_ABSOLUTE(INSTR_MACRO)                                      \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		cpu->tmp.addr = cpu->tmp.addr | (read_byte(cpu, cpu->pc++) << 8); \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(3);

#define TICK_ABSOLUTE_X_READ(INSTR_MACRO)                               \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		cpu->tmp.addr = cpu->tmp.addr | (read_byte(cpu, cpu->pc++) << 8); \
		cpu->tmp.target = (cpu->tmp.addr & 0xFF) + cpu->x;               \
		if (cpu->tmp.target > 0xFF)                                     \
			cpu->page_crossed = true;                                   \
		cpu->tmp.addr = (cpu->tmp.addr & 0xFF00) | (cpu->tmp.target & 0xFF);\
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(3);

#define TICK_ABSOLUTE_X_WRITE(INSTR_MACRO)                              \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		cpu->tmp.addr = cpu->tmp.addr | (read_byte(cpu, cpu->pc++) << 8); \
		cpu->tmp.target = (cpu->tmp.addr & 0xFF) + cpu->x;               \
		if (cpu->tmp.target > 0xFF)                                     \
			cpu->page_crossed = true;                                   \
		cpu->tmp.addr = (cpu->tmp.addr & 0xFF00) | (cpu->tmp.target & 0xFF);\
		cpu->instr_step++;                                              \
		break;                                                          \
	case (3):                                                           \
		read_byte(cpu, cpu->tmp.addr);                                   \
		if (cpu->page_crossed)                                          \
			cpu->tmp.addr += 0x100;                                      \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(4);

#define TICK_ABSOLUTE_Y_READ(INSTR_MACRO)                               \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		cpu->tmp.addr = cpu->tmp.addr | (read_byte(cpu, cpu->pc++) << 8); \
		cpu->tmp.target = (cpu->tmp.addr & 0xFF) + cpu->y;               \
		if (cpu->tmp.target > 0xFF)                                     \
			cpu->page_crossed = true;                                   \
		cpu->tmp.addr = (cpu->tmp.addr & 0xFF00) | (cpu->tmp.target & 0xFF);\
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(3);

#define TICK_ABSOLUTE_Y_WRITE(INSTR_MACRO)                              \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		cpu->tmp.addr = cpu->tmp.addr | (read_byte(cpu, cpu->pc++) << 8); \
		cpu->tmp.target = (cpu->tmp.addr & 0xFF) + cpu->y;               \
		if (cpu->tmp.target > 0xFF)                                     \
			cpu->page_crossed = true;                                   \
		cpu->tmp.addr = (cpu->tmp.addr & 0xFF00) | (cpu->tmp.target & 0xFF);\
		cpu->instr_step++;                                              \
		break;                                                          \
	case (3):                                                           \
		read_byte(cpu, cpu->tmp.addr);                                   \
		if (cpu->page_crossed)                                          \
			cpu->tmp.addr += 0x100;                                      \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(4);

#define TICK_ZEROPAGE_X_INDIRECT(INSTR_MACRO)                           \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		read_byte(cpu, cpu->tmp.addr);                                   \
		cpu->tmp.addr = (cpu->tmp.addr + cpu->x) & 0xFF;                  \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (3):                                                           \
		cpu->tmp.target = read_byte(cpu, cpu->tmp.addr) & 0xFF;          \
		cpu->tmp.addr = (cpu->tmp.addr + 1) & 0xFF;                       \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (4):                                                           \
		cpu->tmp.target |= (read_byte(cpu, cpu->tmp.addr) << 8);         \
		cpu->tmp.addr = cpu->tmp.target;                                 \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(5);

#define TICK_ZEROPAGE_Y_INDIRECT_READ(INSTR_MACRO)                      \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		cpu->tmp.target = read_byte(cpu, cpu->tmp.addr);                 \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (3):                                                           \
		cpu->tmp.target |= (read_byte(cpu, (cpu->tmp.addr + 1) & 0xFF) << 8);\
		cpu->tmp.addr = cpu->tmp.target;                                 \
		cpu->tmp.target = (cpu->tmp.addr & 0xFF) + cpu->y;               \
		if (cpu->tmp.target > 0xFF)                                     \
			cpu->page_crossed = true;                                   \
		cpu->tmp.addr = (cpu->tmp.addr & 0xFF00) | (cpu->tmp.target & 0xFF);\
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(4);

#define TICK_ZEROPAGE_Y_INDIRECT_WRITE(INSTR_MACRO)                     \
	case (1):                                                           \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (2):                                                           \
		cpu->tmp.target = read_byte(cpu, cpu->tmp.addr);                 \
		cpu->instr_step++;                                              \
		break;                                                          \
	case (3):                                                           \
		cpu->tmp.target |= (read_byte(cpu, (cpu->tmp.addr + 1) & 0xFF) << 8);\
		cpu->tmp.addr = cpu->tmp.target;                                 \
		cpu->tmp.target = (cpu->tmp.addr & 0xFF) + cpu->y;               \
		if (cpu->tmp.target > 0xFF)                                     \
			cpu->page_crossed = true;                                   \
		cpu->tmp.addr = (cpu->tmp.addr & 0xFF00) | (cpu->tmp.target & 0xFF);\
		cpu->instr_step++;                                              \
		break;                                                          \
	case (4):                                                           \
		read_byte(cpu, cpu->tmp.addr);                                   \
		if (cpu->page_crossed)                                          \
			cpu->tmp.addr += 0x100;                                      \
		cpu->instr_step++;                                              \
		break;                                                          \
	INSTR_MACRO(5);

#endif // ADDR_MACROS_H
