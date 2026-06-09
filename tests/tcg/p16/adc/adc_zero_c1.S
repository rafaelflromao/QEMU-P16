/* ADC - zero_c1 */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: zero_c1 */
    /* ADC 0x0000, 0x0000 => RES=0x0001 C=0 V=0 N=0 Z=0 */
    MOV R1, #0
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
    MOV R4, #1
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
