/* CMP - unsigned_less */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: unsigned_less */
    /* CMP 0x0003, 0x0005 -> C=0 V=0 N=1 Z=0 */
    MOV R1, #3
    MOV R2, #5
    CMP R1, R2
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #3
    CMP R1, R4
    BNE _fail
    MOV R4, #5
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
