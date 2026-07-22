/* ADD - all_ones */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: all_ones */
    /* ADD 0xFFFF, 0xFFFF => RES=0xFFFE C=1 V=0 N=1 Z=0 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #255
    MOVT R2, #255
    ADD R3, R1, R2
    BCC _fail
    BEQ _fail
    BGE _fail
    MOV R4, #254
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
