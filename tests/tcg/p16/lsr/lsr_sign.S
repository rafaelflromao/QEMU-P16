/* SHIFT LSR - lsr_sign */
.include "macros.inc"
.text
.global _start
_start:
    MOV R1, #0
    MOVT R1, #128
    /* SETUP V=0 AND CARRY */
    MOV R4, #0
    ADD R4, R4, R4
    LSR R3, R1, #4
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #0
    MOVT R4, #8
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
