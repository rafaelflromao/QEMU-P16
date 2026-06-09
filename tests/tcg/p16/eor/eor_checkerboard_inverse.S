/* EOR - checkerboard_inverse */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: checkerboard_inverse */
    /* EOR 0x5555, 0xAAAA -> RES=0xFFFF N=1 Z=0 */
    MOV R1, #85
    MOVT R1, #85
    MOV R2, #170
    MOVT R2, #170
    /* Ensure V=0 */
    MOV R4, #0
    ADD R4, R4, R4
    EOR R3, R1, R2
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #255
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
