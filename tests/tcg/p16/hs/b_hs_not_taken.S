/* BRANCH - HS - not_taken */
.include "macros.inc"
.text
.global _start
_start:

    /* BRANCH TEST: HS - not_taken */
    /* CMP 0x0003, 0x0005 -> Expected jump: NO */
    MOV R1, #3
    MOV R2, #5
    CMP R1, R2
    BHS _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
