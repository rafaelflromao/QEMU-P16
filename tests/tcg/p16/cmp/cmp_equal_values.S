/* CMP - equal_values */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: equal_values */
    /* CMP 0x0005, 0x0005 -> C=1 V=0 N=0 Z=1 */
    MOV R1, #5
    MOV R2, #5
    CMP R1, R2
    BCC _fail
    BNE _fail
    BLT _fail
    MOV R4, #5
    CMP R1, R4
    BNE _fail
    MOV R4, #5
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
