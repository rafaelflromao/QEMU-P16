/* BRANCH - LT - taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: LT - taken */
    /* CMP 0x0000, 0x0001 -> Expected jump: YES */
    MOV R1, #0
    MOV R2, #1
    CMP R1, R2
    BLT _branch_taken
    B _fail
_branch_taken:

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
