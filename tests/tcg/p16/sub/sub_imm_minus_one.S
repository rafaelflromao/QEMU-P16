/* SUB - imm_minus_one */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: imm_minus_one */
    /* SUB 0x0005, #1 => RES=0x0004 C=1 V=0 N=0 Z=0 */
    MOV R1, #5
    SUB R3, R1, #1
    BCC _fail
    BEQ _fail
    BLT _fail
    MOV R4, #4
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
