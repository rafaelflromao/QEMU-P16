/* STRB - reg_offset_5 */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: reg_offset_5 (BYTE - reg_offset) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #153
    MOV R5, #5
    STRB R2, [R1, R5]
    MOV R3, #0
    LDRB R3, [R1, R5]
    MOV R4, #153
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
