/* MOVT - generic */
.include "macros.inc"
.text
.global _start
_start:
    MOV R3, #52
    MOVT R3, #18
    MOVT R3, #0x56
    MOV R4, #52
    MOVT R4, #86
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
