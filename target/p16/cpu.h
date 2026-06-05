/*
 * QEMU P16 CPU
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
#ifndef QEMU_P16_CPU_H
#define QEMU_P16_CPU_H

#include <stdint.h>

#include "cpu-qom.h"
#include "exec/cpu-common.h"
#include "system/memory.h"

/* User-mode emulation is invalid. */
#ifdef CONFIG_USER_ONLY
#error "P16 does not support user mode"
#endif

#define CPU_RESOLVING_TYPE TYPE_P16_CPU

#define MMU_IDX 0

/* --- Register Definitions --- */
#define NUMBER_OF_VISIBLE_REGISTERS 16
#define SP_REG 13  /* Stack Pointer */
#define LR_REG 14  /* Link Register (Active or Banked iLR) */
#define PC_REG 15  /* Program Counter */

/* Interrupt Service Routine Vector */
#define ISR_ADDR 0x0002

/* --- Program Status Register (PSR) Layout --- */
#define PSR_MODE_BIT     5
#define PSR_IRQ_BIT      4
#define PSR_NEGATIVE_BIT 3
#define PSR_OVERFLOW_BIT 2
#define PSR_CARRY_BIT    1
#define PSR_ZERO_BIT     0

#define PSR_MODE_MASK     (1 << PSR_MODE_BIT)     /* 0x0020 */
#define PSR_IRQ_MASK      (1 << PSR_IRQ_BIT)      /* 0x0010 */
#define PSR_NEGATIVE_MASK (1 << PSR_NEGATIVE_BIT) /* 0x0008 */
#define PSR_OVERFLOW_MASK (1 << PSR_OVERFLOW_BIT) /* 0x0004 */
#define PSR_CARRY_MASK    (1 << PSR_CARRY_BIT)    /* 0x0002 */
#define PSR_ZERO_MASK     (1 << PSR_ZERO_BIT)     /* 0x0001 */

/* --- CPU Modes --- */
typedef enum {
    P16_MODE_NORMAL = 0,
    P16_MODE_INTERRUPT = 1
} P16Mode;

/**
 * CPUP16State:
 *
 * The architectural execution state of the P16 processor.
 * Notice that ALU condition flags (Z, C, V, N) are stored as isolated
 * 32-bit integers rather than a single packed CPSR register. This is a
 * standard QEMU optimization that significantly improves the performance
 * of the TCG JIT compiler during flag calculations.
 */
typedef struct CPUArchState {
    /* General Purpose Registers (R0 - R15) */
    uint32_t regs[NUMBER_OF_VISIBLE_REGISTERS];

    /* Shadow register for R14 (Register Banking during Interrupts) */
    uint32_t banked_r14;

    /* Saved Program Status Register (Holds CPSR during exceptions) */
    uint32_t spsr;

    /* TCG Optimized Split Condition Flags */
    uint32_t cond_z; /* Zero */
    uint32_t cond_c; /* Carry */
    uint32_t cond_v; /* Overflow */
    uint32_t cond_n; /* Negative */

    /* Packed CPSR used for VMState Live Migration & Snapshots */
    uint16_t cpsr_backup;

    /* Control Flags */
    uint32_t irq_en; /* Global Hardware Interrupt Enable */
    uint32_t mode;   /* Current CPU Privilege Mode */

    /* Pending Hardware Interrupt Source Bitmap */
    uint32_t intsrc;

} CPUP16State;

/**
 * p16_switch_mode:
 * @env: CPU architecture state.
 * @mode: Target mode (Normal or Interrupt).
 *
 * Handles the architectural context switch.
 * Swaps the active R14 (Link Register) with the shadow register
 * (banked_r14) to preserve the background thread's return address
 * while the ISR executes.
 */
static inline void p16_switch_mode(CPUP16State *env, uint32_t mode) {
    if (env->mode == mode) {
        return;
    }

    /* Hardware Register Banking Swap */
    const uint32_t tmp = env->regs[LR_REG];
    env->regs[LR_REG] = env->banked_r14;
    env->banked_r14 = tmp;

    env->mode = mode;
}

/**
 * p16_get_cpsr:
 * @env: CPU architecture state.
 *
 * Packs the TCG split variables back into the architectural CPSR word.
 */
