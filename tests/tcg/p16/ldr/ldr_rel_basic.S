/* LDR - rel_basic */
.include "macros.inc"
.text
.global _start
_start:

    /* RELATIVE TEST: rel_basic (LDR only) */
    LDR R3, _data_rel_basic
    MOV R4, #118
    MOVT R4, #152
    CMP R3, R4
    BNE _fail
    
    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL

    /* Placing data at the end isolates it from the execution flow */
    .align 2
_data_rel_basic: .word 0x9876