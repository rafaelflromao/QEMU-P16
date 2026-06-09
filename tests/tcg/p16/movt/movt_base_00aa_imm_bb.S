/* MOVT - base_00aa_imm_bb */
.include "macros.inc"
.text
.global _start
_start:
    MOV R3, #170
    MOVT R3, #0xBB
    MOV R4, #170
    MOVT R4, #187
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS
_fail:
    TEST_EXIT_FAIL
