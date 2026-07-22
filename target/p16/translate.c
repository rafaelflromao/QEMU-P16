/*
 * QEMU P16 CPU translation
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
#include "qemu/bitops.h"
#include "qemu/qemu-print.h"
#include "tcg/tcg.h"
#include "cpu.h"
#include "exec/translation-block.h"
#include "tcg/tcg-op.h"
#include "exec/helper-proto.h"
#include "exec/helper-gen.h"
#include "exec/log.h"
#include "exec/translator.h"
#include "exec/target_page.h"

#define HELPER_H "helper.h"
#include "exec/helper-info.c.inc"
#include "tcg/tcg-temp-internal.h"
#undef  HELPER_H

/*
 * TCG Global Variables
 * Memory mapping between the emulated CPU registers and the Host.
 */
static TCGv cpu_r[16];
static TCGv cpu_spsr;
static TCGv cpu_cond_z, cpu_cond_c, cpu_cond_v, cpu_cond_n;
static TCGv cpu_irq_en, cpu_mode;

/**
 * DisasContext:
 * @base: Base translation context.
 * @env: Pointer to the P16 CPU state.
 *
 * Disassembly and Translation Context.
 * Maintains the translation state throughout a Translation Block (TB).
 * Stores information about the current instruction and pointers to the CPU state.
 */
typedef struct DisasContext {
    DisasContextBase base;
    CPUP16State *env;
} DisasContext;

static const char reg_names[NUMBER_OF_VISIBLE_REGISTERS][8] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "sp", "lr", "pc"
};

#define DISAS_JUMP DISAS_TARGET_0
#define DISAS_UPDATE DISAS_TARGET_1
#define DISAS_EXIT DISAS_TARGET_2

/**
 * p16_cpu_tcg_init:
 *
 * Initializes the TCG environment for the P16 CPU.
 * Called once during QEMU boot. Responsible for allocating and
 * structurally mapping the processor's registers and state flags to TCG global variables.
 */
void p16_cpu_tcg_init(void) {
    for (int i = 0; i < 16; i++) {
        cpu_r[i] = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, regs[i]), reg_names[i]);
    }

    cpu_spsr = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, spsr), "spsr");
    cpu_cond_z = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, cond_z), "cond_z");
    cpu_cond_c = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, cond_c), "cond_c");
    cpu_cond_v = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, cond_v), "cond_v");
    cpu_cond_n = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, cond_n), "cond_n");
    cpu_irq_en = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, irq_en), "irq_en");
    cpu_mode = tcg_global_mem_new_i32(tcg_env, offsetof(CPUP16State, mode), "mode");
}

static int mult_2(DisasContext *ctx, int value) {
    return value << 1;
}

/**
 * p16_calc_branch_dest:
 * @ctx: Current translation context.
 * @offset: Relative offset provided by the instruction.
 *
 * Calculates the absolute destination address of a branch.
 *
 * Returns: Calculated virtual address (PC + 2 + Offset).
 */
static inline target_ulong p16_calc_branch_dest(DisasContext *ctx, int offset) {
    return ctx->base.pc_next + 2 + offset;
}

/**
 * calc_cond_z_n:
 * @res: TCG register containing the final operation result (assumed valid 16-bit).
 *
 * Updates the architectural Zero (Z) and Negative (N) flags.
 */
static inline void calc_cond_z_n(TCGv res) {
    TCGv temp = tcg_temp_new_i32();

    tcg_gen_ext16u_i32(temp, res);
    tcg_gen_setcondi_i32(TCG_COND_EQ, cpu_cond_z, temp, 0);

    tcg_gen_shri_i32(cpu_cond_n, res, 15);
    tcg_gen_andi_i32(cpu_cond_n, cpu_cond_n, 1);
}

/**
 * calc_arithmetic_cond_v:
 * @a: Operand 1.
 * @b: Operand 2 (pre-processed with two's complement if it's a subtraction).
 * @res: Final result of the operation.
 *
 * Calculates the Overflow (V) flag for arithmetic operations.
 * Uses boolean logic on the signs of the original operands and the
 * sign of the result to detect two's complement overflow.
 */
static inline void calc_arithmetic_cond_v(TCGv a, TCGv b, TCGv res) {
    TCGv_i32 t0 = tcg_temp_new_i32();
    TCGv_i32 t1 = tcg_temp_new_i32();

    tcg_gen_xor_i32(t0, a, res);
    tcg_gen_xor_i32(t1, b, res);
    tcg_gen_and_i32(cpu_cond_v, t0, t1);

    tcg_gen_shri_i32(cpu_cond_v, cpu_cond_v, 15);
    tcg_gen_andi_i32(cpu_cond_v, cpu_cond_v, 1);
}

