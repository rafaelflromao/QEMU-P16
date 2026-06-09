/* ADC - carry_out_c1 */
.include "macros.inc"
.text
.global _start
_start:

    /* TEST: carry_out_c1 */
    /* ADC 0xFFFF, 0x0000 => RES=0x0000 C=1 V=0 N=0 Z=1 */
    MOV R1, #255
    MOVT R1, #255
    MOV R2, #0
    /* Configure Carry_In */
    MOV R4, #0xFF
    MOVT R4, #0xFF
    MOV R5, #1
    ADD R4, R4, R5
    ADC R3, R1, R2
    BCC _fail
    BNE _fail
    BLT _fail
    MOV R4, #0
    CMP R3, R4
    BNE _fail

    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL
