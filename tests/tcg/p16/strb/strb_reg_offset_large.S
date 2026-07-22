/* STRB - reg_offset_large */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: reg_offset_large (BYTE - reg_offset) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #238
    MOV R5, #0
    MOVT R5, #2
    STRB R2, [R1, R5]
    MOV R3, #0
    LDRB R3, [R1, R5]
    MOV R4, #238
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
