/* STR - imm_offset_14 */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE: imm_offset_14 (WORD - imm_offset) */
    MOV R1, #0
    MOVT R1, #32
    MOV R2, #254
    MOVT R2, #202
    STR R2, [R1, #14]
    MOV R3, #0
    LDR R3, [R1, #14]
    CMP R3, R2
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
