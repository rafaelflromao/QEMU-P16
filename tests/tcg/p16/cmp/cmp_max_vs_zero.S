/* CMP - max_vs_zero */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: max_vs_zero */
    /* CMP 0xFFFF, 0x0000 -> C=1 V=0 N=1 Z=0 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #0
    CMP R1, R2
    BCC _fail
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R1, R4
    BNE _fail
    MOV R4, #0
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