static bool decode_insn(DisasContext *ctx, uint16_t insn);

#include "decode-insn.c.inc"

/**
 * gen_goto_tb:
 * @ctx: Current context.
 * @tb_slot_idx: Exit slot index (0 or 1) for chaining optimization.
 * @dest: Virtual destination address.
 *
 * Emits opcodes to transition to the next Translation Block.
 */
static void gen_goto_tb(DisasContext *ctx, int tb_slot_idx, target_ulong dest) {
    if (translator_use_goto_tb(&ctx->base, dest)) {
        tcg_gen_goto_tb(tb_slot_idx);
        tcg_gen_movi_i32(cpu_r[PC_REG], dest);
        tcg_gen_exit_tb(ctx->base.tb, tb_slot_idx);
    } else {
        tcg_gen_movi_i32(cpu_r[PC_REG], dest);
        tcg_gen_lookup_and_goto_ptr();
    }
    ctx->base.is_jmp = DISAS_NORETURN;
}

/* --- Register Access Wrappers --- */

static inline void apply_reg_mask(TCGv_i32 val) {
    tcg_gen_ext16u_i32(val, val);
}

/**
 * load_reg:
 * @dc: Current Translation Context.
 * @reg: Target register index (0 to 15).
 *
 * Reads data from an architectural register.
 *
 * Architectural Exception (R15 / PC):
 * QEMU defers updating the global R15 variable until the end of a
 * Translation Block (Lazy PC Update). To ensure accurate reads mid-block,
 * R15 is never read from the CPU RAM state. Instead, we inject the exact
 * instruction address as a hardcoded TCG constant at translation time.
 */
static TCGv_i32 load_reg(DisasContext *dc, int reg) {
    g_assert(reg >= 0 && reg < 16);
    if (reg == PC_REG) {
        return tcg_constant_i32(p16_calc_branch_dest(dc, 0));
    }
    return cpu_r[reg];
}

/**
 * store_reg:
 * @dc: Current context.
 * @reg: Target register index.
 * @val: Value to be stored.
 *
 * Emits opcodes to store a TCG value in a register and jump if it is PC_REG.
 */
static void store_reg(DisasContext *dc, int reg, TCGv val) {
    if (reg < 0) return;
    g_assert(reg < 16);

    apply_reg_mask(val);

    tcg_gen_mov_i32(cpu_r[reg], val);

    if (reg == PC_REG) {
        dc->base.is_jmp = DISAS_JUMP;
    }
}

/**
 * store_reg:
 * @dc: Current context.
 * @reg: Target register index.
 * @val: Value to be stored.
 *
 * Emits opcodes to store a constant value in a register and jump if it is PC_REG.
 */
static void store_regi(DisasContext *dc, int reg, uint32_t val) {
    if (reg < 0) return;
    g_assert(reg < 16);

    const uint32_t clean_val = val & 0xFFFF;

    if (reg == PC_REG) {
        gen_goto_tb(dc, 1, clean_val);
        return;
    }
    tcg_gen_movi_i32(cpu_r[reg], clean_val);
}

/* --- ALU Core --- */

#define MATH_SUB_BIT 0b1
#define MATH_CARRY_BIT 0b10

/**
 * MathOperation:
 *
 * Bitmask of structural properties for ALU operations.
 */
typedef enum {
    MATH_ADD = 0,                                 /* Simple addition */
    MATH_SUB = MATH_SUB_BIT,                      /* Simple subtraction */
    MATH_ADC = MATH_CARRY_BIT,                    /* Addition with Carry injection */
    MATH_SBC = MATH_SUB_BIT | MATH_CARRY_BIT      /* Subtraction with structural borrow */
} MathOperation;

/**
 * gen_arithmetic_op:
 * @ctx: Current context.
 * @t0: Primary operand.
 * @t1: Secondary operand (will be internally inverted if operation is subtraction).
 * @dest: Destination register.
 * @op: Bitmask containing the operation's mathematical signature.
 *
 * Core engine to emit Arithmetic Logic Unit (ALU) operations.
 * Unifies addition and subtraction processing into a single generator tree,
 * inferring the operation via MathOperation enum bitmasks. Performs calculations
 * and automatic updates of architectural state flags (Z, N, C, V).
 */
