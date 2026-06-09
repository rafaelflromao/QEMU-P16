/* BRANCH - LO - not_taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: LO - not_taken */
    /* CMP 0x0005, 0x0003 -> Expected jump: NO */
    MOV R1, #5
    MOV R2, #3
    CMP R1, R2
    BLO _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
