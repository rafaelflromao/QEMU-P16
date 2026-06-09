/* CMP - all_ones */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: all_ones */
    /* CMP 0xFFFF, 0xFFFF -> C=1 V=0 N=0 Z=1 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #255
    MOVT R2, #255
    CMP R1, R2
    BCC _fail
    BNE _fail
    BLT _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R1, R4
    BNE _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