static inline void gen_arithmetic_op(DisasContext *ctx, TCGv_i32 t0, TCGv_i32 t1, const int dest, const MathOperation op) {
    TCGv_i32 res = tcg_temp_new_i32();
    TCGv_i32 op2 = tcg_temp_new_i32();
    TCGv_i32 carry_in = tcg_temp_new_i32();

    bool is_sub = (op & MATH_SUB_BIT);
    bool use_carry = (op & MATH_CARRY_BIT);

    if (is_sub) {
        /* If subtraction, get first complement and truncate to 16-bit */
        tcg_gen_not_i32(op2, t1);
        tcg_gen_ext16u_i32(op2, op2);
    } else {
        tcg_gen_mov_i32(op2, t1);
    }

    if (use_carry) {
        tcg_gen_mov_i32(carry_in, cpu_cond_c);
    } else {
        tcg_gen_movi_i32(carry_in, is_sub ? 1 : 0);
    }

    tcg_gen_add_i32(res, t0, op2);
    tcg_gen_add_i32(res, res, carry_in);

    calc_cond_z_n(res);
    calc_arithmetic_cond_v(t0, op2, res);

    tcg_gen_shri_i32(cpu_cond_c, res, 16);
    tcg_gen_andi_i32(cpu_cond_c, cpu_cond_c, 1);

    tcg_gen_ext16u_i32(res, res);
    store_reg(ctx, dest, res);
}

static bool trans_ADD(DisasContext *ctx, arg_ADD *a) {
    gen_arithmetic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), a->rd, MATH_ADD);
    return true;
}

static bool trans_ADD_I(DisasContext *ctx, arg_ADD_I *a) {
    gen_arithmetic_op(ctx, load_reg(ctx, a->rn), tcg_constant_i32(a->imm4), a->rd, MATH_ADD);
    return true;
}

static bool trans_ADC(DisasContext *ctx, arg_ADC *a) {
    gen_arithmetic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), a->rd, MATH_ADC);
    return true;
}

static bool trans_SUB(DisasContext *ctx, arg_SUB *a) {
    gen_arithmetic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), a->rd, MATH_SUB);
    return true;
}

static bool trans_SUB_I(DisasContext *ctx, arg_SUB_I *a) {
    gen_arithmetic_op(ctx, load_reg(ctx, a->rn), tcg_constant_i32(a->imm4), a->rd, MATH_SUB);
    return true;
}

static bool trans_SBC(DisasContext *ctx, arg_SBC *a) {
    gen_arithmetic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), a->rd, MATH_SBC);
    return true;
}

static bool trans_CMP(DisasContext *ctx, arg_CMP *a) {
    gen_arithmetic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), -1, MATH_SUB);
    return true;
}

/**
 * LogicOperation:
 *
 * Supported logical operators.
 */
typedef enum {
    LOGIC_AND,
    LOGIC_OR,
    LOGIC_XOR
} LogicOperation;

/**
 * gen_logic_op:
 * @ctx: Current context.
 * @t0: Primary operand.
 * @t1: Secondary operand.
 * @res: Destination TCG register.
 * @op: Type of logical operation.
 *
 * Generic emitter for logical instructions (AND, OR, XOR).
 */
static inline void gen_logic_op(DisasContext *ctx, TCGv_i32 t0, TCGv_i32 t1, const uint32_t rd, const LogicOperation op) {
    TCGv res = tcg_temp_new_i32();
    switch (op) {
        case LOGIC_AND:
            tcg_gen_and_i32(res, t0, t1);
            break;
        case LOGIC_OR:
            tcg_gen_or_i32(res, t0, t1);
            break;
        case LOGIC_XOR:
            tcg_gen_xor_i32(res, t0, t1);
            break;
    }

    tcg_gen_ext16u_i32(res, res);
    calc_cond_z_n(res);
    store_reg(ctx, rd, res);
}

static bool trans_AND(DisasContext *ctx, arg_AND *a) {
    gen_logic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), a->rd, LOGIC_AND);
    return true;
}

static bool trans_EOR(DisasContext *ctx, arg_EOR *a) {
    gen_logic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), a->rd, LOGIC_XOR);
    return true;
}

static bool trans_ORR(DisasContext *ctx, arg_ORR *a) {
    gen_logic_op(ctx, load_reg(ctx, a->rn), load_reg(ctx, a->rm), a->rd, LOGIC_OR);
    return true;
}

/* --- Shift and Rotate --- */

