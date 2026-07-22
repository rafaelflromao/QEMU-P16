/* LDRB - imm_offset_3 */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: imm_offset_3 (BYTE - imm_offset) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #51
    STRB R2, [R1, #3]
    MOV R3, #0
    LDRB R3, [R1, #3]
    MOV R4, #51
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
