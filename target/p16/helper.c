/*
 * QEMU P16 CPU helpers
 *
 * Copyright (c) 2025-2026 Rafael Romão
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include "qemu/osdep.h"
#include "cpu.h"
#include "accel/tcg/cpu-ops.h"
#include "exec/cpu-interrupt.h"
#include "exec/cputlb.h"
#include "exec/page-protection.h"
#include "exec/target_page.h"
#include "accel/tcg/cpu-ldst.h"
#include "exec/helper-proto.h"
#include "qemu/plugin.h"

/**
 * p16_cpu_exec_interrupt:
 * @cs: Generic CPU state.
 * @interrupt_request: Bitmask of pending interrupt types.
 *
 * Evaluates if a hardware interrupt (IRQ) is pending and if the CPU's
 * current architectural state permits handling it.
 *
 * Returns: true if an interrupt was handled, false otherwise.
 */
bool p16_cpu_exec_interrupt(CPUState *cs, int interrupt_request)
{
    P16CPU *cpu = P16_CPU(cs);
    CPUP16State *env = &cpu->env;
    bool ret = false;

    /*
     * We only care about external hardware interrupts (HARD).
     * If there's an active line and the CPU's global interrupt enable (I)
     * bit is set, we process the context switch.
     */
    if (interrupt_request & CPU_INTERRUPT_HARD) {
        if (env->irq_en && env->intsrc != 0) {
            /*
             * Reset the generic exception index. The P16 handles the vector
             * resolution internally during do_interrupt.
             */
            cs->exception_index = -1;

            p16_cpu_do_interrupt(cs);
            ret = true;
        }
    }

    return ret;
}

/**
 * p16_cpu_do_interrupt:
 * @cs: Generic CPU state.
 *
 * Performs the architectural context switch for an exception/interrupt.
 * Saves the current execution state (PC and CPSR) into the shadow/banked
 * registers, alters the CPU mode, and forces the Program Counter to the
 * Interrupt Service Routine (ISR) vector.
 */
void p16_cpu_do_interrupt(CPUState *cs)
{
    P16CPU *cpu = P16_CPU(cs);
    CPUP16State *env = &cpu->env;

    /*
     * 1. Save Context (Return Address)
     * Store the current PC into the banked Link Register (iLR)
     * so the CPU knows where to return after the RFI instruction.
     */
    env->banked_r14 = env->regs[PC_REG];

    /*
     * 2. Save Context (Status Flags)
     * Freeze the current ALU flags and mode into the Saved Program
     * Status Register (SPSR).
     */
    env->spsr = p16_get_cpsr(env);

    /*
     * 3. Modify CPU State
     * Switch to Interrupt Mode and immediately disable
     * further nested interrupts to prevent stack overflow or loops.
     */
    p16_switch_mode(env, P16_MODE_INTERRUPT);
    env->irq_en = 0;

    /*
     * 4. Branch to Vector
     * Hijack the instruction pointer to the hardcoded ISR address.
     */
    env->regs[PC_REG] = ISR_ADDR;

    cs->exception_index = -1;
}

/**
 * p16_cpu_tlb_fill:
 * @cs: Generic CPU state.
 * @address: The virtual address the CPU is trying to access.
 * @size: Access size.
 * @access_type: Type of memory operation (Read, Write, Fetch).
 * @mmu_idx: Current MMU execution context.
 * @probe: If true, only check permissions without causing a fault.
 * @retaddr: Host address to return to if a fault exception is raised.
 *
 * Resolves a SoftMMU TLB miss.
 * Since the P16 is a simple microcontroller without a Virtual Memory
 * Management Unit (no paging), this function performs a 1:1 Identity
 * Mapping (Virtual Address == Physical Address) with full permissions.
 */
bool p16_cpu_tlb_fill(CPUState *cs, vaddr address, int size,
                      MMUAccessType access_type, int mmu_idx,
                      bool probe, uintptr_t retaddr)
{
    /* Align the address to the start of the page boundary */
    address &= TARGET_PAGE_MASK;

    /*
     * Map the virtual page to the exact same physical page.
     * All accesses (RWX) are inherently granted in this architecture.
     */
    tlb_set_page(cs, address,
                 address,
                 PAGE_READ | PAGE_WRITE | PAGE_EXEC,
                 mmu_idx, TARGET_PAGE_SIZE);

    return true;
}

/**
 * HELPER(get_cpsr):
 * @cs: Pointer to the P16 architectural state.
 *
 * TCG Helper: Translates the architectural CPSR value to the
 * intermediate representation during code execution.
 */
uint32_t HELPER(get_cpsr)(CPUP16State *cs) {
    return p16_get_cpsr(cs);
}

/**
 * HELPER(set_cpsr):
 * @cs: Pointer to the P16 architectural state.
 * @val: New CPSR value calculated by the TCG.
 *
 * TCG Helper: Updates the architectural CPSR from translated host code.
 */
void HELPER(set_cpsr)(CPUP16State *cs, uint32_t val) {
    p16_set_cpsr(cs, val);
}