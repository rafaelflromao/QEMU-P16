/* ADD - carry_out */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: carry_out */
    /* ADD 0xFFFF, 0x0001 => RES=0x0000 C=1 V=0 N=0 Z=1 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #1
    ADD R3, R1, R2
    BCC _fail
    BNE _fail
    BLT _fail
    MOV R4, #0
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
