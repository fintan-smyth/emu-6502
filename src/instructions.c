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

void exec_LDA(t_cpu *cpu, AddrMode mode)
{
	cpu->a = get_operand(cpu, mode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_LDX(t_cpu *cpu, AddrMode mode)
{
	cpu->x = get_operand(cpu, mode);

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
}

void exec_LDY(t_cpu *cpu, AddrMode mode)
{
	cpu->y = get_operand(cpu, mode);

	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
}

void exec_STA(t_cpu *cpu, AddrMode mode)
{
	get_addr(cpu, mode);
	write_byte(cpu, cpu->addrbus, cpu->a);
}

void exec_STX(t_cpu *cpu, AddrMode mode)
{
	get_addr(cpu, mode);
	write_byte(cpu, cpu->addrbus, cpu->x);
}

void exec_STY(t_cpu *cpu, AddrMode mode)
{
	get_addr(cpu, mode);
	write_byte(cpu, cpu->addrbus, cpu->y);
}

void exec_TAX(t_cpu *cpu, AddrMode mode)
{
	cpu->x = cpu->a;

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
}

void exec_TAY(t_cpu *cpu, AddrMode mode)
{
	cpu->y = cpu->a;

	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
}

void exec_TSX(t_cpu *cpu, AddrMode mode)
{
	cpu->x = cpu->sp;

	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
}

void exec_TXA(t_cpu *cpu, AddrMode mode)
{
	cpu->a = cpu->x;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_TXS(t_cpu *cpu, AddrMode mode)
{
	cpu->sp = cpu->x;
}

void exec_TYA(t_cpu *cpu, AddrMode mode)
{
	cpu->a = cpu->y;

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_PHA(t_cpu *cpu, AddrMode mode)
{
	uint16_t addr = cpu->sp-- | 0x100;
	write_byte(cpu, addr, cpu->a);
}

void exec_PHP(t_cpu *cpu, AddrMode mode)
{
	uint16_t addr = cpu->sp-- | 0x100;
	write_byte(cpu, addr, cpu->status);
}

void exec_PLA(t_cpu *cpu, AddrMode mode)
{
	uint16_t addr = ++cpu->sp | 0x100;
	cpu->a = read_byte(cpu, addr);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_PLP(t_cpu *cpu, AddrMode mode)
{
	uint16_t addr = ++cpu->sp | 0x100;
	cpu->status = read_byte(cpu, addr);
}

void exec_ASL(t_cpu *cpu, AddrMode mode)
{
	if (mode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_7);
		SET_BIT(cpu->status, FLAG_C, to_carry);

		cpu->a <<= 1;
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		return ;
	}

	uint8_t val = get_operand(cpu, mode);
	bool to_carry = (val & BIT_7);
	SET_BIT(cpu->status, FLAG_C, to_carry);

	val <<= 1;
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);
	write_byte(cpu, cpu->addrbus, val);
}

void exec_LSR(t_cpu *cpu, AddrMode mode)
{
	if (mode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_0);
		SET_BIT(cpu->status, FLAG_C, to_carry);

		cpu->a >>= 1;
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		return ;
	}

	uint8_t val = get_operand(cpu, mode);
	bool to_carry = (val & BIT_0);
	SET_BIT(cpu->status, FLAG_C, to_carry);

	val >>= 1;
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);

	write_byte(cpu, cpu->addrbus, val);
}

void exec_ROL(t_cpu *cpu, AddrMode mode)
{
	if (mode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_7);
		cpu->a <<= 1;
		SET_BIT(cpu->a, BIT_0, cpu->status & FLAG_C);

		SET_BIT(cpu->status, FLAG_C, to_carry);
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		return ;
	}

	uint8_t val = get_operand(cpu, mode);

	bool to_carry = (val & BIT_7);
	val <<= 1;
	SET_BIT(val, BIT_0, cpu->status & FLAG_C);

	SET_BIT(cpu->status, FLAG_C, to_carry);
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);

	write_byte(cpu, cpu->addrbus, val);
}

void exec_ROR(t_cpu *cpu, AddrMode mode)
{
	if (mode == ACCUMULATOR)
	{
		bool to_carry = (cpu->a & BIT_0);
		cpu->a >>= 1;
		SET_BIT(cpu->a, BIT_7, cpu->status & FLAG_C);

		SET_BIT(cpu->status, FLAG_C, to_carry);
		SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
		SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);

		return ;
	}

	uint8_t val = get_operand(cpu, mode);

	bool to_carry = (val & BIT_0);
	val >>= 1;
	SET_BIT(val, BIT_7, cpu->status & FLAG_C);

	SET_BIT(cpu->status, FLAG_C, to_carry);
	SET_BIT(cpu->status, FLAG_N, val & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, val == 0);

	write_byte(cpu, cpu->addrbus, val);
}

