/* CMP - positive_boundary */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: positive_boundary */
    /* CMP 0x7FFF, 0x0001 -> C=1 V=0 N=0 Z=0 */
    MOV R1, #255
    MOVT R1, #127
    MOV R2, #1
    CMP R1, R2
    BCC _fail
    BEQ _fail
    BLT _fail
    MOV R4, #255
    MOVT R4, #127
    CMP R1, R4
    BNE _fail
    MOV R4, #1
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