#define SHIFT_RIGHT_BIT 0b1
#define SHIFT_ARITHMETIC_BIT 0b10
#define SHIFT_ROTATE_BIT 0b100
#define SHIFT_USE_CARRY_BIT 0b1000

/**
 * ShiftOps:
 *
 * Bitmask signature of shifting and rotation characteristics.
 * Allows deconstructing complex operations like RRX into pure
 * mathematical attributes to simplify the TCG tree.
 */
typedef enum {
    SHIFT_LSL = 0,
    SHIFT_LSR = SHIFT_RIGHT_BIT,
    SHIFT_ASR = SHIFT_ARITHMETIC_BIT | SHIFT_RIGHT_BIT,
    SHIFT_ROR = SHIFT_ROTATE_BIT | SHIFT_RIGHT_BIT,
    SHIFT_RRX = SHIFT_RIGHT_BIT | SHIFT_USE_CARRY_BIT,
} ShiftOps;

/**
 * gen_shift_op:
 * @ctx: Current context.
 * @dest: Destination register.
 * @src: Source register.
 * @shift_val: Absolute shift value provided by the instruction.
 * @type: Operation attributes.
 *
 * Unified translation engine for Shift and Rotate operations.
 * Processes logical/arithmetic shifts and rotations within a unified logical tree.
 * Features isolated parallel paths for special architectural cases, such as the RRX
 * (Rotate Right with Extend) instruction, safeguarding C flag integrity and preventing
 * unwanted behaviors caused by zero-shift instructions.
 */
static inline void gen_shift_op(DisasContext *ctx, const int dest, TCGv_i32 src, const unsigned int shift_val, const ShiftOps type) {
    TCGv res = tcg_temp_new_i32();
    tcg_gen_ext16u_i32(res, src);
    TCGv old_carry = tcg_temp_new_i32();
    tcg_gen_mov_i32(old_carry, cpu_cond_c);

    const bool rotate = type & SHIFT_ROTATE_BIT;
    const bool arithmetic = type & SHIFT_ARITHMETIC_BIT;
    const bool right = type & SHIFT_RIGHT_BIT;
    const bool use_carry = type & SHIFT_USE_CARRY_BIT;

    if (arithmetic) {
        tcg_gen_ext16s_i32(res, res);
    }

    /*
     * CASE 1: ZERO SHIFT (Ignore)
     */
    if (shift_val == 0) {
        /* Ignored. Res remains untouched, carry is not updated. */
    }
    /*
     * CASE 2: STANDARD SHIFTS (INCLUDES RRX) AND ROTATES (shift_val > 0)
     */
    else {
        TCGv new_carry = tcg_temp_new_i32();

        if (right) {
            tcg_gen_extract_i32(new_carry, res, shift_val - 1, 1);
        }

        if (rotate) {
            unsigned int mod_shift = shift_val & 15;
            unsigned int inv_shift = 16 - mod_shift;

            TCGv left_part = tcg_temp_new_i32();
            TCGv right_part = tcg_temp_new_i32();

            if (right) {
                tcg_gen_shri_i32(right_part, res, mod_shift);
                tcg_gen_shli_i32(left_part, res, inv_shift);
            } else {
                tcg_gen_shli_i32(left_part, res, mod_shift);
                tcg_gen_shri_i32(right_part, res, inv_shift);
            }
            tcg_gen_or_i32(res, right_part, left_part);

        } else if (right) {
            if (arithmetic) {
                tcg_gen_sari_i32(res, res, shift_val);
            } else {
                tcg_gen_shri_i32(res, res, shift_val);
            }

        } else {
            tcg_gen_shli_i32(res, res, shift_val);
        }

        /* Deposit Carry Bit */
        if (use_carry) {
            if (right) {
                tcg_gen_deposit_i32(res, res, old_carry, 15, 1);
            } else {
                tcg_gen_deposit_i32(res, res, old_carry, 0, 1);
            }
        }

        if (!right) {
            tcg_gen_extract_i32(new_carry, res, 16, 1);
        }

        tcg_gen_mov_i32(cpu_cond_c, new_carry);
    }

    tcg_gen_ext16u_i32(res, res);
    calc_cond_z_n(res);
    store_reg(ctx, dest, res);
}

