/* STACK TEST - lifo_sp_integrity */
.include "macros.inc"
.text
.global _start
_start:

    /* LIFO + SP integrity */

    MOV SP, #0x00
    MOVT SP, #0x20

    MOV R1, #0x11
    MOVT R1, #0x11

    MOV R2, #0x22
    MOVT R2, #0x22

    MOV R3, #0x33
    MOVT R3, #0x33

    PUSH R1
    PUSH R2
    PUSH R3

    POP R4
    CMP R4, R3
    BNE _fail

    POP R4
    CMP R4, R2
    BNE _fail

    POP R4
    CMP R4, R1
    BNE _fail

    MOV R5, #0x00
    MOVT R5, #0x20
    CMP R5, SP
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
