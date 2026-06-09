/* SBC - zero_minus_zero */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: zero_minus_zero */
    /* SBC 0x0000, 0x0000 => RES=0xFFFF C=0 V=0 N=1 Z=0 */
    MOV R1, #0
    MOV R2, #0
    /* Configure Carry_In */
    MOV R4, #0
    ADD R4, R4, R4
    SBC R3, R1, R2
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
