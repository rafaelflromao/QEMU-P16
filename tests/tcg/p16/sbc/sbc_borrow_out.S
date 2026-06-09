/* SBC - borrow_out */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: borrow_out */
    /* SBC 0x0003, 0x0005 => RES=0xFFFE C=0 V=0 N=1 Z=0 */
    MOV R1, #3
    MOV R2, #5
    /* Configure Carry_In */
    MOV R4, #0xFF
    MOVT R4, #0xFF
    MOV R5, #1
    ADD R4, R4, R5
    SBC R3, R1, R2
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #254
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
