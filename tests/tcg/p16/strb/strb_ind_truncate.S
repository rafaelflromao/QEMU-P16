/* STRB - ind_truncate */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: ind_truncate (BYTE - indirect) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #204
    MOVT R2, #255
    STRB R2, [R1]
    MOV R3, #0
    LDRB R3, [R1]
    MOV R4, #204
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
