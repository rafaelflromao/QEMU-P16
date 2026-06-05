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
#include "qapi/error.h"
#include "qemu/qemu-print.h"
#include "exec/translation-block.h"
#include "exec/cpu-interrupt.h"
#include "cpu.h"
#include "disas/dis-asm.h"
#include "tcg/debug-assert.h"
#include "hw/core/qdev-properties.h"
#include "accel/tcg/cpu-ops.h"
#include "hw/core/sysemu-cpu-ops.h"

/* --- Core CPU Operations --- */

/**
 * p16_cpu_set_pc:
 *
 * Forces a new value into the Program Counter.
 * Architectural detail: The P16 enforces 16-bit instruction alignment.
 * The least significant bit (bit 0) is aggressively cleared to prevent
 * unaligned execution faults.
 */
static void p16_cpu_set_pc(CPUState *cs, vaddr value) {
    P16CPU *cpu = P16_CPU(cs);
    cpu->env.regs[PC_REG] = value & 0xFFFE;
}

/**
 * p16_cpu_get_pc:
 * @cs: Generic CPU state.
 *
 * Retrieves the current Program Counter directly from the architectural state.
 *
 * Returns: The unmasked value of the PC register.
 */
static vaddr p16_cpu_get_pc(CPUState *cs) {
    P16CPU *cpu = P16_CPU(cs);
    return cpu->env.regs[PC_REG];
}

/**
 * p16_cpu_has_work:
 *
 * Evaluates if the CPU should wake up from a halted state (e.g., WFI).
 * The CPU wakes up if a Reset is pending, or if a Hardware Interrupt
 * is pending AND the global interrupt enable flag (irq_en) is active.
 */
static bool p16_cpu_has_work(CPUState *cs) {
    return cpu_test_interrupt(cs, CPU_INTERRUPT_RESET) ||
           (cpu_test_interrupt(cs, CPU_INTERRUPT_HARD) && P16_CPU(cs)->env.irq_en);
}

/**
 * p16_cpu_set_int:
 * @opaque: Pointer to the CPU object.
 * @irq: Interrupt line index.
 * @level: High (1) or Low (0) state of the line.
 *
 * Simulates the physical IRQ pin of the processor.
 * Updates the internal interrupt source bitmap and triggers/clears
 * the QEMU main loop interrupt request accordingly.
 */
static void p16_cpu_set_int(void *opaque, int irq, int level) {
    P16CPU *cpu = opaque;
    CPUP16State *env = &cpu->env;
    CPUState *cs = CPU(cpu);
    uint32_t mask = (1 << irq);

    if (level) {
        env->intsrc &= ~mask;
        if (env->intsrc == 0) {
            cpu_reset_interrupt(cs, CPU_INTERRUPT_HARD);
        }
    } else {
        env->intsrc |= mask;
        cpu_interrupt(cs, CPU_INTERRUPT_HARD);
    }
}

/* --- State & Lifecycle Management --- */

/**
 * p16_cpu_reset_hold:
 *
 * Simulates a hardware reset (power-on or warm reset).
 * Flushes all registers, clears condition flags, disables interrupts,
 * and forces the CPU back into Normal Mode with PC pointing to 0x0000.
 */
static void p16_cpu_reset_hold(Object *obj, ResetType type) {
    CPUState *cs = CPU(obj);
    P16CPU *cpu = P16_CPU(cs);
    P16CPUClass *mcc = P16_CPU_GET_CLASS(obj);
    CPUP16State *env = &cpu->env;

    /* Propagate reset to parent classes */
    if (mcc->parent_phases.hold) {
        mcc->parent_phases.hold(obj, type);
    }

    memset(env->regs, 0, sizeof(env->regs));

    env->cond_z = 0;
    env->cond_c = 0;
    env->cond_n = 0;
    env->cond_v = 0;

    env->irq_en = 0;
    env->mode = P16_MODE_NORMAL;
    env->spsr = 0;
    env->banked_r14 = 0;

    env->regs[SP_REG] = 0;
    env->regs[PC_REG] = 0;
}

/**
 * p16_cpu_dump_state:
 *
 * Generates the human-readable string output for the QEMU Monitor
 * 'info registers' command and crash logs. Crucial for debugging.
 */
