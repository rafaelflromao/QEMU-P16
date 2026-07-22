/* STACK TEST - push_pop */
.include "macros.inc"
.text
.global _start
_start:

    /* Simple PUSH/POP */

    MOV SP, #0x00
    MOVT SP, #0x20

    MOV R1, #0xAA
    MOVT R1, #0xAA

    MOV R2, #0

    PUSH R1
    POP R2

    CMP R1, R2
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
