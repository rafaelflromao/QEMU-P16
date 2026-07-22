/* SUB - imm_equal */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: imm_equal */
    /* SUB 0x0005, #5 => RES=0x0000 C=1 V=0 N=0 Z=1 */
    MOV R1, #5
    SUB R3, R1, #5
    BCC _fail
    BNE _fail
    BLT _fail
    MOV R4, #0
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
