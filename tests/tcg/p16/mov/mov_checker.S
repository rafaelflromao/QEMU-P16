/* MOV - checker */
.include "macros.inc"
.text
.global _start
_start:
    MOV R2, #170
    MOVT R2, #170
    MOV R4, #170
    MOVT R4, #170
    CMP R2, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
