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
#include "qemu/osdep.h"
#include "cpu.h"
#include "migration/cpu.h"
#include "migration/qemu-file-types.h"

/* --- VMState Info Functions (Custom Types) --- */

/**
 * get_p16_reg:
 * @f: QEMUFile incoming stream.
 * @opaque: Pointer to the internal 32-bit register variable.
 * @size: Expected size (unused here).
 * @field: VMState field metadata.
 *
 * Deserializes a 16-bit register value from the migration stream
 * and zero-extends it into the internal 32-bit CPU state representation.
 */
static int get_p16_reg(QEMUFile *f, void *opaque, size_t size,
                       const VMStateField *field)
{
    uint32_t *v = opaque;
    *v = qemu_get_be16(f);
    return 0;
}

/**
 * put_p16_reg:
 * @f: QEMUFile outgoing stream.
 * @opaque: Pointer to the internal 32-bit register variable.
 * @size: Expected size (unused here).
 * @field: VMState field metadata.
 * @vmdesc: JSON writer for debugging.
 *
 * Truncates the internal 32-bit representation of a register into
 * a 16-bit big-endian value to save bandwidth and enforce architectural
 * boundaries during live migration/snapshots.
 */
static int put_p16_reg(QEMUFile *f, void *opaque, size_t size,
                       const VMStateField *field, JSONWriter *vmdesc)
{
    uint32_t *v = opaque;
    qemu_put_be16(f, (uint16_t)*v);
    return 0;
}

/**
 * vmstate_info_p16_reg:
 * * Custom VMState type mapping for P16 architectural registers.
 * Links the serializer hooks to the VMSTATE macros.
 */
const VMStateInfo vmstate_info_p16_reg = {
    .name = "p16_register",
    .get  = get_p16_reg,
    .put  = put_p16_reg,
};


/* --- Migration Hooks --- */

/**
 * p16_cpu_pre_save:
 * @opaque: Pointer to the P16CPU object.
 *
 * Hook invoked immediately before the CPU state is serialized.
 * Since QEMU often keeps ALU flags (Z, N, C, V) spread across multiple
 * variables for TCG performance, we must pack them into a single
 * architectural CPSR value before transmitting.
 */
static int p16_cpu_pre_save(void *opaque) {
    P16CPU *cpu = opaque;
    CPUP16State *env = &cpu->env;

    env->cpsr_backup = p16_get_cpsr(env);
    return 0;
}

/**
 * p16_cpu_post_load:
 * @opaque: Pointer to the P16CPU object.
 * @version_id: Migration stream version.
 *
 * Hook invoked immediately after the CPU state is deserialized.
 * Unpacks the transmitted single CPSR word back into the internal
 * split-flag variables used by the TCG engine.
 */
static int p16_cpu_post_load(void *opaque, int version_id) {
    P16CPU *cpu = opaque;
    CPUP16State *env = &cpu->env;

    p16_set_cpsr(env, env->cpsr_backup);
    return 0;
}


/* --- VMState Description --- */

/**
 * vms_p16_cpu:
 * * Defines the precise data layout for live migration and save-states.
 * Any structural changes to the CPU state requires bumping the version_id
 * to prevent restoring incompatible states from older QEMU versions.
 */
const VMStateDescription vms_p16_cpu = {
    .name = "cpu",
    .version_id = 1,
    .minimum_version_id = 1,
    .pre_save = p16_cpu_pre_save,
    .post_load = p16_cpu_post_load,
    .fields = (const VMStateField[]) {
        /* General Purpose Registers (R0 to R15) */
        VMSTATE_ARRAY(env.regs, P16CPU, NUMBER_OF_VISIBLE_REGISTERS, 0,
                      vmstate_info_p16_reg, uint32_t),

        /* Banked Return Address for Interrupts (Shadow R14) */
        VMSTATE_SINGLE(env.banked_r14, P16CPU, 0,
                       vmstate_info_p16_reg, uint32_t),

        /* Saved Program Status Register (Exception state) */
        VMSTATE_SINGLE(env.spsr, P16CPU, 0,
                       vmstate_info_p16_reg, uint32_t),

        /* Packed Current Program Status Register */
        VMSTATE_UINT16(env.cpsr_backup, P16CPU),

        VMSTATE_END_OF_LIST()
    }
};