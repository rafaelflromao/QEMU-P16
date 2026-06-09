/* STR - reg_offset_large */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: reg_offset_large (WORD - reg_offset) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #52
    MOVT R2, #18
    MOV R5, #0
    MOVT R5, #1
    STR R2, [R1, R5]
    MOV R3, #0
    LDR R3, [R1, R5]
    CMP R3, R2
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
