/* AND - checkerboard_inverse */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: checkerboard_inverse */
    /* AND 0x5555, 0xAAAA -> RES=0x0000 N=0 Z=1 */
    MOV R1, #85
    MOVT R1, #85
    MOV R2, #170
    MOVT R2, #170
    /* Ensure V=0 */
    MOV R4, #0
    ADD R4, R4, R4
    AND R3, R1, R2
    BNE _fail
    BLT _fail
    MOV R4, #0
    CMP R3, R4
    BNE _fail
    MOV R4, #85
    MOVT R4, #85
    CMP R1, R4
    BNE _fail
    MOV R4, #170
    MOVT R4, #170
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
