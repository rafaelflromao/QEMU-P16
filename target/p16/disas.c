/*
 * P16 disassembler
 *
 * Copyright (c) 2026 Rafael Romão <a48863@alunos.isel.pt>
 * Copyright (c) 2019-2020 Richard Henderson <rth@twiddle.net>
 * Copyright (c) 2019-2020 Michael Rolnik <mrolnik@gmail.com>
 *
 * This work is based on the AVR disassembler (target/avr/disas.c).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/bitops.h"
#include "disas/dis-asm.h"
#include "cpu.h"

typedef struct {
    disassemble_info *info;
} DisasContext;

static int mult_2(DisasContext *ctx, int value) {
    return value << 1;
}

/* Include the auto-generated decoder.  */
static bool decode_insn(DisasContext *ctx, uint16_t insn);

#include "decode-insn.c.inc"

#define output(mnemonic, format, ...) \
    (pctx->info->fprintf_func(pctx->info->stream, "%-9s " format, \
                              mnemonic, ##__VA_ARGS__))

int print_insn_p16(bfd_vma addr, disassemble_info *info) {
    DisasContext ctx = {info};
    DisasContext *pctx = &ctx;
    bfd_byte buffer[2];
    uint16_t insn;
    int status;

    ctx.info = info;

    status = info->read_memory_func(addr, buffer, 2, info);
    if (status != 0) {
        info->memory_error_func(status, addr, info);
        return -1;
    }

    insn = bfd_getl16(buffer);

    if (!decode_insn(&ctx, insn)) {
        output(".dw", "0x%02x%02x", buffer[1], buffer[0]);
    }

    return 2;
}


#define INSN(opcode, format, ...)                                       \
static bool trans_##opcode(DisasContext *pctx, arg_##opcode * a)        \
{                                                                       \
    output(#opcode, format, ##__VA_ARGS__);                             \
    return true;                                                        \
}

#define INSN_MNEMONIC(opcode, mnemonic, format, ...)                    \
static bool trans_##opcode(DisasContext *pctx, arg_##opcode * a)        \
{                                                                       \
    output(mnemonic, format, ##__VA_ARGS__);                            \
    return true;                                                        \
}


/*
 * Arithmetic Instructions
 */
INSN(ADC, "r%d, r%d, r%d", a->rd, a->rn, a->rm)
INSN(ADD, "r%d, r%d, r%d", a->rd, a->rn, a->rm)
INSN_MNEMONIC(ADD_I, "ADD", "r%d, r%d, #%d", a->rd, a->rn, a->imm4)
INSN(AND, "r%d, r%d, r%d", a->rd, a->rn, a->rm)
INSN(ASR, "r%d, r%d, #%d", a->rd, a->rn, a->imm4)
INSN(CMP, "r%d, r%d", a->rn, a->rm)
INSN(EOR, "r%d, r%d, r%d", a->rd, a->rn, a->rm)
INSN(LSL, "r%d, r%d, #%d", a->rd, a->rn, a->imm4)
INSN(LSR, "r%d, r%d, #%d", a->rd, a->rn, a->imm4)
INSN(ORR, "r%d, r%d, r%d", a->rd, a->rn, a->rm)
INSN(ROR, "r%d, r%d, #%d", a->rd, a->rn, a->imm4)
INSN(RRX, "r%d, r%d", a->rd, a->rn)
INSN(SUB, "r%d, r%d, r%d", a->rd, a->rn, a->rm)
INSN_MNEMONIC(SUB_I, "SUB", "r%d, r%d, #%d", a->rd, a->rn, a->imm4)
INSN(SBC, "r%d, r%d, r%d", a->rd, a->rn, a->rm)

/*
 * Branch Instructions
 */

INSN(B, ".%+d", a->offset+2)
INSN(BLO, ".%+d", a->offset+2)
INSN(BHS, ".%+d", a->offset+2)
INSN(BGE, ".%+d", a->offset+2)
INSN(BL, ".%+d", a->offset+2)
INSN(BLT, ".%+d", a->offset+2)
INSN(BZC, ".%+d", a->offset+2)
INSN(BZS, ".%+d", a->offset+2)

/*
 * Data Transfer Instructions
 */
INSN_MNEMONIC(LDR_L, "LDR", "r%d, .%+d", a->rd, a->imm7)
INSN_MNEMONIC(LDR_C, "LDR", "r%d, [r%d, #%d]", a->rd, a->rn, a->imm4)
INSN_MNEMONIC(LDR_R, "LDR", "r%d, [r%d, r%d]", a->rd, a->rn, a->rm)
INSN_MNEMONIC(LDRB_C, "LDRB", "r%d, [r%d, #%d]", a->rd, a->rn, a->imm3)
INSN_MNEMONIC(LDRB_R, "LDRB", "r%d, [r%d, r%d]", a->rd, a->rn, a->rm)
INSN(POP, "r%d", a->rd)
INSN(PUSH, "r%d", a->rd)
INSN_MNEMONIC(STR_C, "STR", "r%d, [r%d, #%d]", a->rd, a->rn, a->imm4)
INSN_MNEMONIC(STR_R, "STR", "r%d, [r%d, r%d]", a->rd, a->rn, a->rm)
INSN_MNEMONIC(STRB_C, "STRB", "r%d, [r%d, #%d]", a->rd, a->rn, a->imm3)
INSN_MNEMONIC(STRB_R, "STRB", "r%d, [r%d, r%d]", a->rd, a->rn, a->rm)

INSN_MNEMONIC(MOV_I, "MOV", "r%d, #%d", a->rd, a->imm8)
INSN(MOV, "r%d, r%d", a->rd, a->rm)
INSN(MOVS, "PC, LR")
INSN(MOVT, "r%d, #%d", a->rd, a->imm8)
INSN_MNEMONIC(MRS_C, "MRS", "r%d, CPSR", a->rd)
INSN_MNEMONIC(MRS_S, "MRS", "r%d, SPSR", a->rd)
INSN_MNEMONIC(MSR_C, "MSR", "CPSR, r%d", a->rm)
INSN_MNEMONIC(MSR_S, "MSR", "SPSR, r%d", a->rm)
INSN(MVN, "r%d, %d", a->rd, a->rm)