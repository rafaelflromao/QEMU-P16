/* ADD - one_plus_one */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: one_plus_one */
    /* ADD 0x0001, 0x0001 => RES=0x0002 C=0 V=0 N=0 Z=0 */
    MOV R1, #1
    MOV R2, #1
    ADD R3, R1, R2
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #2
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