static bool trans_ASR(DisasContext *ctx, arg_ASR *a) {
    gen_shift_op(ctx, a->rd, load_reg(ctx, a->rn), a->imm4, SHIFT_ASR);
    return true;
}
static bool trans_LSL(DisasContext *ctx, arg_LSL *a) {
    gen_shift_op(ctx, a->rd, load_reg(ctx, a->rn), a->imm4, SHIFT_LSL);
    return true;
}
static bool trans_LSR(DisasContext *ctx, arg_LSR *a) {
    gen_shift_op(ctx, a->rd, load_reg(ctx, a->rn), a->imm4, SHIFT_LSR);
    return true;
}
static bool trans_ROR(DisasContext *ctx, arg_ROR *a) {
    gen_shift_op(ctx, a->rd, load_reg(ctx, a->rn), a->imm4, SHIFT_ROR);
    return true;
}
static bool trans_RRX(DisasContext *ctx, arg_RRX *a) {
    gen_shift_op(ctx, a->rd, load_reg(ctx, a->rn), 1, SHIFT_RRX);
    return true;
}

/* --- Data Transfer Instructions --- */

static bool trans_MOV(DisasContext *ctx, arg_MOV *a) {
    store_reg(ctx, a->rd, load_reg(ctx, a->rm));
    return true;
}

static bool trans_MOV_I(DisasContext *ctx, arg_MOV_I *a) {
    store_regi(ctx, a->rd, a->imm8);
    return true;
}

static bool trans_MOVT(DisasContext *ctx, arg_MOVT *a) {
    TCGv_i32 tmp = tcg_temp_new_i32();
    tcg_gen_deposit_i32(tmp, load_reg(ctx, a->rd), tcg_constant_i32(a->imm8), 8, 8);
    store_reg(ctx, a->rd, tmp);
    return true;
}

static bool trans_MVN(DisasContext *ctx, arg_MVN *a) {
    gen_logic_op(ctx, tcg_constant_i32(0xFFFF), load_reg(ctx, a->rm), a->rd, LOGIC_XOR);
    return true;
}

static bool trans_MOVS(DisasContext *ctx, arg_MOVS *a) {
    store_reg(ctx, PC_REG, load_reg(ctx, LR_REG));
    gen_helper_set_cpsr(tcg_env, cpu_spsr);
    ctx->base.is_jmp = DISAS_EXIT;
    return true;
}

static bool trans_MRS_C(DisasContext *ctx, arg_MRS_C *a) {
    TCGv_i32 cpsr = tcg_temp_new_i32();
    gen_helper_get_cpsr(cpsr, tcg_env);
    store_reg(ctx, a->rd, cpsr);
    return true;
}

static bool trans_MRS_S(DisasContext *ctx, arg_MRS_S *a) {
    store_reg(ctx, a->rd, cpu_spsr);
    return true;
}

static bool trans_MSR_C(DisasContext *ctx, arg_MSR_C *a) {
    TCGv_i32 tmp = tcg_temp_new_i32();
    tcg_gen_andi_i32(tmp, load_reg(ctx, a->rm), 0x003F);
    gen_helper_set_cpsr(tcg_env, tmp);
    ctx->base.is_jmp = DISAS_UPDATE;
    return true;
}

static bool trans_MSR_S(DisasContext *ctx, arg_MSR_S *a) {
    tcg_gen_andi_i32(cpu_spsr, load_reg(ctx, a->rm), 0x003F);
    return true;
}

/* --- Branching --- */

/**
 * gen_branch_cond:
 * @ctx: Current context.
 * @cond: TCG Conditional operator used for verification.
 * @arg1: First comparison parameter.
 * @arg2: Second comparison parameter.
 * @offset: Offset to apply if the condition is met.
 *
 * Generic structure for translating conditional branches.
 */
static void gen_branch_cond(DisasContext *ctx, const TCGCond cond, TCGv_i32 arg1, TCGv_i32 arg2, const int32_t offset) {
    TCGLabel *label_taken = gen_new_label();

    tcg_gen_brcond_i32(cond, arg1, arg2, label_taken);

    gen_goto_tb(ctx, 0, p16_calc_branch_dest(ctx, 0));

    gen_set_label(label_taken);
    gen_goto_tb(ctx, 1, p16_calc_branch_dest(ctx, offset));
}

static bool trans_B(DisasContext *ctx, arg_B *a) {
    gen_goto_tb(ctx, 1, p16_calc_branch_dest(ctx, a->offset));
    return true;
}

static bool trans_BL(DisasContext *ctx, arg_BL *a) {
    store_regi(ctx, LR_REG, p16_calc_branch_dest(ctx, 0));
    gen_goto_tb(ctx, 1, p16_calc_branch_dest(ctx, a->offset));
    return true;
}

