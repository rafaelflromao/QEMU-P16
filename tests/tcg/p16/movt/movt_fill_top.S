/* MOVT - fill_top */
.include "macros.inc"
.text
.global _start
_start:
    MOV R3, #0
    MOVT R3, #0xFF
    MOV R4, #0
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
