/* BRANCH - GE - taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: GE - taken */
    /* CMP 0x0005, 0x0003 -> Expected jump: YES */
    MOV R1, #5
    MOV R2, #3
    CMP R1, R2
    BGE _branch_taken
    
    TEST_EXIT_FAIL
    
_branch_taken:
    TEST_EXIT_PASS