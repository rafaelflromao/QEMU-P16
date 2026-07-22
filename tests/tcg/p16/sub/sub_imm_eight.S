/* SUB - imm_eight */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: imm_eight */
    /* SUB 0x0000, #8 => RES=0xFFF8 C=0 V=0 N=1 Z=0 */
    MOV R1, #0
    SUB R3, R1, #8
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #248
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
