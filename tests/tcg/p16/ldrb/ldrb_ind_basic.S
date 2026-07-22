/* LDRB - ind_basic */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: ind_basic (BYTE - indirect) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #171
    STRB R2, [R1]
    MOV R3, #0
    LDRB R3, [R1]
    MOV R4, #171
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
