/* AND - mask_zero */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: mask_zero */
    /* AND 0xFFFF, 0x0000 -> RES=0x0000 N=0 Z=1 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #0
    /* Ensure V=0 */
    MOV R4, #0
    ADD R4, R4, R4
    AND R3, R1, R2
    BNE _fail
    BLT _fail
    MOV R4, #0
    CMP R3, R4
    BNE _fail
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
