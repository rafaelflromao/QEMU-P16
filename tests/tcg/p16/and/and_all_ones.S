/* AND - all_ones */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: all_ones */
    /* AND 0xFFFF, 0xFFFF -> RES=0xFFFF N=1 Z=0 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #255
    MOVT R2, #255
    /* Ensure V=0 */
    MOV R4, #0
    ADD R4, R4, R4
    AND R3, R1, R2
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
    MOV R4, #255
    MOVT R4, #255
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
