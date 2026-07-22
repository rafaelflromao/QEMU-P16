/* CMP - zero_vs_max */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: zero_vs_max */
    /* CMP 0x0000, 0xFFFF -> C=0 V=0 N=0 Z=0 */
    MOV R1, #0
    MOV R2, #255
    MOVT R2, #255
    CMP R1, R2
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #0
    CMP R1, R4
    BNE _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
