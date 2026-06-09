/* ADD - negative_plus_minus1 */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: negative_plus_minus1 */
    /* ADD 0x8000, 0xFFFF => RES=0x7FFF C=1 V=1 N=0 Z=0 */
    MOV R1, #0
    MOVT R1, #128
    MOV R2, #255
    MOVT R2, #255
    ADD R3, R1, R2
    BCC _fail
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #127
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
