/* CMP - negative_boundary */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: negative_boundary */
    /* CMP 0x8000, 0x0001 -> C=1 V=32768 N=0 Z=0 */
    MOV R1, #0
    MOVT R1, #128
    MOV R2, #1
    CMP R1, R2
    BCC _fail
    BEQ _fail
    BGE _fail
    MOV R4, #0
    MOVT R4, #128
    CMP R1, R4
    BNE _fail
    MOV R4, #1
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