static void p16_cpu_dump_state(CPUState *cs, FILE *f, int flags) {
    P16CPU *cpu = P16_CPU(cs);
    CPUP16State *env = &cpu->env;
    uint32_t cpsr = p16_get_cpsr(env);

    qemu_fprintf(f, "CPU State:\n");
    qemu_fprintf(f, "PC:   %04x   SP:   %04x   LR:   %04x   iLR:   %04x\n",
                 env->regs[PC_REG], env->regs[SP_REG], env->regs[LR_REG], env->banked_r14);

    qemu_fprintf(f, "CPSR: %04x [M:%d I:%d N:%d V:%d C:%d Z:%d]\n",
                 cpsr, env->mode & 1, env->irq_en & 1,
                 env->cond_n & 1, env->cond_v & 1,
                 env->cond_c & 1, env->cond_z & 1);

    qemu_fprintf(f, "SPSR: %04x [M:%d I:%d N:%d V:%d C:%d Z:%d]\n",
                 env->spsr,
                 (env->spsr & PSR_MODE_MASK) ? 1 : 0,
                 (env->spsr & PSR_IRQ_MASK) ? 1 : 0,
                 (env->spsr & PSR_NEGATIVE_MASK) ? 1 : 0,
                 (env->spsr & PSR_OVERFLOW_MASK) ? 1 : 0,
                 (env->spsr & PSR_CARRY_MASK) ? 1 : 0,
                 (env->spsr & PSR_ZERO_MASK) ? 1 : 0);

    qemu_fprintf(f, "nINT Line: %d ", (env->intsrc != 0) ? 0 : 1);
    qemu_fprintf(f, "------------------------------------------\n");

    for (int i = 0; i < 16; i++) {
        qemu_fprintf(f, "R%02d: %04x", i, env->regs[i]);
        if ((i % 4) == 3) {
            qemu_fprintf(f, "\n");
        } else {
            qemu_fprintf(f, "   ");
        }
    }
    qemu_fprintf(f, "\n");
}

/* --- TCG & Emulation Integration Hooks --- */

/**
 * p16_cpu_synchronize_from_tb:
 *
 * Synchronizes the architectural CPU state with the TCG translation block.
 * Called when execution traps out of the generated code.
 */
static void p16_cpu_synchronize_from_tb(CPUState *cs,
                                        const TranslationBlock *tb) {
    tcg_debug_assert(!tcg_cflags_has(cs, CF_PCREL));
    P16CPU *cpu = P16_CPU(cs);
    cpu->env.regs[PC_REG] = tb->pc;
}

/**
 * p16_restore_state_to_opc:
 *
 * Unwinds the CPU state to a specific host opcode during exception handling.
 * Essential for accurate fault reporting (e.g., MMU faults).
 */
static void p16_restore_state_to_opc(CPUState *cs,
                                     const TranslationBlock *tb,
                                     const uint64_t *data) {
    P16CPU *cpu = P16_CPU(cs);
    cpu->env.regs[PC_REG] = data[0];
}

/**
 * p16_cpu_get_tb_cpu_state:
 * @cs: Generic CPU state.
 *
 * Generates the lookup key for the TCG Translation Block cache.
 * The P16 translation relies strictly on the PC. If the architecture had
 * execution modes that changed instruction decoding (like ARM's Thumb mode),
 * those specific state flags would need to be appended here.
 *
 * Returns: A populated TCGTBCPUState struct used for cache hashing.
 */
static TCGTBCPUState p16_cpu_get_tb_cpu_state(CPUState *cs)
{
    return (TCGTBCPUState){ .pc = p16_cpu_get_pc(cs), .flags = 0 };
}

/**
 * p16_cpu_mmu_index:
 * @cs: Generic CPU state.
 * @ifetch: True if this is an instruction fetch, false for data access.
 *
 * Determines the current Memory Management Unit index.
 * Since the P16 lacks a complex MMU hierarchy (e.g., no User vs Kernel
 * virtual memory isolation), it returns a static/default index.
 */
static int p16_cpu_mmu_index(CPUState *cs, bool ifetch) {
    return MMU_IDX;
}

/* --- Debugging & GDB --- */

/**
 * p16_cpu_get_phys_page_debug:
 * @cs: Generic CPU state.
 * @addr: Virtual address requested by the debugger.
 *
 * Resolves a virtual address to a physical address for GDB inspection.
 * Since the P16 has no address translation hardware, this performs a
 * 1:1 Identity mapping.
 */
static hwaddr p16_cpu_get_phys_page_debug(CPUState *cs, vaddr addr) {
    return addr;
}

/**
 * p16_cpu_gdb_adjust_breakpoint:
 * @cs: Generic CPU state.
 * @addr: Requested breakpoint address.
 *
 * Normalizes debugger breakpoint addresses.
 * Enforces 16-bit alignment to ensure GDB software breakpoints trap
 * correctly on valid instruction boundaries, preventing partial opcode corruption.
 */
static vaddr p16_cpu_gdb_adjust_breakpoint(CPUState *cs, vaddr addr)
{
    return addr & 0xFFFE;
}

/**
 * p16_cpu_disas_set_info:
 * @cpu: Generic CPU state.
 * @info: Disassembler configuration structure.
 *
 * Configures the internal QEMU disassembler for the QEMU Monitor (hmp)
 * and execution tracing tools (e.g., using '-d in_asm').
 */
static void p16_cpu_disas_set_info(const CPUState *cpu, disassemble_info *info) {
    info->endian = BFD_ENDIAN_LITTLE;
    info->mach = bfd_arch_p16;
    info->print_insn = print_insn_p16;
}

/* --- QOM (QEMU Object Model) --- */

