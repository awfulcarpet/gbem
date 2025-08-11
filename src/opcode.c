#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void
unimlemented_opcode(uint8_t opcode) {
	fprintf(stderr, "unimplemented opcode: %02x\n", opcode);
	exit(1);
}

static const char *
get_prefix(uint8_t opcode)
{
	if ((opcode & 0b11111000) == 0) {
		return "RLC r8";
	}

	if ((opcode & 0b11111000) == 0b1000) {
		return "RRC R8";
	}

	if ((opcode & 0b11111000) == 0b10000) {
		return "RL R8";
	}

	if ((opcode & 0b11111000) == 0b11000) {
		return "RR R8";
	}

	if ((opcode & 0b11111000) == 0b100000) {
		return "SLA R8";
	}

	if ((opcode & 0b11111000) == 0b101000) {
		return "SRA R8";
	}

	if ((opcode & 0b11111000) == 0b110000) {
		return "SWAP R8";
	}

	if ((opcode & 0b11111000) == 0b111000) {
		return "SRL R8";
	}

	if (opcode >> 6 == 1) {
		return "BIT B3 R8";
	}

	if (opcode >> 6 == 0b10) {
		return "RES B3 R8";
	}

	if (opcode >> 6 == 0b11) {
		return "SET B3 R8";
	}

	unimlemented_opcode(opcode);

	return "";
}

