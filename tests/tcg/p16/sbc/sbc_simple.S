/* SBC - simple */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: simple */
    /* SBC 0x0005, 0x0003 => RES=0x0002 C=1 V=0 N=0 Z=0 */
    MOV R1, #5
    MOV R2, #3
    /* Configure Carry_In */
    MOV R4, #0xFF
    MOVT R4, #0xFF
    MOV R5, #1
    ADD R4, R4, R5
    SBC R3, R1, R2
    BCC _fail
    BEQ _fail
    BLT _fail
    MOV R4, #2
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
