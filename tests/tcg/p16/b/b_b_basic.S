/* BRANCH - B - basic */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: B (Unconditional) */
    B _target
    B _fail
_target:

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
