#ifndef INSTR_MACROS_H
# define INSTR_MACROS_H

#include <stdlib.h>

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


#define EXEC_LDA_CORE                                                        \
	cpu->a = cpu->tmp.data;                                                   \
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

#define EXEC_LDA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_LDA_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_LDX_CORE                                                        \
	cpu->x = cpu->tmp.data;                                                   \
	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);

#define EXEC_LDX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_LDX_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_LDY_CORE                                                        \
	cpu->y = cpu->tmp.data;                                                   \
	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);

#define EXEC_LDY(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_LDY_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_STA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		write_byte(cpu, cpu->tmp.addr, cpu->a);                               \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_STX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		write_byte(cpu, cpu->tmp.addr, cpu->x);                               \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_STY(START_STEP)                                                 \
	case (START_STEP):                                                       \
		write_byte(cpu, cpu->tmp.addr, cpu->y);                               \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_TAX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->x = cpu->a;                                                     \
		SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);                     \
		SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);                           \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_TAY(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->y = cpu->a;                                                     \
		SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);                     \
		SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);                           \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_TSX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->x = cpu->sp;                                                    \
		SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);                     \
		SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);                           \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_TXA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->a = cpu->x;                                                     \
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                     \
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);                           \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_TXS(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->sp = cpu->x;                                                    \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_TYA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->a = cpu->y;                                                     \
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                     \
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);                           \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_PUSH(START_STEP, REG_DATA)                                      \
	case (START_STEP):                                                       \
		read_byte(cpu, cpu->pc);                                             \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->sp-- | 0x100, REG_DATA);                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_PLA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		read_byte(cpu, cpu->pc);                                             \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		read_byte(cpu, cpu->sp | 0x100);                                     \
		cpu->sp++;                                                           \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		cpu->a = read_byte(cpu, cpu->sp | 0x100);                            \
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                     \
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);                           \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_PLP(START_STEP)                                                 \
	case (START_STEP):                                                       \
		read_byte(cpu, cpu->pc);                                             \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		read_byte(cpu, cpu->sp | 0x100);                                     \
		cpu->sp++;                                                           \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		cpu->tmp.data = read_byte(cpu, cpu->sp | 0x100);                      \
		cpu->status = (cpu->tmp.data | FLAG_E) & ~FLAG_B;                     \
		END_INSTRUCTION                                                      \
		break ;


#define EXEC_ASL_CORE(OPERAND)                                               \
	SET_BIT(cpu->status, FLAG_C, OPERAND & BIT_7);                           \
	OPERAND <<= 1;                                                           \
	SET_BIT(cpu->status, FLAG_N, OPERAND & SIGN_BIT);                        \
	SET_BIT(cpu->status, FLAG_Z, OPERAND == 0);


#define EXEC_ASL(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ASL_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ASL_ACCUMULATOR                                                 \
	EXEC_ASL_CORE(cpu->a)                                                    \
	END_INSTRUCTION                                                          \

#define EXEC_LSR_CORE(OPERAND)                                               \
	SET_BIT(cpu->status, FLAG_C, OPERAND & BIT_0);                           \
	OPERAND >>= 1;                                                           \
	SET_BIT(cpu->status, FLAG_N, OPERAND & SIGN_BIT);                        \
	SET_BIT(cpu->status, FLAG_Z, OPERAND == 0);

#define EXEC_LSR(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_LSR_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_LSR_ACCUMULATOR                                                 \
	EXEC_LSR_CORE(cpu->a)                                                    \
	END_INSTRUCTION                                                          \

#define EXEC_ROL_CORE(OPERAND)                                               \
	cpu->tmp.carry = (OPERAND & BIT_7);                                      \
	OPERAND <<= 1;                                                           \
	SET_BIT(OPERAND, BIT_0, cpu->status & FLAG_C);                           \
	SET_BIT(cpu->status, FLAG_C, cpu->tmp.carry);                            \
	SET_BIT(cpu->status, FLAG_N, OPERAND & SIGN_BIT);                        \
	SET_BIT(cpu->status, FLAG_Z, OPERAND == 0);

#define EXEC_ROL(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ROL_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ROL_ACCUMULATOR                                                 \
	EXEC_ROL_CORE(cpu->a)                                                    \
	END_INSTRUCTION                                                          \

#define EXEC_ROR_CORE(OPERAND)                                               \
	cpu->tmp.carry = (OPERAND & BIT_0);                                      \
	OPERAND >>= 1;                                                           \
	SET_BIT(OPERAND, BIT_7, cpu->status & FLAG_C);                           \
	SET_BIT(cpu->status, FLAG_C, cpu->tmp.carry);                            \
	SET_BIT(cpu->status, FLAG_N, OPERAND & SIGN_BIT);                        \
	SET_BIT(cpu->status, FLAG_Z, OPERAND == 0);

