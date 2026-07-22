/* LDR - imm_offset_4 */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: imm_offset_4 (WORD - imm_offset) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #85
    MOVT R2, #85
    STR R2, [R1, #4]
    MOV R3, #0
    LDR R3, [R1, #4]
    CMP R3, R2
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
