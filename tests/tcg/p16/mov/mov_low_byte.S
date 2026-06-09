/* MOV - low_byte */
.include "macros.inc"
.text
.global _start
_start:
    MOV R2, #255
    MOV R4, #255
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