static bool trans_BHS(DisasContext *ctx, arg_BHS *a) {
    gen_branch_cond(ctx, TCG_COND_EQ, cpu_cond_c, tcg_constant_i32(1), a->offset);
    return true;
}

static bool trans_BLO(DisasContext *ctx, arg_BLO *a) {
    gen_branch_cond(ctx, TCG_COND_EQ, cpu_cond_c, tcg_constant_i32(0), a->offset);
    return true;
}

static bool trans_BGE(DisasContext *ctx, arg_BGE *a) {
    TCGv_i32 tmp = tcg_temp_new_i32();
    tcg_gen_xor_i32(tmp, cpu_cond_n, cpu_cond_v);

    gen_branch_cond(ctx, TCG_COND_EQ, tmp, tcg_constant_i32(0), a->offset);
    return true;
}

static bool trans_BLT(DisasContext *ctx, arg_BLT *a) {
    TCGv_i32 tmp = tcg_temp_new_i32();
    tcg_gen_xor_i32(tmp, cpu_cond_n, cpu_cond_v);

    gen_branch_cond(ctx, TCG_COND_NE, tmp, tcg_constant_i32(0), a->offset);
    return true;
}

static bool trans_BZS(DisasContext *ctx, arg_BZS *a) {
    gen_branch_cond(ctx, TCG_COND_EQ, cpu_cond_z, tcg_constant_i32(1), a->offset);
    return true;
}

static bool trans_BZC(DisasContext *ctx, arg_BZC *a) {
    gen_branch_cond(ctx, TCG_COND_EQ, cpu_cond_z, tcg_constant_i32(0), a->offset);
    return true;
}

/* --- Memory Interface (Load / Store) --- */

/**
 * gen_load:
 * @ctx: Current context.
 * @rd: Target register index.
 * @addr: Virtual Memory Base Address to be read.
 * @mop: Memory Operation.
 *
 * Emit opcodes to load data from memory and stores in a register.
 */
static void gen_load(DisasContext *ctx, const int rd, TCGv_i32 addr, const MemOp mop) {
    TCGv_i32 dest = tcg_temp_new_i32();
    tcg_gen_qemu_ld_i32(dest, addr, 0, mop);
    store_reg(ctx, rd, dest);
}

/**
 * gen_load:
 * @ctx: Current context.
 * @rd: Target register index.
 * @addr: Virtual Memory Base Address to be read.
 * @mop: Memory Operation.
 *
 * Emit opcodes to store data from a register to memory.
 */
static void gen_store(DisasContext *ctx, const int rm, TCGv_i32 addr, const MemOp mop) {
    tcg_gen_qemu_st_i32(load_reg(ctx, rm), addr, 0, mop);
}

/**
 * gen_addr_imm:
 * @ctx: Current context.
 * @rn: Base address register index.
 * @imm: Address offset immediate.
 *
 * Calculates a memory address ADDR = REGS[rn] + imm
 */
static TCGv_i32 gen_addr_imm(DisasContext *ctx, const int rn, const int imm) {
    TCGv_i32 addr = tcg_temp_new_i32();
    tcg_gen_addi_i32(addr, load_reg(ctx, rn), imm);
    tcg_gen_ext16u_i32(addr, addr);
    return addr;
}

/**
 * gen_addr_reg:
 * @ctx: Current context.
 * @rn: Base address register index.
 * @rm: Address offset register index.
 *
 * Calculates a memory address ADDR = REGS[rn] + REGS[rm]
 */
static TCGv_i32 gen_addr_reg(DisasContext *ctx, const int rn, const int rm) {
    TCGv_i32 addr = tcg_temp_new_i32();
    tcg_gen_add_i32(addr, load_reg(ctx, rn), load_reg(ctx, rm));
    tcg_gen_ext16u_i32(addr, addr);
    return addr;
}

static bool trans_LDR_L(DisasContext *ctx, arg_LDR_L *a) {
    gen_load(ctx, a->rd, tcg_constant_i32(p16_calc_branch_dest(ctx, a->imm7)), MO_LEUW);
    return true;
}

static bool trans_LDR_C(DisasContext *ctx, arg_LDR_C *a) {
    TCGv_i32 addr = gen_addr_imm(ctx, a->rn, a->imm4);
    gen_load(ctx, a->rd, addr, MO_LEUW);
    return true;
}

