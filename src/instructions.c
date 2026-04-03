#include "emu6502.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>


// void	handle_neg_zero_flags(t_cpu *cpu, uint8_t op)
// {
// 	cpu->status &= ~FLAG_Z;
// 	if (op == 0)
// 		cpu->status |= FLAG_Z;
//
// 	cpu->status &= ~FLAG_N;
// 	if ((op >> 7) & 1)
// 		cpu->status |= FLAG_N;
// }

void exec_LDA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a = get_operand(cpu, instr->addrmode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_LDX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x = get_operand(cpu, instr->addrmode);

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
	cpu->pc += instr->n_bytes;
}

void exec_LDY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->y = get_operand(cpu, instr->addrmode);

	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
	cpu->pc += instr->n_bytes;
}

void exec_STA(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	write_byte(cpu, cpu->addrbus, cpu->a);
	cpu->pc += instr->n_bytes;
}

void exec_STX(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	write_byte(cpu, cpu->addrbus, cpu->x);
	cpu->pc += instr->n_bytes;
}

void exec_STY(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	write_byte(cpu, cpu->addrbus, cpu->y);
	cpu->pc += instr->n_bytes;
}

void exec_TAX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x = cpu->a;

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
	cpu->pc += instr->n_bytes;
}

void exec_TAY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->y = cpu->a;

	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
	cpu->pc += instr->n_bytes;
}

void exec_TSX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x = cpu->sp;

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
	cpu->pc += instr->n_bytes;
}

void exec_TXA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a = cpu->x;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_TXS(t_cpu *cpu, const t_instruct *instr)
{
	cpu->sp = cpu->x;
	cpu->pc += instr->n_bytes;
}

void exec_TYA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a = cpu->y;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_PHA(t_cpu *cpu, const t_instruct *instr)
{
	push_stack(cpu, cpu->a);
	cpu->pc += instr->n_bytes;
}

void exec_PHP(t_cpu *cpu, const t_instruct *instr)
{
	push_stack(cpu, cpu->status);
	cpu->pc += instr->n_bytes;
}

void exec_PLA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a = pop_stack(cpu);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_PLP(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status = pop_stack(cpu);
	cpu->pc += instr->n_bytes;
}

void exec_ASL(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_7);
		SET_BIT(cpu->status, FLAG_C, to_carry);

		cpu->a <<= 1;
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		cpu->pc += instr->n_bytes;
		return ;
	}

	uint8_t val = get_operand(cpu, instr->addrmode);
	bool to_carry = (val & BIT_7);
	SET_BIT(cpu->status, FLAG_C, to_carry);

	val <<= 1;
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);
	write_byte(cpu, cpu->addrbus, val);
	cpu->pc += instr->n_bytes;
}

void exec_LSR(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_0);
		SET_BIT(cpu->status, FLAG_C, to_carry);

		cpu->a >>= 1;
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		cpu->pc += instr->n_bytes;
		return ;
	}

	uint8_t val = get_operand(cpu, instr->addrmode);
	bool to_carry = (val & BIT_0);
	SET_BIT(cpu->status, FLAG_C, to_carry);

	val >>= 1;
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);

	write_byte(cpu, cpu->addrbus, val);
	cpu->pc += instr->n_bytes;
}

void exec_ROL(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_7);
		cpu->a <<= 1;
		SET_BIT(cpu->a, BIT_0, cpu->status & FLAG_C);

		SET_BIT(cpu->status, FLAG_C, to_carry);
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		cpu->pc += instr->n_bytes;
		return ;
	}

	uint8_t val = get_operand(cpu, instr->addrmode);

	bool to_carry = (val & BIT_7);
	val <<= 1;
	SET_BIT(val, BIT_0, cpu->status & FLAG_C);

	SET_BIT(cpu->status, FLAG_C, to_carry);
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);

	write_byte(cpu, cpu->addrbus, val);
	cpu->pc += instr->n_bytes;
}

