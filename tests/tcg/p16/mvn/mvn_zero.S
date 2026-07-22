/* MVN - zero */
.include "macros.inc"
.text
.global _start
_start:
    MOV R1, #0
    MOV R4, #0
    ADD R4, R4, R4
    MVN R3, R1
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