#define EXEC_ROR(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ROR_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ROR_ACCUMULATOR                                                 \
	EXEC_ROR_CORE(cpu->a)                                                    \
	END_INSTRUCTION                                                          \

#define EXEC_AND_CORE                                                        \
	cpu->a &= cpu->tmp.data;                                                  \
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

#define EXEC_AND(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_AND_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_BIT_CORE                                                        \
	SET_BIT(cpu->status, FLAG_N, cpu->tmp.data & SIGN_BIT);                   \
	SET_BIT(cpu->status, FLAG_V, cpu->tmp.data & BIT_6);                      \
	cpu->tmp.data &= cpu->a;                                                  \
	SET_BIT(cpu->status, FLAG_Z, cpu->tmp.data == 0);

#define EXEC_BIT(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_BIT_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_EOR_CORE                                                        \
	cpu->a ^= cpu->tmp.data;                                                  \
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

#define EXEC_EOR(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_EOR_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ORA_CORE                                                        \
	cpu->a |= cpu->tmp.data;                                                  \
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

#define EXEC_ORA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_ORA_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ADC_CORE                                                        \
	cpu->tmp.result = cpu->a + cpu->tmp.data + (cpu->status & FLAG_C);        \
	cpu->tmp.overflow = ~(cpu->tmp.data ^ cpu->a) & (cpu->tmp.result ^ cpu->a) & SIGN_BIT;\
	SET_BIT(cpu->status, FLAG_V, cpu->tmp.overflow);                         \
	SET_BIT(cpu->status, FLAG_N, cpu->tmp.result & SIGN_BIT);                \
	SET_BIT(cpu->status, FLAG_Z, (cpu->tmp.result & 0xFF) == 0);             \
	SET_BIT(cpu->status, FLAG_C, cpu->tmp.result > 0xFF);                    \
	cpu->a = (uint8_t)cpu->tmp.result;

#define EXEC_ADC(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_ADC_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ADC_DECIMAL_CORE                                                \
	cpu->tmp.result = cpu->a + cpu->tmp.data + (cpu->status & FLAG_C);        \
	cpu->tmp.overflow = ~(cpu->tmp.data ^ cpu->a) & (cpu->tmp.result ^ cpu->a) & SIGN_BIT;\
	SET_BIT(cpu->status, FLAG_V, cpu->tmp.overflow);                         \
	SET_BIT(cpu->status, FLAG_N, cpu->tmp.result & SIGN_BIT);                \
	SET_BIT(cpu->status, FLAG_Z, (cpu->tmp.result & 0xFF) == 0);             \
	if (cpu->status & FLAG_D)                                                \
	{                                                                        \
		if ((cpu->tmp.result & 0x0F) > 9 || ((cpu->a ^ cpu->tmp.data ^ cpu->tmp.result) & 0x10))\
			cpu->tmp.result += 0x06;                                         \
		SET_BIT(cpu->status, FLAG_C, cpu->tmp.result > 0x9F || (cpu->tmp.result & 0x100));\
		if (cpu->status & FLAG_C)                                            \
			cpu->tmp.result += 0x60;                                         \
	}                                                                        \
	SET_BIT(cpu->status, FLAG_C, cpu->tmp.result > 0xFF);                    \
	cpu->a = (uint8_t)cpu->tmp.result;

#define EXEC_ADC_DECIMAL(START_STEP)                                         \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_ADC_DECIMAL_CORE                                                \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_CMP_CORE(REGISTER)                                              \
	SET_BIT(cpu->status, FLAG_C, cpu->tmp.data <= REGISTER);                  \
	SET_BIT(cpu->status, FLAG_Z, cpu->tmp.data == REGISTER);                  \
	cpu->tmp.data = REGISTER - cpu->tmp.data;                                  \
	SET_BIT(cpu->status, FLAG_N, cpu->tmp.data & SIGN_BIT);

