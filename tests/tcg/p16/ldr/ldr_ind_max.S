/* LDR - ind_max */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: ind_max (WORD - indirect) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #255
    MOVT R2, #255
    STR R2, [R1]
    MOV R3, #0
    LDR R3, [R1]
    CMP R3, R2
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
