/* BRANCH - LO - taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: LO - taken */
    /* CMP 0x0003, 0x0005 -> Expected jump: YES */
    MOV R1, #3
    MOV R2, #5
    CMP R1, R2
    BLO _branch_taken
    
    TEST_EXIT_FAIL
    
_branch_taken:
    TEST_EXIT_PASS