/**
 * p16_cpu_realizefn:
 * @dev: The generic device state.
 * @errp: Error pointer for propagating initialization failures.
 *
 * Finalizes the CPU object creation.
 * Validates configuration, allocates QEMU VCPU execution threads,
 * and performs the initial hardware reset.
 */
static void p16_cpu_realizefn(DeviceState *dev, Error **errp) {
    CPUState *cs = CPU(dev);
    P16CPUClass *mcc = P16_CPU_GET_CLASS(dev);
    Error *local_err = NULL;

    cpu_exec_realizefn(cs, &local_err);
    if (local_err != NULL) {
        error_propagate(errp, local_err);
        return;
    }
    qemu_init_vcpu(cs);
    cpu_reset(cs);

    mcc->parent_realize(dev, errp);
}

/**
 * p16_cpu_initfn:
 * @obj: The raw QOM object instance.
 *
 * Early instance initialization.
 * Sets up internal state and registers inbound GPIO lines (such as
 * the hardware interrupt pin) before the CPU is formally realized.
 */
static void p16_cpu_initfn(Object *obj) {
    P16CPU *cpu = P16_CPU(obj);
    /* Initialize the inbound IRQ GPIO line */
    qdev_init_gpio_in(DEVICE(cpu), p16_cpu_set_int, 1);
}

/**
 * p16_cpu_class_by_name:
 * @cpu_model: String passed via the command line (e.g., "-cpu p16-core").
 *
 * Resolves a CPU model name string to its corresponding QOM class.
 */
static ObjectClass *p16_cpu_class_by_name(const char *cpu_model) {
    return object_class_by_name(cpu_model);
}

static const struct SysemuCPUOps p16_sysemu_ops = {
    .has_work = p16_cpu_has_work,
    .get_phys_page_debug = p16_cpu_get_phys_page_debug,
};

static const TCGCPUOps p16_tcg_ops = {
    .guest_default_memory_order = 0,
    .mttcg_supported = false,
    .initialize = p16_cpu_tcg_init,
    .translate_code = p16_cpu_translate_code,
    .synchronize_from_tb = p16_cpu_synchronize_from_tb,
    .restore_state_to_opc = p16_restore_state_to_opc,
    .mmu_index = p16_cpu_mmu_index,
    .cpu_exec_interrupt = p16_cpu_exec_interrupt,
    .cpu_exec_halt = p16_cpu_has_work,
    .cpu_exec_reset = cpu_reset,
    .tlb_fill = p16_cpu_tlb_fill,
    .do_interrupt = p16_cpu_do_interrupt,
    .get_tb_cpu_state = p16_cpu_get_tb_cpu_state,
    .pointer_wrap = cpu_pointer_wrap_uint32,
};

/**
 * p16_cpu_class_init:
 * @oc: The object class being initialized.
 * @data: Opaque data passed during type registration.
 *
 * Class initialization callback.
 * Populates the virtual method tables (vtable) for the P16 CPU class,
 * registering our architecture-specific hooks into QEMU's generic
 * Sysemu, QOM, and TCG subsystems.
 */
static void p16_cpu_class_init(ObjectClass *oc, const void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);
    CPUClass *cc = CPU_CLASS(oc);
    P16CPUClass *mcc = P16_CPU_CLASS(oc);
    ResettableClass *rc = RESETTABLE_CLASS(oc);

    device_class_set_parent_realize(dc, p16_cpu_realizefn, &mcc->parent_realize);
    resettable_class_set_parent_phases(rc, NULL, p16_cpu_reset_hold, NULL,
                                       &mcc->parent_phases);

    cc->class_by_name = p16_cpu_class_by_name;
    cc->dump_state = p16_cpu_dump_state;
    cc->set_pc = p16_cpu_set_pc;
    cc->get_pc = p16_cpu_get_pc;
    dc->vmsd = &vms_p16_cpu;
    cc->sysemu_ops = &p16_sysemu_ops;
    cc->disas_set_info = p16_cpu_disas_set_info;
    cc->gdb_read_register = p16_cpu_gdb_read_register;
    cc->gdb_write_register = p16_cpu_gdb_write_register;
    cc->gdb_adjust_breakpoint = p16_cpu_gdb_adjust_breakpoint;
    cc->gdb_stop_before_watchpoint = true;
    cc->gdb_core_xml_file = "p16-core.xml";
    cc->tcg_ops = &p16_tcg_ops;
}

/*
 * TODO: For future architectural extensions (e.g., specific P16 core variants),
 * this base class should be marked as abstract (.abstract = true), and specific
 * CPU models should inherit from it. For now, a single concrete class suffices.
 */
static const TypeInfo p16_cpu_type_info = {
    .name = TYPE_P16_CPU,
    .parent = TYPE_CPU,
    .instance_size = sizeof(P16CPU),
    .instance_align = __alignof(P16CPU),
    .instance_init = p16_cpu_initfn,
    .class_size = sizeof(P16CPUClass),
    .class_init = p16_cpu_class_init
};

static void p16_cpu_register_types(void) {
    type_register_static(&p16_cpu_type_info);
}

type_init(p16_cpu_register_types)