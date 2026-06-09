/* LDR - rel_basic */
.include "macros.inc"
.text
.global _start
_start:

    /* TESTE RELATIVO: rel_basic (Apenas LDR) */
    LDR R3, _data_rel_basic
    MOV R4, #118
    MOVT R4, #152
    CMP R3, R4
    BNE _fail
    B _skip_rel_basic
    .align 2
_data_rel_basic: .word 0x9876
_skip_rel_basic:

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
