/* SHIFT ASR - asr_pos */
.include "macros.inc"
.text
.global _start
_start:
    MOV R1, #8
    /* SETUP V=0 AND CARRY */
    MOV R4, #0
    ADD R4, R4, R4
    ASR R3, R1, #2
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #2
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
