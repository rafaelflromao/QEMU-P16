/* SHIFT RRX - rrx_c1 */
.include "macros.inc"
.text
.global _start
_start:
    MOV R1, #0
    /* SETUP V=0 AND CARRY */
    MOV R4, #0
    ADD R4, R4, R4
    MOV R4, #0xFF
    MOVT R4, #0xFF
    MOV R5, #1
    ADD R4, R4, R5
    RRX R3, R1
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #0
    MOVT R4, #128
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