#define EXEC_CMP(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_CMP_CORE(cpu->a)                                                \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_CPX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_CMP_CORE(cpu->x)                                                \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_CPY(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_CMP_CORE(cpu->y)                                                \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_SBC_CORE                                                        \
	cpu->tmp.data = ~cpu->tmp.data;                                            \
	cpu->tmp.result = cpu->a + cpu->tmp.data + (cpu->status & FLAG_C);        \
	cpu->tmp.overflow = ~(cpu->tmp.data ^ cpu->a) & (cpu->tmp.result ^ cpu->a) & SIGN_BIT;\
	SET_BIT(cpu->status, FLAG_V, cpu->tmp.overflow);                         \
	SET_BIT(cpu->status, FLAG_N, cpu->tmp.result & SIGN_BIT);                \
	SET_BIT(cpu->status, FLAG_Z, (cpu->tmp.result & 0xFF) == 0);             \
	SET_BIT(cpu->status, FLAG_C, cpu->tmp.result > 0xFF);                    \
	cpu->a = (uint8_t)cpu->tmp.result;

#define EXEC_SBC(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_SBC_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_SBC_DECIMAL_CORE                                                \
	cpu->tmp.unflipped = cpu->tmp.data;                                       \
	cpu->tmp.data = ~cpu->tmp.data;                                            \
	cpu->tmp.result = cpu->a + cpu->tmp.data + (cpu->status & FLAG_C);        \
	cpu->tmp.overflow = ~(cpu->tmp.data ^ cpu->a) & (cpu->tmp.result ^ cpu->a) & SIGN_BIT;\
	SET_BIT(cpu->status, FLAG_V, cpu->tmp.overflow);                         \
	SET_BIT(cpu->status, FLAG_N, cpu->tmp.result & SIGN_BIT);                \
	SET_BIT(cpu->status, FLAG_Z, (cpu->tmp.result & 0xFF) == 0);             \
	if (cpu->status & FLAG_D)                                                \
	{                                                                        \
		if ((cpu->a ^ cpu->tmp.unflipped ^ cpu->tmp.result) & 0x10)          \
			cpu->tmp.result -= 0x06;                                         \
		if (cpu->tmp.result < 0x100)                                         \
			cpu->tmp.result -= 0x60;                                         \
	}                                                                        \
	SET_BIT(cpu->status, FLAG_C, cpu->tmp.result > 0xFF);                    \
	cpu->a = (uint8_t)cpu->tmp.result;

#define EXEC_SBC_DECIMAL(START_STEP)                                         \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_SBC_DECIMAL_CORE                                                \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_DEC_CORE(OPERAND)                                               \
	OPERAND--;                                                               \
	SET_BIT(cpu->status, FLAG_N, OPERAND & SIGN_BIT);                        \
	SET_BIT(cpu->status, FLAG_Z, OPERAND == 0);

#define EXEC_DEC(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_DEC_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_DEX                                                             \
	EXEC_DEC_CORE(cpu->x)                                                    \
	END_INSTRUCTION

#define EXEC_DEY                                                             \
	EXEC_DEC_CORE(cpu->y)                                                    \
	END_INSTRUCTION

#define EXEC_INC_CORE(OPERAND)                                               \
	OPERAND++;                                                               \
	SET_BIT(cpu->status, FLAG_N, OPERAND & SIGN_BIT);                        \
	SET_BIT(cpu->status, FLAG_Z, OPERAND == 0);

#define EXEC_INC(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_INC_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_INX                                                             \
	EXEC_INC_CORE(cpu->x);                                                   \
	END_INSTRUCTION

#define EXEC_INY                                                             \
	EXEC_INC_CORE(cpu->y);                                                   \
	END_INSTRUCTION

#define EXEC_BRK(START_STEP)                                                 \
	case (START_STEP):                                                       \
		read_byte(cpu, cpu->pc);                                             \
		if (cpu->pending_interrupt == INT_BRK)                               \
			cpu->pc++;                                                       \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->sp-- | 0x100, (cpu->pc >> 8) & 0xFF);           \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->sp-- | 0x100, cpu->pc & 0xFF);                  \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 3):                                                   \
		if (cpu->pending_interrupt == INT_BRK)                               \
			write_byte(cpu, cpu->sp-- | 0x100, cpu->status | FLAG_E | FLAG_B);\
		else                                                                 \
			write_byte(cpu, cpu->sp-- | 0x100, (cpu->status | FLAG_E) & ~FLAG_B);\
		cpu->status |= FLAG_I;                                               \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 4):                                                   \
		cpu->tmp.addr = read_byte(cpu, cpu->interrupt_vector);                \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 5):                                                   \
		cpu->tmp.addr |= (read_byte(cpu, cpu->interrupt_vector + 1) << 8);    \
		cpu->pc = cpu->tmp.addr;                                              \
		cpu->pending_interrupt = INT_NONE;                                   \
		cpu->instr_step = 0;                                                 \
		break ;

