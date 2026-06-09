/* SUB - imm_borrow */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: imm_borrow */
    /* SUB 0x0000, #1 => RES=0xFFFF C=0 V=0 N=1 Z=0 */
    MOV R1, #0
    SUB R3, R1, #1
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #255
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
