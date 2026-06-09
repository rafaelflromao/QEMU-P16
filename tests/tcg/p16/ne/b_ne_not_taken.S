/* BRANCH - NE - not_taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: NE - not_taken */
    /* CMP 0x0005, 0x0005 -> Expected jump: NO */
    MOV R1, #5
    MOV R2, #5
    CMP R1, R2
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
