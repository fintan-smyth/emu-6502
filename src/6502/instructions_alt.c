#include "emu6502.h"
#include "addr_macros.h"
#include "instr_macros.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

static const uint16_t vectors[] = {
	[INT_NONE]	= 0x0000,
	[INT_BRK]	= 0xFFFE,
	[INT_IRQ]	= 0xFFFE,
	[INT_NMI]	= 0xFFFA,
	[INT_RESET]	= 0xFFFC,
};

void	cpu_tick(t_cpu *cpu)
{
	cpu->cycles++;

	if (cpu->dma.active)
	{
		if (cpu->dma.step == 0)
		{
			cpu->dma.step++;
			return;
		}

		bool odd_cycle = (cpu->cycles % 2) != 0;
		if (cpu->dma.step == 1 && odd_cycle)
		{
			cpu->dma.step++;
			return ;
		}

		if (!odd_cycle)
		{
			uint16_t read_addr = (cpu->dma.page << 8) | cpu->dma.offset++;
			cpu->dma.buffer = read_byte(cpu, read_addr);
		}
		else
		{
			write_byte(cpu, 0x2004, cpu->dma.buffer);
			if (cpu->dma.offset > 0xFF)
				cpu->dma.active = false;
		}

		cpu->dma.step++;
		return;
	}

	if (cpu->instr_step == 0)
	{
		if (cpu->pending_interrupt != INT_NONE)
		{
			// if (cpu->pending_interrupt == INT_NMI)
			// {
				// printf("NMI detected: cycle %lu\n", cpu->cycles);
				// cpu->debug_int = true;
			// }
			read_byte(cpu, cpu->pc);
			cpu->current_opcode = 0x00;
			// if (cpu->cycles > 0)
			// 	log_instr(cpu->logfd, cpu, get_instruction(cpu->current_opcode));
			cpu->interrupt_vector = vectors[cpu->pending_interrupt];
		}
		else
		{
			cpu->current_opcode = read_byte(cpu, cpu->pc);
			// if (cpu->cycles > 0)
			// 	log_instr(cpu->logfd, cpu, get_instruction(cpu->current_opcode));
			cpu->pc++;

			if (cpu->current_opcode == 0x00)
			{
				cpu->pending_interrupt = INT_BRK;
				cpu->interrupt_vector = vectors[cpu->pending_interrupt];
			}
		}
		cpu->page_crossed = false;
		cpu->instr_step++;
		return ;
	}

	switch (cpu->current_opcode) {
		case (0x00): // BRK IMPLIED
			switch (cpu->instr_step) {
				EXEC_BRK(1)
			}
			break;
		case (0x01): // ORA ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_ORA)
			}
			break;
		case (0x02): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x03): // SLO ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_SLO)
			}
			break;
		case (0x04): // SKB ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_SKB)
			}
			break;
		case (0x05): // ORA ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_ORA)
			}
			break;
		case (0x06): // ASL ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_ASL)
			}
			break;
		case (0x07): // SLO ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_SLO)
			}
			break;
		case (0x08): // PHP IMPLIED
			switch (cpu->instr_step) {
				EXEC_PUSH(1, cpu->status | FLAG_E | FLAG_B)
			}
			break;
		case (0x09): // ORA IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_ORA_CORE)
			}
			break;
		case (0x0a): // ASL ACCUMULATOR
			EXEC_ASL_ACCUMULATOR
			break;
		case (0x0b): // ANC IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_ANC_CORE)
			}
			break;
		case (0x0c): // SKW ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_SKW)
			}
			break;
		case (0x0d): // ORA ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_ORA)
			}
			break;
		case (0x0e): // ASL ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_ASL)
			}
			break;
		case (0x0f): // SLO ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_SLO)
			}
			break;
		case (0x10): // BPL RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE(!(cpu->status & FLAG_N))
			}
			break;
		case (0x11): // ORA ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_ORA)
			}
			break;
		case (0x12): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x13): // SLO ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_SLO)
			}
			break;
		case (0x14): // SKB ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SKB)
			}
			break;
		case (0x15): // ORA ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_ORA)
			}
			break;
		case (0x16): // ASL ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_ASL)
			}
			break;
		case (0x17): // SLO ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SLO)
			}
			break;
		case (0x18): // CLC IMPLIED
			EXEC_CLC
			break;
		case (0x19): // ORA ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_ORA)
			}
			break;
		case (0x1a): // NOP IMPLIED
			EXEC_NOP
			break;
		case (0x1b): // SLO ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_SLO)
			}
			break;
		case (0x1c): // SKW ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_SKW)
			}
			break;
		case (0x1d): // ORA ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_ORA)
			}
			break;
		case (0x1e): // ASL ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_ASL)
			}
			break;
		case (0x1f): // SLO ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_SLO)
			}
			break;
		case (0x20): // JSR ABSOLUTE
			switch (cpu->instr_step) {
				EXEC_JSR(1)
			}
			break;
		case (0x21): // AND ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_AND)
			}
			break;
		case (0x22): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x23): // RLA ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_RLA)
			}
			break;
		case (0x24): // BIT ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_BIT)
			}
			break;
		case (0x25): // AND ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_AND)
			}
			break;
		case (0x26): // ROL ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_ROL)
			}
			break;
		case (0x27): // RLA ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_RLA)
			}
			break;
		case (0x28): // PLP IMPLIED
			switch (cpu->instr_step) {
				EXEC_PLP(1)
			}
			break;
		case (0x29): // AND IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_AND_CORE)
			}
			break;
		case (0x2a): // ROL ACCUMULATOR
			EXEC_ROL_ACCUMULATOR
			break;
		case (0x2b): // ANC IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_ANC_CORE)
			}
			break;
		case (0x2c): // BIT ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_BIT)
			}
			break;
		case (0x2d): // AND ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_AND)
			}
			break;
		case (0x2e): // ROL ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_ROL)
			}
			break;
		case (0x2f): // RLA ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_RLA)
			}
			break;
		case (0x30): // BMI RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE(cpu->status & FLAG_N)
			}
			break;
		case (0x31): // AND ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_AND)
			}
			break;
		case (0x32): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x33): // RLA ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_RLA)
			}
			break;
		case (0x34): // SKB ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SKB)
			}
			break;
		case (0x35): // AND ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_AND)
			}
			break;
		case (0x36): // ROL ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_ROL)
			}
			break;
		case (0x37): // RLA ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_RLA)
			}
			break;
		case (0x38): // SEC IMPLIED
			EXEC_SEC
			break;
		case (0x39): // AND ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_AND)
			}
			break;
		case (0x3a): // NOP IMPLIED
			EXEC_NOP
			break;
		case (0x3b): // RLA ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_RLA)
			}
			break;
		case (0x3c): // SKW ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_SKW)
			}
			break;
		case (0x3d): // AND ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_AND)
			}
			break;
		case (0x3e): // ROL ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_ROL)
			}
			break;
		case (0x3f): // RLA ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_RLA)
			}
			break;
		case (0x40): // RTI IMPLIED
			switch (cpu->instr_step) {
				EXEC_RTI(1)
			}
			break;
		case (0x41): // EOR ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_EOR)
			}
			break;
		case (0x42): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x43): // SRE ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_SRE)
			}
			break;
		case (0x44): // SKB ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_SKB)
			}
			break;
		case (0x45): // EOR ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_EOR)
			}
			break;
		case (0x46): // LSR ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_LSR)
			}
			break;
		case (0x47): // SRE ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_SRE)
			}
			break;
		case (0x48): // PHA IMPLIED
			switch (cpu->instr_step) {
				EXEC_PUSH(1, cpu->a)
			}
			break;
		case (0x49): // EOR IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_EOR_CORE)
			}
			break;
		case (0x4a): // LSR ACCUMULATOR
			EXEC_LSR_ACCUMULATOR
			break;
		case (0x4b): // ASR IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_ASR_CORE)
			}
			break;
		case (0x4c): // JMP ABSOLUTE
			switch (cpu->instr_step) {
				EXEC_JMP(1)
			}
			break;
		case (0x4d): // EOR ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_EOR)
			}
			break;
		case (0x4e): // LSR ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_LSR)
			}
			break;
		case (0x4f): // SRE ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_SRE)
			}
			break;
		case (0x50): // BVC RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE(!(cpu->status & FLAG_V))
			}
			break;
		case (0x51): // EOR ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_EOR)
			}
			break;
		case (0x52): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x53): // SRE ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_SRE)
			}
			break;
		case (0x54): // SKB ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SKB)
			}
			break;
		case (0x55): // EOR ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_EOR)
			}
			break;
		case (0x56): // LSR ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_LSR)
			}
			break;
		case (0x57): // SRE ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SRE)
			}
			break;
		case (0x58): // CLI IMPLIED
			EXEC_CLI
			break;
		case (0x59): // EOR ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_EOR)
			}
			break;
		case (0x5a): // NOP IMPLIED
			EXEC_NOP
			break;
		case (0x5b): // SRE ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_SRE)
			}
			break;
		case (0x5c): // SKW ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_SKW)
			}
			break;
		case (0x5d): // EOR ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_EOR)
			}
			break;
		case (0x5e): // LSR ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_LSR)
			}
			break;
		case (0x5f): // SRE ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_SRE)
			}
			break;
		case (0x60): // RTS IMPLIED
			switch (cpu->instr_step) {
				EXEC_RTS(1)
			}
			break;
		case (0x61): // ADC ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE_X_INDIRECT(EXEC_ADC)
				#else
					TICK_ZEROPAGE_X_INDIRECT(EXEC_ADC_DECIMAL)
				#endif
			}
			break;
		case (0x62): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x63): // RRA ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_RRA)
			}
			break;
		case (0x64): // SKB ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_SKB)
			}
			break;
		case (0x65): // ADC ZEROPAGE
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE(EXEC_ADC)
				#else
					TICK_ZEROPAGE(EXEC_ADC_DECIMAL)
				#endif
			}
			break;
		case (0x66): // ROR ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_ROR)
			}
			break;
		case (0x67): // RRA ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_RRA)
			}
			break;
		case (0x68): // PLA IMPLIED
			switch (cpu->instr_step) {
				EXEC_PLA(1)
			}
			break;
		case (0x69): // ADC IMMEDIATE
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_IMMEDIATE(EXEC_ADC_CORE)
				#else
					TICK_IMMEDIATE(EXEC_ADC_DECIMAL_CORE)
				#endif
			}
			break;
		case (0x6a): // ROR ACCUMULATOR
			EXEC_ROR_ACCUMULATOR
			break;
		case (0x6b): // ARR IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_ARR_CORE)
			}
			break;
		case (0x6c): // JMP ABSOLUTE_INDIRECT
			switch (cpu->instr_step) {
				EXEC_JMP_INDIRECT(1);
			}
			break;
		case (0x6d): // ADC ABSOLUTE
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ABSOLUTE(EXEC_ADC)
				#else
					TICK_ABSOLUTE(EXEC_ADC_DECIMAL)
				#endif
			}
			break;
		case (0x6e): // ROR ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_ROR)
			}
			break;
		case (0x6f): // RRA ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_RRA)
			}
			break;
		case (0x70): // BVS RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE((cpu->status & FLAG_V))
			}
			break;
		case (0x71): // ADC ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_ADC)
				#else
					TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_ADC_DECIMAL)
				#endif
			}
			break;
		case (0x72): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x73): // RRA ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_RRA)
			}
			break;
		case (0x74): // SKB ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SKB)
			}
			break;
		case (0x75): // ADC ZEROPAGE_X
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE_X(EXEC_ADC)
				#else
					TICK_ZEROPAGE_X(EXEC_ADC_DECIMAL)
				#endif
			}
			break;
		case (0x76): // ROR ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_ROR)
			}
			break;
		case (0x77): // RRA ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_RRA)
			}
			break;
		case (0x78): // SEI IMPLIED
			EXEC_SEI
			break;
		case (0x79): // ADC ABSOLUTE_Y
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ABSOLUTE_Y_READ(EXEC_ADC)
				#else
					TICK_ABSOLUTE_Y_READ(EXEC_ADC_DECIMAL)
				#endif
			}
			break;
		case (0x7a): // NOP IMPLIED
			EXEC_NOP
			break;
		case (0x7b): // RRA ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_RRA)
			}
			break;
		case (0x7c): // SKW ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_SKW)
			}
			break;
		case (0x7d): // ADC ABSOLUTE_X
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ABSOLUTE_X_READ(EXEC_ADC)
				#else
					TICK_ABSOLUTE_X_READ(EXEC_ADC_DECIMAL)
				#endif
			}
			break;
		case (0x7e): // ROR ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_ROR)
			}
			break;
		case (0x7f): // RRA ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_RRA)
			}
			break;
		case (0x80): // SKB IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_NOP_CORE)
			}
			break;
		case (0x81): // STA ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_STA)
			}
			break;
		case (0x82): // SKB IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_NOP_CORE)
			}
			break;
		case (0x83): // SAX ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_SAX)
			}
			break;
		case (0x84): // STY ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_STY)
			}
			break;
		case (0x85): // STA ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_STA)
			}
			break;
		case (0x86): // STX ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_STX)
			}
			break;
		case (0x87): // SAX ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_SAX)
			}
			break;
		case (0x88): // DEY IMPLIED
			EXEC_DEY
			break;
		case (0x89): // SKB IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_NOP_CORE)
			}
			break;
		case (0x8a): // TXA IMPLIED
			switch (cpu->instr_step) {
				EXEC_TXA(1)
			}
			break;
		case (0x8b): // XAA IMMEDIATE
			switch (cpu->instr_step) {
				// TICK_IMMEDIATE(EXEC_XAA)
			}
			break;
		case (0x8c): // STY ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_STY)
			}
			break;
		case (0x8d): // STA ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_STA)
			}
			break;
		case (0x8e): // STX ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_STX)
			}
			break;
		case (0x8f): // SAX ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_SAX)
			}
			break;
		case (0x90): // BCC RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE(!(cpu->status & FLAG_C))
			}
			break;
		case (0x91): // STA ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_STA)
			}
			break;
		case (0x92): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0x93): // SHA ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_SHA)
			}
			break;
		case (0x94): // STY ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_STY)
			}
			break;
		case (0x95): // STA ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_STA)
			}
			break;
		case (0x96): // STX ZEROPAGE_Y
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y(EXEC_STX)
			}
			break;
		case (0x97): // SAX ZEROPAGE_Y
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y(EXEC_SAX)
			}
			break;
		case (0x98): // TYA IMPLIED
			switch (cpu->instr_step) {
				EXEC_TYA(1)
			}
			break;
		case (0x99): // STA ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_STA)
			}
			break;
		case (0x9a): // TXS IMPLIED
			switch (cpu->instr_step) {
				EXEC_TXS(1)
			}
			break;
		case (0x9b): // SHS ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_SHS)
			}
			break;
		case (0x9c): // SHY ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_SHY)
			}
			break;
		case (0x9d): // STA ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_STA)
			}
			break;
		case (0x9e): // SHX ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_SHX)
			}
			break;
		case (0x9f): // SHA ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_SHA)
			}
			break;
		case (0xa0): // LDY IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_LDY_CORE)
			}
			break;
		case (0xa1): // LDA ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_LDA)
			}
			break;
		case (0xa2): // LDX IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_LDX_CORE)
			}
			break;
		case (0xa3): // LAX ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_LAX)
			}
			break;
		case (0xa4): // LDY ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_LDY)
			}
			break;
		case (0xa5): // LDA ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_LDA)
			}
			break;
		case (0xa6): // LDX ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_LDX)
			}
			break;
		case (0xa7): // LAX ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_LAX)
			}
			break;
		case (0xa8): // TAY IMPLIED
			switch (cpu->instr_step) {
				EXEC_TAY(1)
			}
			break;
		case (0xa9): // LDA IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_LDA_CORE)
			}
			break;
		case (0xaa): // TAX IMPLIED
			switch (cpu->instr_step) {
				EXEC_TAX(1)
			}
			break;
		case (0xab): // OAL IMMEDIATE
			switch (cpu->instr_step) {
				// TICK_IMMEDIATE(EXEC_OAL)
			}
			break;
		case (0xac): // LDY ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_LDY)
			}
			break;
		case (0xad): // LDA ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_LDA)
			}
			break;
		case (0xae): // LDX ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_LDX)
			}
			break;
		case (0xaf): // LAX ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_LAX)
			}
			break;
		case (0xb0): // BCS RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE((cpu->status & FLAG_C))
			}
			break;
		case (0xb1): // LDA ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_LDA)
			}
			break;
		case (0xb2): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0xb3): // LAX ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_LAX)
			}
			break;
		case (0xb4): // LDY ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_LDY)
			}
			break;
		case (0xb5): // LDA ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_LDA)
			}
			break;
		case (0xb6): // LDX ZEROPAGE_Y
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y(EXEC_LDX)
			}
			break;
		case (0xb7): // LAX ZEROPAGE_Y
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y(EXEC_LAX)
			}
			break;
		case (0xb8): // CLV IMPLIED
			EXEC_CLV
			break;
		case (0xb9): // LDA ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_LDA)
			}
			break;
		case (0xba): // TSX IMPLIED
			switch (cpu->instr_step) {
				EXEC_TSX(1)
			}
			break;
		case (0xbb): // LAS ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_LAS)
			}
			break;
		case (0xbc): // LDY ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_LDY)
			}
			break;
		case (0xbd): // LDA ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_LDA)
			}
			break;
		case (0xbe): // LDX ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_LDX)
			}
			break;
		case (0xbf): // LAX ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_LAX)
			}
			break;
		case (0xc0): // CPY IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_CMP_CORE(cpu->y))
			}
			break;
		case (0xc1): // CMP ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_CMP)
			}
			break;
		case (0xc2): // SKB IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_NOP_CORE)
			}
			break;
		case (0xc3): // DCP ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_DCP)
			}
			break;
		case (0xc4): // CPY ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_CPY)
			}
			break;
		case (0xc5): // CMP ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_CMP)
			}
			break;
		case (0xc6): // DEC ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_DEC)
			}
			break;
		case (0xc7): // DCP ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_DCP)
			}
			break;
		case (0xc8): // INY IMPLIED
			EXEC_INY
			break;
		case (0xc9): // CMP IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_CMP_CORE(cpu->a))
			}
			break;
		case (0xca): // DEX IMPLIED
			EXEC_DEX
			break;
		case (0xcb): // SBX IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_SBX_CORE)
			}
			break;
		case (0xcc): // CPY ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_CPY)
			}
			break;
		case (0xcd): // CMP ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_CMP)
			}
			break;
		case (0xce): // DEC ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_DEC)
			}
			break;
		case (0xcf): // DCP ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_DCP)
			}
			break;
		case (0xd0): // BNE RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE(!(cpu->status & FLAG_Z))
			}
			break;
		case (0xd1): // CMP ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_CMP)
			}
			break;
		case (0xd2): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0xd3): // DCP ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_DCP)
			}
			break;
		case (0xd4): // SKB ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SKB)
			}
			break;
		case (0xd5): // CMP ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_CMP)
			}
			break;
		case (0xd6): // DEC ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_DEC)
			}
			break;
		case (0xd7): // DCP ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_DCP)
			}
			break;
		case (0xd8): // CLD IMPLIED
			EXEC_CLD
			break;
		case (0xd9): // CMP ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_READ(EXEC_CMP)
			}
			break;
		case (0xda): // NOP IMPLIED
			EXEC_NOP
			break;
		case (0xdb): // DCP ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_DCP)
			}
			break;
		case (0xdc): // SKW ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_SKW)
			}
			break;
		case (0xdd): // CMP ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_CMP)
			}
			break;
		case (0xde): // DEC ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_DEC)
			}
			break;
		case (0xdf): // DCP ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_DCP)
			}
			break;
		case (0xe0): // CPX IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_CMP_CORE(cpu->x))
			}
			break;
		case (0xe1): // SBC ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE_X_INDIRECT(EXEC_SBC)
				#else
					TICK_ZEROPAGE_X_INDIRECT(EXEC_SBC_DECIMAL)
				#endif
			}
			break;
		case (0xe2): // SKB IMMEDIATE
			switch (cpu->instr_step) {
				TICK_IMMEDIATE(EXEC_NOP_CORE)
			}
			break;
		case (0xe3): // ISC ZEROPAGE_X_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X_INDIRECT(EXEC_ISC)
			}
			break;
		case (0xe4): // CPX ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_CPX)
			}
			break;
		case (0xe5): // SBC ZEROPAGE
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE(EXEC_SBC)
				#else
					TICK_ZEROPAGE(EXEC_SBC_DECIMAL)
				#endif
			}
			break;
		case (0xe6): // INC ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_INC)
			}
			break;
		case (0xe7): // ISC ZEROPAGE
			switch (cpu->instr_step) {
				TICK_ZEROPAGE(EXEC_ISC)
			}
			break;
		case (0xe8): // INX IMPLIED
			EXEC_INX
			break;
		case (0xe9): // SBC IMMEDIATE
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_IMMEDIATE(EXEC_SBC_CORE)
				#else
					TICK_IMMEDIATE(EXEC_SBC_DECIMAL_CORE)
				#endif
			}
			break;
		case (0xea): // NOP IMPLIED
			EXEC_NOP
			break;
		case (0xeb): // SBC IMMEDIATE
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_IMMEDIATE(EXEC_SBC_CORE)
				#else
					TICK_IMMEDIATE(EXEC_SBC_DECIMAL_CORE)
				#endif
			}
			break;
		case (0xec): // CPX ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_CPX)
			}
			break;
		case (0xed): // SBC ABSOLUTE
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ABSOLUTE(EXEC_SBC)
				#else
					TICK_ABSOLUTE(EXEC_SBC_DECIMAL)
				#endif
			}
			break;
		case (0xee): // INC ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_INC)
			}
			break;
		case (0xef): // ISC ABSOLUTE
			switch (cpu->instr_step) {
				TICK_ABSOLUTE(EXEC_ISC)
			}
			break;
		case (0xf0): // BEQ RELATIVE
			switch (cpu->instr_step) {
				EXEC_BRANCH_CORE((cpu->status & FLAG_Z))
			}
			break;
		case (0xf1): // SBC ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_SBC)
				#else
					TICK_ZEROPAGE_Y_INDIRECT_READ(EXEC_SBC_DECIMAL)
				#endif
			}
			break;
		case (0xf2): // HLT IMPLIED
			switch (cpu->instr_step) {
				EXEC_HLT(1)
			}
			break;
		case (0xf3): // ISC ZEROPAGE_Y_INDIRECT
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_Y_INDIRECT_WRITE(EXEC_ISC)
			}
			break;
		case (0xf4): // SKB ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_SKB)
			}
			break;
		case (0xf5): // SBC ZEROPAGE_X
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ZEROPAGE_X(EXEC_SBC)
				#else
					TICK_ZEROPAGE_X(EXEC_SBC_DECIMAL)
				#endif
			}
			break;
		case (0xf6): // INC ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_INC)
			}
			break;
		case (0xf7): // ISC ZEROPAGE_X
			switch (cpu->instr_step) {
				TICK_ZEROPAGE_X(EXEC_ISC)
			}
			break;
		case (0xf8): // SED IMPLIED
			EXEC_SED
			break;
		case (0xf9): // SBC ABSOLUTE_Y
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ABSOLUTE_Y_READ(EXEC_SBC)
				#else
					TICK_ABSOLUTE_Y_READ(EXEC_SBC_DECIMAL)
				#endif
			}
			break;
		case (0xfa): // NOP IMPLIED
			EXEC_NOP
			break;
		case (0xfb): // ISC ABSOLUTE_Y
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_Y_WRITE(EXEC_ISC)
			}
			break;
		case (0xfc): // SKW ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_READ(EXEC_SKW)
			}
			break;
		case (0xfd): // SBC ABSOLUTE_X
			switch (cpu->instr_step) {
				#ifdef NES_MODE
					TICK_ABSOLUTE_X_READ(EXEC_SBC)
				#else
					TICK_ABSOLUTE_X_READ(EXEC_SBC_DECIMAL)
				#endif
			}
			break;
		case (0xfe): // INC ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_INC)
			}
			break;
		case (0xff): // ISC ABSOLUTE_X
			switch (cpu->instr_step) {
				TICK_ABSOLUTE_X_WRITE(EXEC_ISC)
			}
			break;
	}
}
