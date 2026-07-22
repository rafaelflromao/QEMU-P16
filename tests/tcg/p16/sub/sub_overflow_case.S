/* SUB - overflow_case */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: overflow_case */
    /* SUB 0x7FFF, 0xFFFF => RES=0x8000 C=0 V=1 N=1 Z=0 */
    MOV R1, #255
    MOVT R1, #127
    MOV R2, #255
    MOVT R2, #255
    SUB R3, R1, R2
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #0
    MOVT R4, #128
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