static bool trans_LDR_R(DisasContext *ctx, arg_LDR_R *a) {
    TCGv_i32 addr = gen_addr_reg(ctx, a->rn, a->rm);
    gen_load(ctx, a->rd, addr, MO_LEUW);
    return true;
}

static bool trans_LDRB_C(DisasContext *ctx, arg_LDRB_C *a) {
    TCGv_i32 addr = gen_addr_imm(ctx, a->rn, a->imm3);
    gen_load(ctx, a->rd, addr, MO_UB);
    return true;
}

static bool trans_LDRB_R(DisasContext *ctx, arg_LDRB_R *a) {
    TCGv_i32 addr = gen_addr_reg(ctx, a->rn, a->rm);
    gen_load(ctx, a->rd, addr, MO_UB);
    return true;
}

static bool trans_STR_C(DisasContext *ctx, arg_STR_C *a) {
    TCGv_i32 addr = gen_addr_imm(ctx, a->rn, a->imm4);
    gen_store(ctx, a->rd, addr, MO_LEUW);
    return true;
}

static bool trans_STR_R(DisasContext *ctx, arg_STR_R *a) {
    TCGv_i32 addr = gen_addr_reg(ctx, a->rn, a->rm);
    gen_store(ctx, a->rd, addr, MO_LEUW);
    return true;
}

static bool trans_STRB_C(DisasContext *ctx, arg_STRB_C *a) {
    TCGv_i32 addr = gen_addr_imm(ctx, a->rn, a->imm3);
    gen_store(ctx, a->rd, addr, MO_UB);
    return true;
}

static bool trans_STRB_R(DisasContext *ctx, arg_STRB_R *a) {
    TCGv_i32 addr = gen_addr_reg(ctx, a->rn, a->rm);
    gen_store(ctx, a->rd, addr, MO_UB);
    return true;
}

/**
 * trans_PUSH:
 * @ctx: Current translation context.
 * @a: Parsed instruction arguments (contains the source register).
 *
 * Translates the PUSH instruction.
 * The P16 stack grows downwards. This function decrements the Stack Pointer (SP)
 * by 2 bytes (16-bit alignment) and stores the target register's value into memory.
 */
static bool trans_PUSH(DisasContext *ctx, arg_PUSH *a) {

    /* 1. SP = SP - 2 */
    TCGv_i32 sp = tcg_temp_new_i32();
    tcg_gen_subi_i32(sp, load_reg(ctx, SP_REG), 2);

    /* 2. [SP] := Rd */
    tcg_gen_qemu_st_i32(load_reg(ctx, a->rd), sp, 0, MO_LEUW);

    /* 3. Update SP register */
    store_reg(ctx, SP_REG, sp);

    return true;
}

/**
 * trans_POP:
 * @ctx: Current translation context.
 * @a: Parsed instruction arguments (contains the destination register).
 *
 * Translates the POP instruction.
 * Loads a 16-bit value from the current Stack Pointer (SP) address into the target
 * register, and then increments the SP by 2 bytes to pop the data.
 */
static bool trans_POP(DisasContext *ctx, arg_POP *a) {

    /* 1. dest := [SP] */
    TCGv_i32 sp = load_reg(ctx, SP_REG);
    TCGv_i32 dest = tcg_temp_new_i32();
    tcg_gen_qemu_ld_i32(dest, sp, 0, MO_LEUW);

    /* 2. SP = SP + 2 */
    TCGv_i32 new_sp = tcg_temp_new_i32();
    tcg_gen_addi_i32(new_sp, sp, 2);
    store_reg(ctx, SP_REG, new_sp);

    /* 3. Rd := dest */
    store_reg(ctx, a->rd, dest);

    return true;
}

/* --- Base TCG Translator Loop --- */

/**
 * p16_tr_init_disas_context:
 * @dcbase: Base translation context.
 * @cs: Current CPU state.
 *
 * Initializes the architecture-specific disassembly context.
 * Binds the global CPU state environment to our local context so it can be
 * accessed during the translation of individual instructions.
 */
static void p16_tr_init_disas_context(DisasContextBase *dcbase, CPUState *cs) {
    DisasContext *ctx = container_of(dcbase, DisasContext, base);
    ctx->env = cpu_env(cs);
}

/**
 * p16_tr_tb_start:
 * @db: Base translation context.
 * @cs: Current CPU state.
 *
 * Hook invoked at the very beginning of a new Translation Block.
 * Left empty as the P16 architecture does not require preliminary setups
 * before emitting a new block of instructions.
 */
