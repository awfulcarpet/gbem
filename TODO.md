- [ ] finish and pass 100% of sm83 test suite
    - [x] block 0
        - [x] nop

        - [x] ld r16, imm16
        - [x] ld [r16mem], a
        - [x] ld a, [r16mem]
        - [x] ld [imm16], sp

        - [x] inc r16
        - [x] dec r16
        - [x] add hl, r16

        - [x] inc r8
        - [x] dec r8

        - [x] ld r8, imm8

        - [x] rlca
        - [x] rrca
        - [x] rla
        - [x] rra
        - [x] daa
        - [x] cpl
        - [x] scf
        - [x] ccf

        - [x] jr imm8
        - [x] jr cond, imm8
        - [x] stop

    - [x] block 1
        - [x] ld r8, r8
        - [x] halt (ld [hl] [hl])

    - [ ] block 2
        - [x] add a, r8
        - [x] adc a, r8
        - [ ] sub a, r8
        - [ ] sbc a, r8
        - [ ] and a, r8
        - [ ] xor a, r8
        - [ ] or a, r8
        - [ ] cp a, r8

    - [ ] block 3
        - [ ] add a, imm8
        - [ ] adc a, imm8
        - [ ] sub a, imm8
        - [ ] sbc a, imm8
        - [ ] and a, imm8
        - [ ] xor a, imm8
        - [ ] or a, imm8
        - [ ] cp a, imm8

        - [ ] ret cond
        - [ ] ret
        - [ ] reti
        - [ ] jp cond, imm16
        - [ ] jp imm16
        - [ ] jp hl
        - [ ] call cond, imm16
        - [ ] call imm16
        - [ ] rst tgt3

        - [ ] pop r16stk
        - [ ] push r16stk

        - [ ] cb prefixes
            - [ ] rlc r8
            - [ ] rrc r8
            - [ ] rl r8
            - [ ] rr r8
            - [ ] sla r8
            - [ ] sra r8
            - [ ] swap r8
            - [ ] srl r8

            - [ ] bit b3, r8
            - [ ] res b3, r8
            - [ ] set b3, r8

        - [ ] ldh [c], a
        - [ ] ldh [imm8], a
        - [ ] ld [imm16], a
        - [ ] ldh a, [c]
        - [ ] ldh a, [imm8]
        - [ ] ldh a, [imm16]

        - [ ] add sp, imm8
        - [ ] ld hl, sp + imm8
        - [ ] ld sp, hl

        - [x] di
        - [x] ei
