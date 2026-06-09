/* SUB - imm_fifteen */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: imm_fifteen */
    /* SUB 0x0000, #15 => RES=0xFFF1 C=0 V=0 N=1 Z=0 */
    MOV R1, #0
    SUB R3, R1, #15
    BCS _fail
    BEQ _fail
    BGE _fail
    MOV R4, #241
    MOVT R4, #255
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
