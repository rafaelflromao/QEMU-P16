/* CMP - unsigned_greater */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: unsigned_greater */
    /* CMP 0x0005, 0x0003 -> C=1 V=0 N=0 Z=0 */
    MOV R1, #5
    MOV R2, #3
    CMP R1, R2
    BCC _fail
    BEQ _fail
    BLT _fail
    MOV R4, #5
    CMP R1, R4
    BNE _fail
    MOV R4, #3
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
