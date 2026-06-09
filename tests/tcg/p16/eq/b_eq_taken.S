/* BRANCH - EQ - taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: EQ - taken */
    /* CMP 0x0005, 0x0005 -> Expected jump: YES */
    MOV R1, #5
    MOV R2, #5
    CMP R1, R2
    BEQ _branch_taken
    B _fail
_branch_taken:

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
