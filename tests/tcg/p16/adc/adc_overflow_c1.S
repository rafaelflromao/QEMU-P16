/* ADC - overflow_c1 */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: overflow_c1 */
    /* ADC 0x7FFF, 0x0000 => RES=0x8000 C=0 V=1 N=1 Z=0 */
    MOV R1, #255
    MOVT R1, #127
    MOV R2, #0
    /* Configure Carry_In */
    MOV R4, #0xFF
    MOVT R4, #0xFF
    MOV R5, #1
    ADD R4, R4, R5
    ADC R3, R1, R2
    BCS _fail
    BEQ _fail
    BLT _fail
    MOV R4, #0
    MOVT R4, #128
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