static void p16_tr_tb_start(DisasContextBase *db, CPUState *cs) {
}

/**
 * p16_tr_insn_start:
 * @dcbase: Base translation context.
 * @cs: Current CPU state.
 *
 * Hook invoked just before decoding and translating a single instruction.
 * Emits a synchronization opcode that maps the current Guest PC to the Host's
 * generated code.
 */
static void p16_tr_insn_start(DisasContextBase *dcbase, CPUState *cs) {
    DisasContext *ctx = container_of(dcbase, DisasContext, base);
    tcg_gen_insn_start(ctx->base.pc_next, 0, 0);
}

/**
 * translate:
 * @ctx: Current context.
 *
 * Decodes the instruction at the current PC, invoking the generated instruction callbacks.
 */
static void translate(DisasContext *ctx) {
    uint16_t insn = translator_lduw(ctx->env, &ctx->base, ctx->base.pc_next);
    if (!decode_insn(ctx, insn)) {
        qemu_log_mask(LOG_GUEST_ERROR, "Invalid instruction at 0x%08" VADDR_PRIx ": 0x%04x\n",
                      ctx->base.pc_next, insn);
        ctx->base.is_jmp = DISAS_NORETURN;
    }
}

/**
 * p16_tr_translate_insn:
 * @dcbase: Current Base DisasContext.
 * @cs: Current CPU State.
 *
 * Translates an instruction by calling translate function and updates next program counter.
 */
static void p16_tr_translate_insn(DisasContextBase *dcbase, CPUState *cs) {
    DisasContext *ctx = container_of(dcbase, DisasContext, base);
    translate(ctx);
    ctx->base.pc_next += 2;
}

/**
 * p16_tr_tb_stop:
 * @dcbase: Base translation context.
 * @cs: Current CPU state.
 *
 * Finalizes the current Translation Block (TB) by generating the appropriate exit opcodes.
 *
 * This is the critical point where the "Lazy PC Update" philosophy is synchronized
 * with the architectural CPU state. The exit strategy depends on how the last
 * instruction in the block altered the control flow (evaluated via ctx->base.is_jmp).
 */
static void p16_tr_tb_stop(DisasContextBase *dcbase, CPUState *cs) {
    DisasContext *ctx = container_of(dcbase, DisasContext, base);

    switch (ctx->base.is_jmp) {
        case DISAS_NORETURN:
            /*
             * No further action is required here.
             */
            break;

        case DISAS_NEXT:
        case DISAS_TOO_MANY:
            /*
             * Normal sequential execution flow (or the instruction limit was reached).
             * We synchronize the PC and attempt to chain this block directly to the
             * next block for maximum emulation performance.
             */
            gen_goto_tb(ctx, 1, ctx->base.pc_next);
            break;

        case DISAS_JUMP:
            /*
             * Indirect jump (e.g., dynamic PC write via RAM or another register).
             * Since the destination address is unknown at translation time, we use
             * the TCG helper to look up the next block in cache during runtime.
             */
            tcg_gen_lookup_and_goto_ptr();
            break;

        case DISAS_UPDATE:
            /*
             * The PC was successfully updated, but critical CPU states were modified
             * (such as the Interrupt Enable flag). We force an exit from the block
             * and return to QEMU's main loop to re-evaluate pending hardware interrupts
             * (e.g., from the UART).
             */
            tcg_gen_movi_i32(cpu_r[PC_REG], ctx->base.pc_next);
            tcg_gen_exit_tb(NULL, 0);
            break;

        case DISAS_EXIT:
            /*
             * Immediate exit required from the execution loop.
             */
            tcg_gen_exit_tb(NULL, 0);
            break;

        default:
            g_assert_not_reached();
    }
}
/**
 * p16_tr_ops:
 *
 * Translation operations table (Hooks) for the P16.
 * Links our local decoding logic to the global TCG (Tiny Code Generator)
 * engine during the lifecycle of a Translation Block.
 */
static const TranslatorOps p16_tr_ops = {
    .init_disas_context = p16_tr_init_disas_context,
    .tb_start = p16_tr_tb_start,
    .insn_start = p16_tr_insn_start,
    .translate_insn = p16_tr_translate_insn,
    .tb_stop = p16_tr_tb_stop,
};

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
                            int *max_insns, vaddr pc, void *host_pc) {
    DisasContext dc = {};
    translator_loop(cs, tb, max_insns, pc, host_pc, &p16_tr_ops, &dc.base,
                    TCG_TYPE_VA);
}