void exec_ROR(t_cpu *cpu, const t_instruct *instr)
{
	if (instr->addrmode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_0);
		cpu->a >>= 1;
		SET_BIT(cpu->a, BIT_7, cpu->status & FLAG_C);

		SET_BIT(cpu->status, FLAG_C, to_carry);
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		cpu->pc += instr->n_bytes;
		return ;
	}

	uint8_t val = get_operand(cpu, instr->addrmode);

	bool to_carry = (val & BIT_0);
	val >>= 1;
	SET_BIT(val, BIT_7, cpu->status & FLAG_C);

	SET_BIT(cpu->status, FLAG_C, to_carry);
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);

	write_byte(cpu, cpu->addrbus, val);
	cpu->pc += instr->n_bytes;
}

void exec_AND(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a &= get_operand(cpu, instr->addrmode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_BIT(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t result = cpu->a & get_operand(cpu, instr->addrmode);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, result == 0);
	SET_BIT(cpu->status, FLAG_V, result & BIT_6);
	cpu->pc += instr->n_bytes;
}

void exec_EOR(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a ^= get_operand(cpu, instr->addrmode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_ORA(t_cpu *cpu, const t_instruct *instr)
{
	cpu->a |= get_operand(cpu, instr->addrmode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_ADC(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = get_operand(cpu, instr->addrmode);
	uint16_t sum = cpu->a + op + (cpu->status & BIT_0);
	bool carry = sum > 0xFF;
	bool overflow = ~(op ^ cpu->a) & (sum ^ cpu->a) & SIGN_BIT;

	cpu->a = (uint8_t)sum;

	SET_BIT(cpu->status, FLAG_C, carry);
	SET_BIT(cpu->status, FLAG_V, overflow);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_CMP(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = get_operand(cpu, instr->addrmode);
	uint8_t result = cpu->a - op;
	SET_BIT(cpu->status, FLAG_C, op <= cpu->a);
	SET_BIT(cpu->status, FLAG_Z, op == cpu->a);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
	cpu->pc += instr->n_bytes;
}

void exec_CPX(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = get_operand(cpu, instr->addrmode);
	uint8_t result = cpu->x - op;
	SET_BIT(cpu->status, FLAG_C, op <= cpu->x);
	SET_BIT(cpu->status, FLAG_Z, op == cpu->x);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
	cpu->pc += instr->n_bytes;
}

void exec_CPY(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = get_operand(cpu, instr->addrmode);
	uint8_t result = cpu->y - op;
	SET_BIT(cpu->status, FLAG_C, op <= cpu->y);
	SET_BIT(cpu->status, FLAG_Z, op == cpu->y);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
	cpu->pc += instr->n_bytes;
}

void exec_SBC(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = ~get_operand(cpu, instr->addrmode);
	uint16_t sum = cpu->a + op + (cpu->status & FLAG_C);
	bool carry = sum > 0xFF;
	bool overflow = (op ^ cpu->a) & (sum ^ cpu->a) & SIGN_BIT;

	cpu->a = (uint8_t)sum;

	SET_BIT(cpu->status, FLAG_C, carry);
	SET_BIT(cpu->status, FLAG_V, overflow);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
	cpu->pc += instr->n_bytes;
}

void exec_DEC(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = get_operand(cpu, instr->addrmode);

	op--;
	write_byte(cpu, cpu->addrbus, op);
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);
	cpu->pc += instr->n_bytes;
}

void exec_DEX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x--;
	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
	cpu->pc += instr->n_bytes;
}

void exec_DEY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->y--;
	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
	cpu->pc += instr->n_bytes;
}

void exec_INC(t_cpu *cpu, const t_instruct *instr)
{
	uint8_t op = get_operand(cpu, instr->addrmode);

	op++;
	write_byte(cpu, cpu->addrbus, op);
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);
	cpu->pc += instr->n_bytes;
}

void exec_INX(t_cpu *cpu, const t_instruct *instr)
{
	cpu->x++;
	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
	cpu->pc += instr->n_bytes;
}

void exec_INY(t_cpu *cpu, const t_instruct *instr)
{
	cpu->y++;
	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
	cpu->pc += instr->n_bytes;
}

void exec_BRK(t_cpu *cpu, const t_instruct *instr)
{
	// TODO: Understand BRK lol
	cpu->pc += 2;
	push_stack(cpu, cpu->pc_hi);
	push_stack(cpu, cpu->pc_lo);
	push_stack(cpu, cpu->status);
	cpu->pc = cpu->addrbus;
}

void exec_JMP(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = cpu->addrbus;
}

void exec_JSR(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);

	cpu->pc += 2;
	push_stack(cpu, cpu->pc_hi);
	push_stack(cpu, cpu->pc_lo);
	cpu->pc = cpu->addrbus;
}

void exec_RTI(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status = pop_stack(cpu);
	cpu->pc_lo = pop_stack(cpu);
	cpu->pc_hi = pop_stack(cpu);
	cpu->pc++;
}

void exec_RTS(t_cpu *cpu, const t_instruct *instr)
{
	cpu->pc_lo = pop_stack(cpu);
	cpu->pc_hi = pop_stack(cpu);
	cpu->pc++;
}

void exec_BCC(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = !(cpu->status & FLAG_C) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_BCS(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = (cpu->status & FLAG_C) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_BEQ(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = (cpu->status & FLAG_Z) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_BMI(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = (cpu->status & FLAG_N) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_BNE(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = !(cpu->status & FLAG_Z) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_BPL(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = !(cpu->status & FLAG_N) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_BVC(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = !(cpu->status & FLAG_V) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_BVS(t_cpu *cpu, const t_instruct *instr)
{
	get_addr(cpu, instr->addrmode);
	cpu->pc = (cpu->status & FLAG_V) ? cpu->addrbus : cpu->pc + instr->n_bytes;
}

void exec_CLC(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_C;
}

void exec_CLD(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_D;
}

void exec_CLI(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_I;
}

void exec_CLV(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status &= ~FLAG_V;
}

void exec_SEC(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status |= FLAG_C;
}

void exec_SED(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status |= FLAG_D;
}

void exec_SEI(t_cpu *cpu, const t_instruct *instr)
{
	cpu->status |= FLAG_I;
}

void exec_NOP(t_cpu *cpu, const t_instruct *instr)
{
	return ;
}

static void (*const instr_funcs[])(t_cpu *, const t_instruct *) = {
	[LDA] = exec_LDA,
	[LDX] = exec_LDX,
	[LDY] = exec_LDY,
	[STA] = exec_STA,
	[STX] = exec_STX,
	[STY] = exec_STY,
	[TAX] = exec_TAX,
	[TAY] = exec_TAY,
	[TSX] = exec_TSX,
	[TXA] = exec_TXA,
	[TXS] = exec_TXS,
	[TYA] = exec_TYA,
	[PHA] = exec_PHA,
	[PHP] = exec_PHP,
	[PLA] = exec_PLA,
	[PLP] = exec_PLP,
	[ASL] = exec_ASL,
	[LSR] = exec_LSR,
	[ROL] = exec_ROL,
	[ROR] = exec_ROR,
	[AND] = exec_AND,
	[BIT] = exec_BIT,
	[EOR] = exec_EOR,
	[ORA] = exec_ORA,
	[ADC] = exec_ADC,
	[CMP] = exec_CMP,
	[CPX] = exec_CPX,
	[CPY] = exec_CPY,
	[SBC] = exec_SBC,
	[DEC] = exec_DEC,
	[DEX] = exec_DEX,
	[DEY] = exec_DEY,
	[INC] = exec_INC,
	[INX] = exec_INX,
	[INY] = exec_INY,
	[BRK] = exec_BRK,
	[JMP] = exec_JMP,
	[JSR] = exec_JSR,
	[RTI] = exec_RTI,
	[RTS] = exec_RTS,
	[BCC] = exec_BCC,
	[BCS] = exec_BCS,
	[BEQ] = exec_BEQ,
	[BMI] = exec_BMI,
	[BNE] = exec_BNE,
	[BPL] = exec_BPL,
	[BVC] = exec_BVC,
	[BVS] = exec_BVS,
	[CLC] = exec_CLC,
	[CLD] = exec_CLD,
	[CLI] = exec_CLI,
	[CLV] = exec_CLV,
	[SEC] = exec_SEC,
	[SED] = exec_SED,
	[SEI] = exec_SEI,
	[NOP] = exec_NOP,
};


void	execute_instr(t_cpu *cpu, const t_instruct *instr)
{
	instr_funcs[instr->instruction](cpu, instr);
}