static inline uint32_t p16_get_cpsr(const CPUP16State *env)
{
    uint32_t cpsr = 0;

    if (env->cond_z) cpsr |= PSR_ZERO_MASK;
    if (env->cond_n) cpsr |= PSR_NEGATIVE_MASK;
    if (env->cond_c) cpsr |= PSR_CARRY_MASK;
    if (env->cond_v) cpsr |= PSR_OVERFLOW_MASK;
    if (env->irq_en) cpsr |= PSR_IRQ_MASK;
    if (env->mode == P16_MODE_INTERRUPT) cpsr |= PSR_MODE_MASK;

    return cpsr;
}

/**
 * p16_set_cpsr:
 * @env: CPU architecture state.
 * @val: Packed CPSR word.
 *
 * Unpacks the architectural CPSR word back into the TCG split variables.
 */
static inline void p16_set_cpsr(CPUP16State *env, uint32_t val)
{
    env->cond_z = (val & PSR_ZERO_MASK) != 0;
    env->cond_n = (val & PSR_NEGATIVE_MASK) != 0;
    env->cond_c = (val & PSR_CARRY_MASK) != 0;
    env->cond_v = (val & PSR_OVERFLOW_MASK) != 0;
    env->irq_en = (val & PSR_IRQ_MASK) != 0;

    p16_switch_mode(env, (val & PSR_MODE_MASK) ? P16_MODE_INTERRUPT : P16_MODE_NORMAL);
}

/**
 * P16CPU:
 * @env: CPU architectural state.
 * @init_pc: Boot program counter injected by the Machine Loader.
 *
 * The QOM instance structure for the P16 CPU object.
 */
struct ArchCPU {
    CPUState parent_obj;
    CPUP16State env;
    uint32_t init_pc;
};

/**
 * P16CPUClass:
 * @parent_realize: The parent class' realize handler.
 * @parent_phases: The parent class' reset phase handlers.
 *
 * The QOM class structure for the P16 CPU.
 */
struct P16CPUClass {
    CPUClass parent_class;
    DeviceRealize parent_realize;
    ResettablePhases parent_phases;
};

extern const VMStateDescription vms_p16_cpu;

/**
 * p16_cpu_do_interrupt:
 * @cs: Generic CPU state.
 *
 * Performs the architectural context switch for an exception/interrupt.
 * Saves the current execution state (PC and CPSR) into the shadow/banked
 * registers, alters the CPU mode, and forces the Program Counter to the
 * Interrupt Service Routine (ISR) vector.
 */
void p16_cpu_do_interrupt(CPUState *cpu);
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
bool p16_cpu_exec_interrupt(CPUState *cpu, int int_req);

/**
 * p16_cpu_gdb_read_register:
 * @cs: Generic CPU state.
 * @buf: Byte array where the serialized register value will be appended.
 * @n: GDB register index requested by the debugger.
 *
 * Handles GDB 'g' (read all) and 'p' (read single) packets.
 * Includes architectural register banking logic to ensure GDB inspects
 * the correct shadow register based on the active CPU mode.
 *
 * Returns: The size of the registered read in bytes (2), or 0 if unknown.
 */
int p16_cpu_gdb_read_register(CPUState *cpu, GByteArray *buf, int reg);

/**
 * p16_cpu_gdb_write_register:
 * @cs: Generic CPU state.
 * @mem_buf: Raw byte buffer containing the value sent by GDB.
 * @n: GDB register index to be modified.
 *
 * Handles GDB 'G' (write all) and 'P' (write single) packets.
 * Deserializes the incoming value and correctly routes it to either
 * the active or banked physical registers.
 *
 * Returns: The size of the register written in bytes (2), or 0 if unknown.
 */
int p16_cpu_gdb_write_register(CPUState *cpu, uint8_t *buf, int reg);

/**
 * p16_cpu_tcg_init:
 *
 * Initializes the TCG environment for the P16 CPU.
 * Called once during QEMU boot. Responsible for allocating and
 * structurally mapping the processor's registers and state flags to TCG global variables.
 */
void p16_cpu_tcg_init(void);

/**
 * p16_cpu_translate_code:
 * @cs: Pointer to the CPU State.
 * @tb: Pointer to the Translation Block.
 * @max_insns: Pointer to the maximum number of instructions to translate.
 * @pc: Current program counter.
 * @host_pc: Host PC.
 *
 * Entry point of the execution system to translate P16 code to the Host.
 */
void p16_cpu_translate_code(CPUState *cs, TranslationBlock *tb,
                            int *max_insns, vaddr pc, void *host_pc);

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
                      bool probe, uintptr_t retaddr);

#endif /* QEMU_P16_CPU_H */