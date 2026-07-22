/* BRANCH - B - basic */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: B (Unconditional) */
    B _target
    
    /* If we reach here, the branch was not taken, which is a failure. */
    TEST_EXIT_FAIL

_target:
    TEST_EXIT_PASS