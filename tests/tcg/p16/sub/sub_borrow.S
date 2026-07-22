/* SUB - borrow */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: borrow */
    /* SUB 0x0003, 0x0005 => RES=0xFFFE C=0 V=0 N=1 Z=0 */
    MOV R1, #3
    MOV R2, #5
    SUB R3, R1, R2
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #254
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
