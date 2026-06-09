/* SHIFT LSL - lsl_carry */
.include "macros.inc"
.text
.global _start
_start:
    MOV R1, #0
    MOVT R1, #128
    /* SETUP V=0 AND CARRY */
    MOV R4, #0
    ADD R4, R4, R4
    LSL R3, R1, #1
    BCC _fail
    BNE _fail
    BLT _fail
    MOV R4, #0
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
