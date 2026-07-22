/* ORR - sign_bit_only */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: sign_bit_only */
    /* ORR 0x8000, 0x8000 -> RES=0x8000 N=1 Z=0 */
    MOV R1, #0
    MOVT R1, #128
    MOV R2, #0
    MOVT R2, #128
    /* Ensure V=0 */
    MOV R4, #0
    ADD R4, R4, R4
    ORR R3, R1, R2
    BEQ _fail
    BGE _fail
    MOV R4, #0
    MOVT R4, #128
    CMP R3, R4
    BNE _fail
    MOV R4, #0
    MOVT R4, #128
    CMP R1, R4
    BNE _fail
    MOV R4, #0
    MOVT R4, #128
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
