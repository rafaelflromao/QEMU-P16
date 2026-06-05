/*
 * QEMU P16 CPU gdbstub
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
#include "exec/gdbstub.h"

#include "gdbstub/helpers.h"

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
int p16_cpu_gdb_read_register(CPUState *cs, GByteArray *buf, int n)
{
    P16CPU *cpu = P16_CPU(cs);
    CPUP16State *env = &cpu->env;

    /* Standard General Purpose Registers (R0 - R15) */
    if (n >= 0 && n < 16) {
        /*
         * Register Banking: R14 (Link Register) is shadowed.
         * If the CPU is in Interrupt Mode, the active R14 is banked_r14.
         */
        if (n == 14 && env->mode == P16_MODE_INTERRUPT) {
            return gdb_get_reg16(buf, env->banked_r14);
        }
        return gdb_get_reg16(buf, env->regs[n]);
    }

    /* System and Coprocessor Registers */
    switch (n) {
        case 16: /* CPSR (Current Program Status Register) */
            /* Pack the internal TCG flags into the architectural representation */
            return gdb_get_reg16(buf, p16_get_cpsr(env));

        case 17: /* iLR (Background/User Link Register) */
            /* * Allows GDB to inspect the background thread's return address
             * even while the CPU is currently handling an interrupt.
             */
            if (env->mode == P16_MODE_INTERRUPT) {
                return gdb_get_reg16(buf, env->regs[LR_REG]);
            }
            return gdb_get_reg16(buf, env->banked_r14);

        case 18: /* SPSR (Saved Program Status Register) */
            return gdb_get_reg16(buf, env->spsr);

        default:
            /* Unsupported register index */
            return 0;
    }
}

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
int p16_cpu_gdb_write_register(CPUState *cs, uint8_t *mem_buf, int n)
{
    P16CPU *cpu = P16_CPU(cs);
    CPUP16State *env = &cpu->env;
    const uint16_t tmp = lduw_p(mem_buf); /* Load unsigned word (16-bit) */

    /* Standard General Purpose Registers (R0 - R15) */
    if (n >= 0 && n < 16) {
        /* Route the write to the banked R14 if in Interrupt Mode */
        if (n == 14 && env->mode == P16_MODE_INTERRUPT) {
            env->banked_r14 = tmp;
            return 2;
        }
        env->regs[n] = tmp;
        return 2;
    }

    /* iLR and Status Registers */
    switch (n) {
        case 16: /* CPSR */
            /* Unpack the architectural value back into the TCG flags */
            p16_set_cpsr(env, tmp);
            return 2;

        case 17: /* iLR */
            /* Allow GDB to modify the background thread's R14 */
            if (env->mode == P16_MODE_INTERRUPT) {
                env->regs[LR_REG] = tmp;
                return 2;
            }
            env->banked_r14 = tmp;
            return 2;

        case 18: /* SPSR */
            env->spsr = tmp;
            return 2;

        default:
            return 0;
    }
}