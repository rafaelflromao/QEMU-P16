.macro TEST_EXIT_PASS
    MOV r0, #0xAA
    MOV r1, #0x00
    MOVT r1, #0xFF
    STR r0, [r1]
    B .
.endm

.macro TEST_EXIT_FAIL
    MOV r0, #0xDE
    MOV r1, #0x00
    MOVT r1, #0xFF
    STR r0, [r1]
    B .
.endm
