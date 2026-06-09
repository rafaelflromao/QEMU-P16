/* ADD - imm_eight */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: imm_eight */
    /* ADD 0x0000, #8 => RES=0x0008 C=0 V=0 N=0 Z=0 */
    MOV R1, #0
    ADD R3, R1, #8
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #8
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