#define EXEC_JMP(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                     \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 1):                                                   \
		cpu->tmp.addr |= (read_byte(cpu, cpu->pc++) << 8);                    \
		cpu->pc = cpu->tmp.addr;                                              \
		END_INSTRUCTION                                                      \
		break;

#define EXEC_JMP_INDIRECT(START_STEP)                                        \
	case (START_STEP):                                                       \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                     \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 1):                                                   \
		cpu->tmp.addr |= (read_byte(cpu, cpu->pc++) << 8);                    \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		cpu->pc = read_byte(cpu, cpu->tmp.addr) & 0xFF;                       \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 3):                                                   \
		if ((cpu->tmp.addr & 0xFF) == 0xFF)                                   \
			cpu->pc |= read_byte(cpu, cpu->tmp.addr & 0xFF00) << 8;           \
		else                                                                 \
			cpu->pc |= read_byte(cpu, cpu->tmp.addr + 1) << 8;                \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_JSR(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.addr = read_byte(cpu, cpu->pc++) & 0xFF;                     \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 1):                                                   \
		read_byte(cpu, cpu->sp | 0x100);                                     \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->sp-- | 0x100, (cpu->pc >> 8) & 0xFF);           \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 3):                                                   \
		write_byte(cpu, cpu->sp-- | 0x100, cpu->pc & 0xFF);                  \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 4):                                                   \
		cpu->tmp.addr |= (read_byte(cpu, cpu->pc) << 8);                      \
		cpu->pc = cpu->tmp.addr;                                              \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_RTI(START_STEP)                                                 \
	case (START_STEP):                                                       \
		read_byte(cpu, cpu->pc) & 0xFF;                                      \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 1):                                                   \
		read_byte(cpu, cpu->sp++ | 0x100);                                   \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 2):                                                   \
		cpu->status = (read_byte(cpu, cpu->sp++ | 0x100) | FLAG_E) & ~FLAG_B;\
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 3):                                                   \
		cpu->tmp.addr = read_byte(cpu, cpu->sp++ | 0x100);                    \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 4):                                                   \
		cpu->tmp.addr |= (read_byte(cpu, cpu->sp | 0x100) << 8);              \
		cpu->pc = cpu->tmp.addr;                                              \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_RTS(START_STEP)                                                 \
	case (START_STEP):                                                       \
		read_byte(cpu, cpu->pc) & 0xFF;                                      \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 1):                                                   \
		read_byte(cpu, cpu->sp++ | 0x100);                                   \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 2):                                                   \
		cpu->tmp.addr = read_byte(cpu, cpu->sp++ | 0x100);                    \
		cpu->instr_step++;                                                   \
		break;                                                               \
	case (START_STEP + 3):                                                   \
		cpu->tmp.addr |= (read_byte(cpu, cpu->sp | 0x100) << 8);              \
		cpu->pc = cpu->tmp.addr;                                              \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 4):                                                   \
		read_byte(cpu, cpu->pc);                                             \
		cpu->pc++;                                                           \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_BRANCH_CORE(CONDITION)                                          \
	case (1):                                                                \
		cpu->tmp.rel = read_byte(cpu, cpu->pc++);                            \
		if (CONDITION)                                                       \
			cpu->instr_step++;                                               \
		else                                                                 \
		{                                                                    \
			END_INSTRUCTION                                                  \
		}                                                                    \
		break ;                                                              \
	case (2):                                                                \
		read_byte(cpu, cpu->pc);                                             \
		cpu->tmp.result = cpu->pc + cpu->tmp.rel;                            \
		if ((cpu->pc & 0xFF00) != (cpu->tmp.result & 0xFF00))                \
		{                                                                    \
			cpu->instr_step++;                                               \
			cpu->tmp.page_crossed_down = (cpu->pc & 0xFF00) > (cpu->tmp.result & 0xFF00);\
		}                                                                    \
		else                                                                 \
		{                                                                    \
			END_INSTRUCTION                                                  \
		}                                                                    \
		cpu->pc = (cpu->pc & 0xFF00) | (cpu->tmp.result & 0xFF);             \
		break;                                                               \
	case (3):                                                                \
		read_byte(cpu, cpu->pc);                                             \
		cpu->pc = (cpu->tmp.page_crossed_down) ? cpu->pc - 0x100 : cpu->pc + 0x100;\
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_CLC                                                             \
	cpu->status &= ~FLAG_C;                                                  \
	END_INSTRUCTION

#define EXEC_CLD                                                             \
	cpu->status &= ~FLAG_D;                                                  \
	END_INSTRUCTION

