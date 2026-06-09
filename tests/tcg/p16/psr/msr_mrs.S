/* test_msr_mrs.S */
/* Validate writing (MSR) and reading (MRS) of the Status Register */

.include "macros.inc"
.text
.global _start
_start:
    /* ARRANGE: Prepare a value with bits 0 to 5 all set to '1' (0x3F) */
    MOV R1, #0x3F
    MOV R2, #0x00   /* Clear R2 to receive the result */

    /* ACT Part 1: Write to the Status Register */
    msr cpsr, r1

    /* ACT Part 2: Read from the Status Register back into R2 */
    mrs r2, cpsr

    /* ASSERT: Check whether the processor stored and returned 0x3F */
    /* Note: If your architecture has reserved bits that force zeros,
       you may need to adjust 0x3F to the exact expected value. */
    MOV R3, #0x3F
    CMP R2, R3
    BNE _fail

    /* BONUS: Mask test (Ensure bits above bit 5 are ignored!) */
    MOV R1, #0xFF   /* Try to write bits 0 through 7 */
    msr cpsr, R1
    mrs r2, cpsr
    CMP R2, R3      /* The result should still be 0x3F – bits 6 and 7 must be ignored! */
    BNE _fail

_pass:
    TEST_EXIT_PASS

_fail:
    TEST_EXIT_FAIL