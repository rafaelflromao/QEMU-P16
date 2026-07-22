/* MOV - zero */
.include "macros.inc"
.text
.global _start
_start:
    MOV R2, #0
    MOV R4, #0
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