#define EXEC_CLI                                                             \
	cpu->status &= ~FLAG_I;                                                  \
	END_INSTRUCTION

#define EXEC_CLV                                                             \
	cpu->status &= ~FLAG_V;                                                  \
	END_INSTRUCTION

#define EXEC_SEC                                                             \
	cpu->status |= FLAG_C;                                                   \
	END_INSTRUCTION

#define EXEC_SED                                                             \
	cpu->status |= FLAG_D;                                                   \
	END_INSTRUCTION

#define EXEC_SEI                                                             \
	cpu->status |= FLAG_I;                                                   \
	END_INSTRUCTION

#define EXEC_NOP_CORE                                                        \
	read_byte(cpu, cpu->tmp.addr);

#define EXEC_NOP                                                             \
	read_byte(cpu, cpu->pc);                                                 \
	END_INSTRUCTION

#define EXEC_HLT(START_STEP)                                                 \
	case (START_STEP):                                                       \
		exit(1);                                                             \
		break ;

#define EXEC_SKB(START_STEP)                                                 \
	case (START_STEP):                                                       \
		EXEC_NOP_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_SKW(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		EXEC_NOP_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_SLO(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ASL_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ORA_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_RLA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ROL_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_AND_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_SRE(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_LSR_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_EOR_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_RRA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ROR_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_ADC_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_SAX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		write_byte(cpu, cpu->tmp.addr, cpu->a & cpu->x);                      \
		END_INSTRUCTION                                                      \
		break ;

// TODO: Unstable opcode
#define EXEC_SHA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		break ;

// TODO: Unstable opcode
#define EXEC_SHX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		break ;

// TODO: Unstable opcode
#define EXEC_SHY(START_STEP)                                                 \
	case (START_STEP):                                                       \
		break ;

#define EXEC_LAX_CORE                                                        \
	cpu->a = cpu->tmp.data;                                                   \
	cpu->x = cpu->tmp.data;                                                   \
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

#define EXEC_LAX(START_STEP)                                                 \
	case (START_STEP):                                                       \
		if (cpu->page_crossed)                                               \
		{                                                                    \
			read_byte(cpu, cpu->tmp.addr);                                    \
			cpu->tmp.addr += 0x100;                                           \
			cpu->page_crossed = false;                                       \
			break ;                                                          \
		}                                                                    \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		EXEC_LAX_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_DCP(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_DEC_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_CMP_CORE(cpu->a)                                                \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ARR_CORE                                                        \
	cpu->a &= cpu->tmp.data;                                                  \
	cpu->a >>= 1;                                                            \
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);                         \
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);                               \
	SET_BIT(cpu->status, FLAG_C, cpu->a & BIT_6);                            \
	SET_BIT(cpu->status, FLAG_V, ((cpu->a >> 6) ^ (cpu->a >> 5)) & 1);
	

// TODO: Unstable opcode
#define EXEC_XAA(START_STEP)                                                 \
	case (START_STEP):                                                       \
		break ;

// TODO: Unstable opcode
#define EXEC_SHS(START_STEP)                                                 \
	case (START_STEP):                                                       \
		break ;

// TODO: Unstable opcode
#define EXEC_OAL(START_STEP)                                                 \
	case (START_STEP):                                                       \
		break ;

// TODO: Unstable opcode
#define EXEC_LAS(START_STEP)                                                 \
	case (START_STEP):                                                       \
		break ;

#define EXEC_SBX_CORE                                                        \
	cpu->x &= cpu->a;                                                        \
	EXEC_CMP_CORE(cpu->x);                                                   \
	cpu->x = cpu->tmp.data;

#define EXEC_ISC(START_STEP)                                                 \
	case (START_STEP):                                                       \
		cpu->tmp.data = read_byte(cpu, cpu->tmp.addr);                         \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 1):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_INC_CORE(cpu->tmp.data)                                          \
		cpu->instr_step++;                                                   \
		break ;                                                              \
	case (START_STEP + 2):                                                   \
		write_byte(cpu, cpu->tmp.addr, cpu->tmp.data);                         \
		EXEC_SBC_CORE                                                        \
		END_INSTRUCTION                                                      \
		break ;

#define EXEC_ASR_CORE                                                        \
	cpu->a &= cpu->tmp.data;                                                  \
	EXEC_LSR_CORE(cpu->a);

#define EXEC_ANC_CORE                                                        \
	EXEC_AND_CORE                                                            \
	SET_BIT(cpu->status, FLAG_C, cpu->a & BIT_7);

#endif // INSTR_MACROS_H
