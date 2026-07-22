/* EOR - mask_zero */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: mask_zero */
    /* EOR 0xFFFF, 0x0000 -> RES=0xFFFF N=1 Z=0 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #0
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
