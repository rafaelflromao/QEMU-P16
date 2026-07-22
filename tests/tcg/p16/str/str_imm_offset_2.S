/* STR - imm_offset_2 */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: imm_offset_2 (WORD - imm_offset) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #170
    MOVT R2, #170
    STR R2, [R1, #2]
    MOV R3, #0
    LDR R3, [R1, #2]
    CMP R3, R2
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
