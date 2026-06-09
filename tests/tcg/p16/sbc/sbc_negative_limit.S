/* SBC - negative_limit */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: negative_limit */
    /* SBC 0x8000, 0x0000 => RES=0x7FFF C=1 V=1 N=0 Z=0 */
    MOV R1, #0
    MOVT R1, #128
    MOV R2, #0
    /* Configure Carry_In */
    MOV R4, #0
    ADD R4, R4, R4
    SBC R3, R1, R2
    BCC _fail
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #127
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
