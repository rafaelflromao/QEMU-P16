/* BRANCH - HS - taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: HS - taken */
    /* CMP 0x0005, 0x0003 -> Expected jump: YES */
    MOV R1, #5
    MOV R2, #3
    CMP R1, R2
    BHS _branch_taken
    B _fail
_branch_taken:

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