char *
get_mnemonic(uint8_t *opcode)
{
	char *buf = calloc(16 + 1, sizeof(char));
    switch (*opcode) {
	case 0x00: {  const char *tmp = "NOP"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x01: {  const char *tmp = "LD BC, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x02: {  const char *tmp = "LD [BC], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x03: {  const char *tmp = "INC BC"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x04: {  const char *tmp = "INC B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x05: {  const char *tmp = "DEC B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x06: {  const char *tmp = "LD B, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x07: {  const char *tmp = "RLCA"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x08: {  const char *tmp = "LD [imm16], SP"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x09: {  const char *tmp = "ADD HL, BC"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x0a: {  const char *tmp = "LD A, [BC]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x0b: {  const char *tmp = "DEC BC"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x0c: {  const char *tmp = "INC C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x0d: {  const char *tmp = "DEC C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x0e: {  const char *tmp = "LD C, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x0f: {  const char *tmp = "RRCA"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x10: {  const char *tmp = "STOP imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x11: {  const char *tmp = "LD DE, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x12: {  const char *tmp = "LD [DE], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x13: {  const char *tmp = "INC DE"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x14: {  const char *tmp = "INC D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x15: {  const char *tmp = "DEC D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x16: {  const char *tmp = "LD D, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x17: {  const char *tmp = "RLA"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x18: {  const char *tmp = "JR imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x19: {  const char *tmp = "ADD HL, DE"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x1a: {  const char *tmp = "LD A, [DE]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x1b: {  const char *tmp = "DEC DE"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x1c: {  const char *tmp = "INC E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x1d: {  const char *tmp = "DEC E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x1e: {  const char *tmp = "LD E, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x1f: {  const char *tmp = "RRA"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x20: {  const char *tmp = "JR NZ, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x21: {  const char *tmp = "LD HL, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x22: {  const char *tmp = "LD [HL+], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x23: {  const char *tmp = "INC HL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x24: {  const char *tmp = "INC H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x25: {  const char *tmp = "DEC H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x26: {  const char *tmp = "LD H, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x27: {  const char *tmp = "DAA"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x28: {  const char *tmp = "JR Z, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x29: {  const char *tmp = "ADD HL, HL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x2a: {  const char *tmp = "LD A, [HL+]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x2b: {  const char *tmp = "DEC HL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x2c: {  const char *tmp = "INC L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x2d: {  const char *tmp = "DEC L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x2e: {  const char *tmp = "LD L, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x2f: {  const char *tmp = "CPL"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x30: {  const char *tmp = "JR NC, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x31: {  const char *tmp = "LD SP, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x32: {  const char *tmp = "LD [HL-], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x33: {  const char *tmp = "INC SP"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x34: {  const char *tmp = "INC [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x35: {  const char *tmp = "DEC [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x36: {  const char *tmp = "LD [HL], imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x37: {  const char *tmp = "SCF"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x38: {  const char *tmp = "JR C, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x39: {  const char *tmp = "ADD HL, SP"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x3a: {  const char *tmp = "LD A, [HL-]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x3b: {  const char *tmp = "DEC SP"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x3c: {  const char *tmp = "INC A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x3d: {  const char *tmp = "DEC A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x3e: {  const char *tmp = "LD A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x3f: {  const char *tmp = "CCF"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x40: {  const char *tmp = "LD B, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x41: {  const char *tmp = "LD B, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x42: {  const char *tmp = "LD B, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x43: {  const char *tmp = "LD B, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x44: {  const char *tmp = "LD B, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x45: {  const char *tmp = "LD B, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x46: {  const char *tmp = "LD B, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x47: {  const char *tmp = "LD B, A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x48: {  const char *tmp = "LD C, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x49: {  const char *tmp = "LD C, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x4a: {  const char *tmp = "LD C, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x4b: {  const char *tmp = "LD C, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x4c: {  const char *tmp = "LD C, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x4d: {  const char *tmp = "LD C, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x4e: {  const char *tmp = "LD C, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x4f: {  const char *tmp = "LD C, A"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x50: {  const char *tmp = "LD D, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x51: {  const char *tmp = "LD D, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x52: {  const char *tmp = "LD D, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x53: {  const char *tmp = "LD D, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x54: {  const char *tmp = "LD D, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x55: {  const char *tmp = "LD D, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x56: {  const char *tmp = "LD D, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x57: {  const char *tmp = "LD D, A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x58: {  const char *tmp = "LD E, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x59: {  const char *tmp = "LD E, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x5a: {  const char *tmp = "LD E, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x5b: {  const char *tmp = "LD E, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x5c: {  const char *tmp = "LD E, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x5d: {  const char *tmp = "LD E, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x5e: {  const char *tmp = "LD E, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x5f: {  const char *tmp = "LD E, A"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x60: {  const char *tmp = "LD H, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x61: {  const char *tmp = "LD H, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x62: {  const char *tmp = "LD H, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x63: {  const char *tmp = "LD H, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x64: {  const char *tmp = "LD H, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x65: {  const char *tmp = "LD H, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x66: {  const char *tmp = "LD H, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x67: {  const char *tmp = "LD H, A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x68: {  const char *tmp = "LD L, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x69: {  const char *tmp = "LD L, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x6a: {  const char *tmp = "LD L, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x6b: {  const char *tmp = "LD L, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x6c: {  const char *tmp = "LD L, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x6d: {  const char *tmp = "LD L, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x6e: {  const char *tmp = "LD L, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x6f: {  const char *tmp = "LD L, A"; memcpy(buf, tmp, strlen(tmp)); break; }


	case 0x70: {  const char *tmp = "LD [HL], B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x71: {  const char *tmp = "LD [HL], C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x72: {  const char *tmp = "LD [HL], D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x73: {  const char *tmp = "LD [HL], E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x74: {  const char *tmp = "LD [HL], H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x75: {  const char *tmp = "LD [HL], L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x76: {  const char *tmp = "HALT"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x77: {  const char *tmp = "LD [HL], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x78: {  const char *tmp = "LD A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x79: {  const char *tmp = "LD A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x7a: {  const char *tmp = "LD A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x7b: {  const char *tmp = "LD A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x7c: {  const char *tmp = "LD A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x7d: {  const char *tmp = "LD A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x7e: {  const char *tmp = "LD A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x7f: {  const char *tmp = "LD A, A"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x80: {  const char *tmp = "ADD A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x81: {  const char *tmp = "ADD A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x82: {  const char *tmp = "ADD A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x83: {  const char *tmp = "ADD A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x84: {  const char *tmp = "ADD A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x85: {  const char *tmp = "ADD A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x86: {  const char *tmp = "ADD A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x87: {  const char *tmp = "ADD A, A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x88: {  const char *tmp = "ADC A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x89: {  const char *tmp = "ADC A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x8a: {  const char *tmp = "ADC A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x8b: {  const char *tmp = "ADC A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x8c: {  const char *tmp = "ADC A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x8d: {  const char *tmp = "ADC A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x8e: {  const char *tmp = "ADC A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x8f: {  const char *tmp = "ADC A, A"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0x90: {  const char *tmp = "SUB A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x91: {  const char *tmp = "SUB A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x92: {  const char *tmp = "SUB A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x93: {  const char *tmp = "SUB A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x94: {  const char *tmp = "SUB A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x95: {  const char *tmp = "SUB A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x96: {  const char *tmp = "SUB A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x97: {  const char *tmp = "SUB A, A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x98: {  const char *tmp = "SBC A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x99: {  const char *tmp = "SBC A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x9a: {  const char *tmp = "SBC A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x9b: {  const char *tmp = "SBC A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x9c: {  const char *tmp = "SBC A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x9d: {  const char *tmp = "SBC A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x9e: {  const char *tmp = "SBC A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0x9f: {  const char *tmp = "SBC A, A"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0xa0: {  const char *tmp = "AND A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa1: {  const char *tmp = "AND A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa2: {  const char *tmp = "AND A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa3: {  const char *tmp = "AND A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa4: {  const char *tmp = "AND A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa5: {  const char *tmp = "AND A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa6: {  const char *tmp = "AND A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa7: {  const char *tmp = "AND A, A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa8: {  const char *tmp = "XOR A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xa9: {  const char *tmp = "XOR A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xaa: {  const char *tmp = "XOR A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xab: {  const char *tmp = "XOR A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xac: {  const char *tmp = "XOR A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xad: {  const char *tmp = "XOR A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xae: {  const char *tmp = "XOR A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xaf: {  const char *tmp = "XOR A, A"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0xb0: {  const char *tmp = "OR A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb1: {  const char *tmp = "OR A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb2: {  const char *tmp = "OR A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb3: {  const char *tmp = "OR A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb4: {  const char *tmp = "OR A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb5: {  const char *tmp = "OR A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb6: {  const char *tmp = "OR A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb7: {  const char *tmp = "OR A, A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb8: {  const char *tmp = "CP A, B"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xb9: {  const char *tmp = "CP A, C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xba: {  const char *tmp = "CP A, D"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xbb: {  const char *tmp = "CP A, E"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xbc: {  const char *tmp = "CP A, H"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xbd: {  const char *tmp = "CP A, L"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xbe: {  const char *tmp = "CP A, [HL]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xbf: {  const char *tmp = "CP A, A"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0xc0: {  const char *tmp = "RET NZ"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc1: {  const char *tmp = "POP BC"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc2: {  const char *tmp = "JP NZ, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc3: {  const char *tmp = "JP imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc4: {  const char *tmp = "CALL NZ, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc5: {  const char *tmp = "PUSH BC"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc6: {  const char *tmp = "ADD A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc7: {  const char *tmp = "RST $00"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc8: {  const char *tmp = "RET Z"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xc9: {  const char *tmp = "RET"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xca: {  const char *tmp = "JP Z"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xcb: {  const char *tmp = get_prefix(*(opcode+1)); memcpy(buf, tmp, strlen(tmp)); break; } // TODO: implement
	case 0xcc: {  const char *tmp = "CALL Z, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xcd: {  const char *tmp = "CALL imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xce: {  const char *tmp = "ADC A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xcf: {  const char *tmp = "RST $08"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0xd0: {  const char *tmp = "RET NC"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd1: {  const char *tmp = "POP DE"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd2: {  const char *tmp = "JP NC, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd3: {  const char *tmp = "0xd3 ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd4: {  const char *tmp = "CALL NC, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd5: {  const char *tmp = "PUSH DE"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd6: {  const char *tmp = "SUB A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd7: {  const char *tmp = "RST $10"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd8: {  const char *tmp = "RET C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xd9: {  const char *tmp = "RETI"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xda: {  const char *tmp = "JP C"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xdb: {  const char *tmp = "0xdb ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xdc: {  const char *tmp = "CALL C, imm16"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xdd: {  const char *tmp = "0xdd ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xde: {  const char *tmp = "SBC A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xdf: {  const char *tmp = "RST $18"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0xe0: {  const char *tmp = "LDH [imm8], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe1: {  const char *tmp = "POP HL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe2: {  const char *tmp = "LDH [C], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe3: {  const char *tmp = "0xe3 ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe4: {  const char *tmp = "0xe4 ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe5: {  const char *tmp = "PUSH HL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe6: {  const char *tmp = "AND A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe7: {  const char *tmp = "RST $20"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe8: {  const char *tmp = "ADD SP, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xe9: {  const char *tmp = "JP HL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xea: {  const char *tmp = "LD [imm16], A"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xeb: {  const char *tmp = "0xeb ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xec: {  const char *tmp = "0xec ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xed: {  const char *tmp = "0xed ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xee: {  const char *tmp = "XOR A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xef: {  const char *tmp = "RST $28"; memcpy(buf, tmp, strlen(tmp)); break; }

	case 0xf0: {  const char *tmp = "LDH A, [imm8]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf1: {  const char *tmp = "POP AF"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf2: {  const char *tmp = "LDH A, [C]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf3: {  const char *tmp = "DI"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf4: {  const char *tmp = "0xf4 ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf5: {  const char *tmp = "PUSH AF"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf6: {  const char *tmp = "OR A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf7: {  const char *tmp = "RST $30"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xf8: {  const char *tmp = "LD HL, SP + imm8"; memcpy(buf, tmp, strlen(tmp)); break; } // Signed
	case 0xf9: {  const char *tmp = "LD SP, HL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xfa: {  const char *tmp = "LD A, [imm16]"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xfb: {  const char *tmp = "EI"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xfc: {  const char *tmp = "0xfc ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xfd: {  const char *tmp = "0xfd ILLEGAL"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xfe: {  const char *tmp = "CP A, imm8"; memcpy(buf, tmp, strlen(tmp)); break; }
	case 0xff: {  const char *tmp = "RST $38"; memcpy(buf, tmp, strlen(tmp)); break; }

	default:
	    unimlemented_opcode(*opcode);
	    break;
    }
	return buf;
}

void
print_mnemonic(uint8_t *opcode) {
    printf("->%02x ", *opcode);
    switch (*opcode) {
	case 0x00: printf("NOP"); break;
	case 0x01: printf("LD BC, $%02x%02x", opcode[2], opcode[1]); break;
	case 0x02: printf("LD [BC], A"); break;
	case 0x03: printf("INC BC"); break;
	case 0x04: printf("INC B"); break;
	case 0x05: printf("DEC B"); break;
	case 0x06: printf("LD B, $%02x", opcode[1]); break;
	case 0x07: printf("RLCA"); break;
	case 0x08: printf("LD [$%02x%02x], SP", opcode[2], opcode[1]); break;
	case 0x09: printf("ADD HL, BC"); break;
	case 0x0a: printf("LD A, [BC]"); break;
	case 0x0b: printf("DEC BC"); break;
	case 0x0c: printf("INC C"); break;
	case 0x0d: printf("DEC C"); break;
	case 0x0e: printf("LD C, $%02x", opcode[1]); break;
	case 0x0f: printf("RRCA"); break;

	case 0x10: printf("STOP $%02x", opcode[1]); break;
	case 0x11: printf("LD DE, $%02x%02x", opcode[2], opcode[1]); break;
	case 0x12: printf("LD [DE], A"); break;
	case 0x13: printf("INC DE"); break;
	case 0x14: printf("INC D"); break;
	case 0x15: printf("DEC D"); break;
	case 0x16: printf("LD D, $%02x", opcode[1]); break;
	case 0x17: printf("RLA"); break;
	case 0x18: printf("JR $%02x", opcode[1]); break;
	case 0x19: printf("ADD HL, DE"); break;
	case 0x1a: printf("LD A, [DE]"); break;
	case 0x1b: printf("DEC DE"); break;
	case 0x1c: printf("INC E"); break;
	case 0x1d: printf("DEC E"); break;
	case 0x1e: printf("LD E, $%02x", opcode[1]); break;
	case 0x1f: printf("RRA"); break;

	case 0x20: printf("JR NZ, $%02x", opcode[1]); break;
	case 0x21: printf("LD HL, $%02x%02x", opcode[2], opcode[1]); break;
	case 0x22: printf("LD [HL+], A"); break;
	case 0x23: printf("INC HL"); break;
	case 0x24: printf("INC H"); break;
	case 0x25: printf("DEC H"); break;
	case 0x26: printf("LD H, $%02x", opcode[1]); break;
	case 0x27: printf("DAA"); break;
	case 0x28: printf("JR Z, $%02x", opcode[1]); break;
	case 0x29: printf("ADD HL, HL"); break;
	case 0x2a: printf("LD A, [HL+]"); break;
	case 0x2b: printf("DEC HL"); break;
	case 0x2c: printf("INC L"); break;
	case 0x2d: printf("DEC L"); break;
	case 0x2e: printf("LD L, $%02x", opcode[1]); break;
	case 0x2f: printf("CPL"); break;

	case 0x30: printf("JR NC, $%02x", opcode[1]); break;
	case 0x31: printf("LD SP, $%02x%02x", opcode[2], opcode[1]); break;
	case 0x32: printf("LD [HL-], A"); break;
	case 0x33: printf("INC SP"); break;
	case 0x34: printf("INC [HL]"); break;
	case 0x35: printf("DEC [HL]"); break;
	case 0x36: printf("LD [HL], $%02x", opcode[1]); break;
	case 0x37: printf("SCF"); break;
	case 0x38: printf("JR C, $%02x", opcode[1]); break;
	case 0x39: printf("ADD HL, SP"); break;
	case 0x3a: printf("LD A, [HL-]"); break;
	case 0x3b: printf("DEC SP"); break;
	case 0x3c: printf("INC A"); break;
	case 0x3d: printf("DEC A"); break;
	case 0x3e: printf("LD A, $%02x", opcode[1]); break;
	case 0x3f: printf("CCF"); break;

	case 0x40: printf("LD B, B"); break;
	case 0x41: printf("LD B, C"); break;
	case 0x42: printf("LD B, D"); break;
	case 0x43: printf("LD B, E"); break;
	case 0x44: printf("LD B, H"); break;
	case 0x45: printf("LD B, L"); break;
	case 0x46: printf("LD B, [HL]"); break;
	case 0x47: printf("LD B, A"); break;
	case 0x48: printf("LD C, B"); break;
	case 0x49: printf("LD C, C"); break;
	case 0x4a: printf("LD C, D"); break;
	case 0x4b: printf("LD C, E"); break;
	case 0x4c: printf("LD C, H"); break;
	case 0x4d: printf("LD C, L"); break;
	case 0x4e: printf("LD C, [HL]"); break;
	case 0x4f: printf("LD C, A"); break;

	case 0x50: printf("LD D, B"); break;
	case 0x51: printf("LD D, C"); break;
	case 0x52: printf("LD D, D"); break;
	case 0x53: printf("LD D, E"); break;
	case 0x54: printf("LD D, H"); break;
	case 0x55: printf("LD D, L"); break;
	case 0x56: printf("LD D, [HL]"); break;
	case 0x57: printf("LD D, A"); break;
	case 0x58: printf("LD E, B"); break;
	case 0x59: printf("LD E, C"); break;
	case 0x5a: printf("LD E, D"); break;
	case 0x5b: printf("LD E, E"); break;
	case 0x5c: printf("LD E, H"); break;
	case 0x5d: printf("LD E, L"); break;
	case 0x5e: printf("LD E, [HL]"); break;
	case 0x5f: printf("LD E, A"); break;

	case 0x60: printf("LD H, B"); break;
	case 0x61: printf("LD H, C"); break;
	case 0x62: printf("LD H, D"); break;
	case 0x63: printf("LD H, E"); break;
	case 0x64: printf("LD H, H"); break;
	case 0x65: printf("LD H, L"); break;
	case 0x66: printf("LD H, [HL]"); break;
	case 0x67: printf("LD H, A"); break;
	case 0x68: printf("LD L, B"); break;
	case 0x69: printf("LD L, C"); break;
	case 0x6a: printf("LD L, D"); break;
	case 0x6b: printf("LD L, E"); break;
	case 0x6c: printf("LD L, H"); break;
	case 0x6d: printf("LD L, L"); break;
	case 0x6e: printf("LD L, [HL]"); break;
	case 0x6f: printf("LD L, A"); break;


	case 0x70: printf("LD [HL], B"); break;
	case 0x71: printf("LD [HL], C"); break;
	case 0x72: printf("LD [HL], D"); break;
	case 0x73: printf("LD [HL], E"); break;
	case 0x74: printf("LD [HL], H"); break;
	case 0x75: printf("LD [HL], L"); break;
	case 0x76: printf("HALT"); break;
	case 0x77: printf("LD [HL], A"); break;
	case 0x78: printf("LD A, B"); break;
	case 0x79: printf("LD A, C"); break;
	case 0x7a: printf("LD A, D"); break;
	case 0x7b: printf("LD A, E"); break;
	case 0x7c: printf("LD A, H"); break;
	case 0x7d: printf("LD A, L"); break;
	case 0x7e: printf("LD A, [HL]"); break;
	case 0x7f: printf("LD A, A"); break;

	case 0x80: printf("ADD A, B"); break;
	case 0x81: printf("ADD A, C"); break;
	case 0x82: printf("ADD A, D"); break;
	case 0x83: printf("ADD A, E"); break;
	case 0x84: printf("ADD A, H"); break;
	case 0x85: printf("ADD A, L"); break;
	case 0x86: printf("ADD A, [HL]"); break;
	case 0x87: printf("ADD A, A"); break;
	case 0x88: printf("ADC A, B"); break;
	case 0x89: printf("ADC A, C"); break;
	case 0x8a: printf("ADC A, D"); break;
	case 0x8b: printf("ADC A, E"); break;
	case 0x8c: printf("ADC A, H"); break;
	case 0x8d: printf("ADC A, L"); break;
	case 0x8e: printf("ADC A, [HL]"); break;
	case 0x8f: printf("ADC A, A"); break;

	case 0x90: printf("SUB A, B"); break;
	case 0x91: printf("SUB A, C"); break;
	case 0x92: printf("SUB A, D"); break;
	case 0x93: printf("SUB A, E"); break;
	case 0x94: printf("SUB A, H"); break;
	case 0x95: printf("SUB A, L"); break;
	case 0x96: printf("SUB A, [HL]"); break;
	case 0x97: printf("SUB A, A"); break;
	case 0x98: printf("SBC A, B"); break;
	case 0x99: printf("SBC A, C"); break;
	case 0x9a: printf("SBC A, D"); break;
	case 0x9b: printf("SBC A, E"); break;
	case 0x9c: printf("SBC A, H"); break;
	case 0x9d: printf("SBC A, L"); break;
	case 0x9e: printf("SBC A, [HL]"); break;
	case 0x9f: printf("SBC A, A"); break;

	case 0xa0: printf("AND A, B"); break;
	case 0xa1: printf("AND A, C"); break;
	case 0xa2: printf("AND A, D"); break;
	case 0xa3: printf("AND A, E"); break;
	case 0xa4: printf("AND A, H"); break;
	case 0xa5: printf("AND A, L"); break;
	case 0xa6: printf("AND A, [HL]"); break;
	case 0xa7: printf("AND A, A"); break;
	case 0xa8: printf("XOR A, B"); break;
	case 0xa9: printf("XOR A, C"); break;
	case 0xaa: printf("XOR A, D"); break;
	case 0xab: printf("XOR A, E"); break;
	case 0xac: printf("XOR A, H"); break;
	case 0xad: printf("XOR A, L"); break;
	case 0xae: printf("XOR A, [HL]"); break;
	case 0xaf: printf("XOR A, A"); break;

	case 0xb0: printf("OR A, B"); break;
	case 0xb1: printf("OR A, C"); break;
	case 0xb2: printf("OR A, D"); break;
	case 0xb3: printf("OR A, E"); break;
	case 0xb4: printf("OR A, H"); break;
	case 0xb5: printf("OR A, L"); break;
	case 0xb6: printf("OR A, [HL]"); break;
	case 0xb7: printf("OR A, A"); break;
	case 0xb8: printf("CP A, B"); break;
	case 0xb9: printf("CP A, C"); break;
	case 0xba: printf("CP A, D"); break;
	case 0xbb: printf("CP A, E"); break;
	case 0xbc: printf("CP A, H"); break;
	case 0xbd: printf("CP A, L"); break;
	case 0xbe: printf("CP A, [HL]"); break;
	case 0xbf: printf("CP A, A"); break;

	case 0xc0: printf("RET NZ"); break;
	case 0xc1: printf("POP BC"); break;
	case 0xc2: printf("JP NZ, $%02x%02x", opcode[2], opcode[1]); break;
	case 0xc3: printf("JP $%02x%02x", opcode[2], opcode[1]); break;
	case 0xc4: printf("CALL NZ, $%02x%02x", opcode[2], opcode[1]); break;
	case 0xc5: printf("PUSH BC"); break;
	case 0xc6: printf("ADD A, $%02x", opcode[1]); break;
	case 0xc7: printf("RST $00"); break;
	case 0xc8: printf("RET Z"); break;
	case 0xc9: printf("RET"); break;
	case 0xca: printf("JP Z"); break;
	case 0xcb: printf("PREFIX"); break; // TODO: implement
	case 0xcc: printf("CALL Z, $%02x%02x", opcode[2], opcode[1]); break;
	case 0xcd: printf("CALL $%02x%02x", opcode[2], opcode[1]); break;
	case 0xce: printf("ADC A, $%02x", opcode[1]); break;
	case 0xcf: printf("RST $08"); break;

	case 0xd0: printf("RET NC"); break;
	case 0xd1: printf("POP DE"); break;
	case 0xd2: printf("JP NC, $%02x%02x", opcode[2], opcode[1]); break;
	case 0xd3: printf("0xd3 ILLEGAL"); break;
	case 0xd4: printf("CALL NC, $%02x%02x", opcode[2], opcode[1]); break;
	case 0xd5: printf("PUSH DE"); break;
	case 0xd6: printf("SUB A, $%02x", opcode[1]); break;
	case 0xd7: printf("RST $10"); break;
	case 0xd8: printf("RET C"); break;
	case 0xd9: printf("RETI"); break;
	case 0xda: printf("JP C"); break;
	case 0xdb: printf("0xdb ILLEGAL"); break; // TODO: implement
	case 0xdc: printf("CALL C, %02x%02x", opcode[2], opcode[1]); break;
	case 0xdd: printf("0xdd ILLEGAL"); break;
	case 0xde: printf("SBC A, $%02x", opcode[1]); break;
	case 0xdf: printf("RST $18"); break;

	case 0xe0: printf("LDH [$%02x], A", opcode[1]); break;
	case 0xe1: printf("POP HL"); break;
	case 0xe2: printf("LDH [C], A"); break;
	case 0xe3: printf("0xe3 ILLEGAL"); break;
	case 0xe4: printf("0xe4 ILLEGAL"); break;
	case 0xe5: printf("PUSH HL"); break;
	case 0xe6: printf("AND A, $%02x", opcode[1]); break;
	case 0xe7: printf("RST $20"); break;
	case 0xe8: printf("ADD SP, $%02x", opcode[1]); break; // Signed
	case 0xe9: printf("JP HL"); break;
	case 0xea: printf("LD [$%02x%02x], A", opcode[2], opcode[1]); break;
	case 0xeb: printf("0xeb ILLEGAL"); break;
	case 0xec: printf("0xec ILLEGAL"); break;
	case 0xed: printf("0xed ILLEGAL"); break;
	case 0xee: printf("XOR A, %02x", opcode[1]); break;
	case 0xef: printf("RST $28"); break;

	case 0xf0: printf("LDH A, [$%02x]", opcode[1]); break;
	case 0xf1: printf("POP AF"); break;
	case 0xf2: printf("LDH A, [C]"); break;
	case 0xf3: printf("DI"); break;
	case 0xf4: printf("0xf4 ILLEGAL"); break;
	case 0xf5: printf("PUSH AF"); break;
	case 0xf6: printf("OR A, $%02x", opcode[1]); break;
	case 0xf7: printf("RST $30"); break;
	case 0xf8: printf("LD HL, SP + $%02x", opcode[1]); break; // Signed
	case 0xf9: printf("LD SP, HL"); break;
	case 0xfa: printf("LD A, [$%02x%02x]", opcode[2], opcode[1]); break;
	case 0xfb: printf("EI"); break;
	case 0xfc: printf("0xfc ILLEGAL"); break;
	case 0xfd: printf("0xfd ILLEGAL"); break;
	case 0xfe: printf("CP A, %02x", opcode[1]); break;
	case 0xff: printf("RST $38"); break;

	default:
	    unimlemented_opcode(*opcode);
	    break;
    }
}