void exec_AND(t_cpu *cpu, AddrMode mode)
{
	cpu->a &= get_operand(cpu, mode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_BIT(t_cpu *cpu, AddrMode mode)
{
	uint8_t result = cpu->a & get_operand(cpu, mode);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, result == 0);
	SET_BIT(cpu->status, FLAG_V, result & BIT_6);
}

void exec_EOR(t_cpu *cpu, AddrMode mode)
{
	cpu->a ^= get_operand(cpu, mode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_ORA(t_cpu *cpu, AddrMode mode)
{
	cpu->a |= get_operand(cpu, mode);

	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_ADC(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = get_operand(cpu, mode);
	uint16_t sum = cpu->a + op + (cpu->status & BIT_0);
	bool carry = sum > 0xFF;
	bool overflow = ~(op ^ cpu->a) & (sum ^ cpu->a) & SIGN_BIT;

	cpu->a = (uint8_t)sum;

	SET_BIT(cpu->status, FLAG_C, carry);
	SET_BIT(cpu->status, FLAG_V, overflow);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_CMP(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = get_operand(cpu, mode);
	uint8_t result = cpu->a - op;
	SET_BIT(cpu->status, FLAG_C, op <= cpu->a);
	SET_BIT(cpu->status, FLAG_Z, op == cpu->a);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
}

void exec_CPX(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = get_operand(cpu, mode);
	uint8_t result = cpu->x - op;
	SET_BIT(cpu->status, FLAG_C, op <= cpu->x);
	SET_BIT(cpu->status, FLAG_Z, op == cpu->x);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
}

void exec_CPY(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = get_operand(cpu, mode);
	uint8_t result = cpu->y - op;
	SET_BIT(cpu->status, FLAG_C, op <= cpu->y);
	SET_BIT(cpu->status, FLAG_Z, op == cpu->y);
	SET_BIT(cpu->status, FLAG_N, result & SIGN_BIT);
}

void exec_SBC(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = ~get_operand(cpu, mode);
	uint16_t sum = cpu->a + op + (cpu->status & FLAG_C);
	bool carry = sum > 0xFF;
	bool overflow = (op ^ cpu->a) & (sum ^ cpu->a) & SIGN_BIT;

	cpu->a = (uint8_t)sum;

	SET_BIT(cpu->status, FLAG_C, carry);
	SET_BIT(cpu->status, FLAG_V, overflow);
	SET_BIT(cpu->status, FLAG_N, cpu->a & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->a == 0);
}

void exec_DEC(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = get_operand(cpu, mode);

	op--;
	write_byte(cpu, cpu->addrbus, op);
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);
}

void exec_DEX(t_cpu *cpu, AddrMode mode)
{
	cpu->x--;
	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
}

void exec_DEY(t_cpu *cpu, AddrMode mode)
{
	cpu->y--;
	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
}

void exec_INC(t_cpu *cpu, AddrMode mode)
{
	uint8_t op = get_operand(cpu, mode);

	op++;
	write_byte(cpu, cpu->addrbus, op);
	SET_BIT(cpu->status, FLAG_N, op & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, op == 0);
}

void exec_INX(t_cpu *cpu, AddrMode mode)
{
	cpu->x++;
	SET_BIT(cpu->status, FLAG_N, cpu->x & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->x == 0);
}

void exec_INY(t_cpu *cpu, AddrMode mode)
{
	cpu->y++;
	SET_BIT(cpu->status, FLAG_N, cpu->y & SIGN_BIT);
	SET_BIT(cpu->status, FLAG_Z, cpu->y == 0);
}

void exec_BRK(t_cpu *cpu, AddrMode mode)
{
	// TODO: Understand BRK lol
}

void exec_JMP(t_cpu *cpu, AddrMode mode)
{
	get_addr(cpu, mode);
	cpu->pc = cpu->addrbus;
}

void exec_JSR(t_cpu *cpu, AddrMode mode)
{
	get_addr(cpu, mode);

	cpu->pc += 2;
	cpu->memory[cpu->sp--] = cpu->pc_hi;
	cpu->memory[cpu->sp--] = cpu->pc_lo;
	cpu->pc = cpu->addrbus;
}

void exec_RTI(t_cpu *cpu, AddrMode mode)
{
}

void exec_RTS(t_cpu *cpu, AddrMode mode)
{
}

void exec_BCC(t_cpu *cpu, AddrMode mode)
{
}

void exec_BCS(t_cpu *cpu, AddrMode mode)
{
}

void exec_BEQ(t_cpu *cpu, AddrMode mode)
{
}

void exec_BMI(t_cpu *cpu, AddrMode mode)
{
}

void exec_BNE(t_cpu *cpu, AddrMode mode)
{
}

void exec_BPL(t_cpu *cpu, AddrMode mode)
{
}

void exec_BVC(t_cpu *cpu, AddrMode mode)
{
}

void exec_BVS(t_cpu *cpu, AddrMode mode)
{
}

void exec_CLC(t_cpu *cpu, AddrMode mode)
{
}

void exec_CLD(t_cpu *cpu, AddrMode mode)
{
}

void exec_CLI(t_cpu *cpu, AddrMode mode)
{
}

void exec_CLV(t_cpu *cpu, AddrMode mode)
{
}

void exec_SEC(t_cpu *cpu, AddrMode mode)
{
}

void exec_SED(t_cpu *cpu, AddrMode mode)
{
}

void exec_SEI(t_cpu *cpu, AddrMode mode)
{
}

void exec_NOP(t_cpu *cpu, AddrMode mode)
{
}

static void (*const instr_funcs[])(t_cpu *, AddrMode mode) = {
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
	instr_funcs[instr->instruction](cpu, instr->addrmode);
}
