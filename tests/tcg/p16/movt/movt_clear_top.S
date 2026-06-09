/* MOVT - clear_top */
.include "macros.inc"
.text
.global _start
_start:
    MOV R3, #255
    MOVT R3, #255
    MOVT R3, #0x00
    MOV R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
