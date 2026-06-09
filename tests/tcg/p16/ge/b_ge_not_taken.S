/* BRANCH - GE - not_taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: GE - not_taken */
    /* CMP 0x0000, 0x0001 -> Expected jump: NO */
    MOV R1, #0
    MOV R2, #1
    CMP R1, R2
    BGE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
