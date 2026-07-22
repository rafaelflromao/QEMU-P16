/* SHIFT ASR - asr_all1 */
.include "macros.inc"
.text
.global _start
_start:
    MOV R1, #255
    MOVT R1, #255
    /* SETUP V=0 AND CARRY */
    MOV R4, #0
    ADD R4, R4, R4
    ASR R3, R1, #1
    BCC _fail
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
