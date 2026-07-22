/* BRANCH - BL - basic */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: BL (Branch with Link) */
    MOV R14, #0
    BL _subroutine
    
    TEST_EXIT_FAIL
    
_subroutine:
    MOV R1, #0
    CMP R1, R14
    BEQ _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL