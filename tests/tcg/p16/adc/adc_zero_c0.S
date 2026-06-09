/* ADC - zero_c0 */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: zero_c0 */
    /* ADC 0x0000, 0x0000 => RES=0x0000 C=0 V=0 N=0 Z=1 */
    MOV R1, #0
    MOV R2, #0
    /* Configure Carry_In */
    MOV R4, #0
    ADD R4, R4, R4
    ADC R3, R1, R2
    BCS _fail
    BNE _fail
    BLT _fail
    MOV R4, #0
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
