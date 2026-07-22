/* ADD - random */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: random */
    /* ADD 0x1234, 0x5678 => RES=0x68AC C=0 V=0 N=0 Z=0 */
    MOV R1, #52
    MOVT R1, #18
    MOV R2, #120
    MOVT R2, #86
    ADD R3, R1, R2
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #172
    MOVT R4, #104
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
