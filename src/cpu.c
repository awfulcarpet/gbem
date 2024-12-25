#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

struct CPU *
init_cpu(void) {
	struct CPU *cpu = calloc(1, sizeof(struct CPU));
	cpu->a = 0;
	cpu->b = 0;
	cpu->c = 0;
	cpu->d = 0;
	cpu->e = 0;
	cpu->h = 0;
	cpu->l = 0;
	cpu->f.flags = 0;
	cpu->pc = 0;
	cpu->sp = 0;
	cpu->ie = 0;
	cpu->ir = 0;

	memset(cpu->memory, 0, 0xFFFF + 1);

	return cpu;
}

static void
unimlemented_opcode(uint8_t opcode) {
	fprintf(stderr, "unimplemented opcode: %02x\n", opcode);
	exit(1);
}

void
print_mnemonic(uint8_t *opcode) {
	switch (*opcode) {
	case 0x00: printf("NOP"); break;
	case 0x01: printf("LD BC,%02x%02x", opcode[2], opcode[1]); break;
	case 0x02: printf("LD [BC], A"); break;
	case 0x03: printf("INC BC"); break;
	case 0x04: printf("INC B"); break;
	case 0x05: printf("DEC B"); break;
	case 0x06: printf("LD B, %02x", opcode[1]); break;
	case 0x07: printf("RLCA"); break;
	case 0x08: printf("LD [%02x%02x], SP", opcode[2], opcode[1]); break;
	case 0x09: printf("ADD HL, BC"); break;
	case 0x0a: printf("LD A, [BC]"); break;
	case 0x0b: printf("DEC BC"); break;
	case 0x0c: printf("INC C"); break;
	case 0x0d: printf("DEC C"); break;
	case 0x0e: printf("LD C, %02x", opcode[1]); break;
	case 0x0f: printf("RRCA"); break;
	case 0x10: printf("NOP"); break;
	case 0x11: printf("NOP"); break;
	case 0x12: printf("NOP"); break;
	case 0x13: printf("NOP"); break;
	case 0x14: printf("NOP"); break;
	case 0x15: printf("NOP"); break;
	case 0x16: printf("NOP"); break;
	case 0x17: printf("NOP"); break;
	case 0x18: printf("NOP"); break;
	case 0x19: printf("NOP"); break;
	case 0x1a: printf("NOP"); break;
	case 0x1b: printf("NOP"); break;
	case 0x1c: printf("NOP"); break;
	case 0x1d: printf("NOP"); break;
	case 0x1e: printf("NOP"); break;
	case 0x1f: printf("NOP"); break;
	case 0x20: printf("NOP"); break;
	case 0x21: printf("NOP"); break;
	case 0x22: printf("NOP"); break;
	case 0x23: printf("NOP"); break;
	case 0x24: printf("NOP"); break;
	case 0x25: printf("NOP"); break;
	case 0x26: printf("NOP"); break;
	case 0x27: printf("NOP"); break;
	case 0x28: printf("NOP"); break;
	case 0x29: printf("NOP"); break;
	case 0x2a: printf("NOP"); break;
	case 0x2b: printf("NOP"); break;
	case 0x2c: printf("NOP"); break;
	case 0x2d: printf("NOP"); break;
	case 0x2e: printf("NOP"); break;
	case 0x2f: printf("NOP"); break;
	case 0x30: printf("NOP"); break;
	case 0x31: printf("NOP"); break;
	case 0x32: printf("NOP"); break;
	case 0x33: printf("NOP"); break;
	case 0x34: printf("NOP"); break;
	case 0x35: printf("NOP"); break;
	case 0x36: printf("NOP"); break;
	case 0x37: printf("NOP"); break;
	case 0x38: printf("NOP"); break;
	case 0x39: printf("NOP"); break;
	case 0x3a: printf("NOP"); break;
	case 0x3b: printf("NOP"); break;
	case 0x3c: printf("NOP"); break;
	case 0x3d: printf("NOP"); break;
	case 0x3e: printf("NOP"); break;
	case 0x3f: printf("NOP"); break;
	case 0x40: printf("NOP"); break;
	case 0x41: printf("NOP"); break;
	case 0x42: printf("NOP"); break;
	case 0x43: printf("NOP"); break;
	case 0x44: printf("NOP"); break;
	case 0x45: printf("NOP"); break;
	case 0x46: printf("NOP"); break;
	case 0x47: printf("NOP"); break;
	case 0x48: printf("NOP"); break;
	case 0x49: printf("NOP"); break;
	case 0x4a: printf("NOP"); break;
	case 0x4b: printf("NOP"); break;
	case 0x4c: printf("NOP"); break;
	case 0x4d: printf("NOP"); break;
	case 0x4e: printf("NOP"); break;
	case 0x4f: printf("NOP"); break;
	case 0x50: printf("NOP"); break;
	case 0x51: printf("NOP"); break;
	case 0x52: printf("NOP"); break;
	case 0x53: printf("NOP"); break;
	case 0x54: printf("NOP"); break;
	case 0x55: printf("NOP"); break;
	case 0x56: printf("NOP"); break;
	case 0x57: printf("NOP"); break;
	case 0x58: printf("NOP"); break;
	case 0x59: printf("NOP"); break;
	case 0x5a: printf("NOP"); break;
	case 0x5b: printf("NOP"); break;
	case 0x5c: printf("NOP"); break;
	case 0x5d: printf("NOP"); break;
	case 0x5e: printf("NOP"); break;
	case 0x5f: printf("NOP"); break;
	case 0x60: printf("NOP"); break;
	case 0x61: printf("NOP"); break;
	case 0x62: printf("NOP"); break;
	case 0x63: printf("NOP"); break;
	case 0x64: printf("NOP"); break;
	case 0x65: printf("NOP"); break;
	case 0x66: printf("NOP"); break;
	case 0x67: printf("NOP"); break;
	case 0x68: printf("NOP"); break;
	case 0x69: printf("NOP"); break;
	case 0x6a: printf("NOP"); break;
	case 0x6b: printf("NOP"); break;
	case 0x6c: printf("NOP"); break;
	case 0x6d: printf("NOP"); break;
	case 0x6e: printf("NOP"); break;
	case 0x6f: printf("NOP"); break;
	case 0x70: printf("NOP"); break;
	case 0x71: printf("NOP"); break;
	case 0x72: printf("NOP"); break;
	case 0x73: printf("NOP"); break;
	case 0x74: printf("NOP"); break;
	case 0x75: printf("NOP"); break;
	case 0x76: printf("NOP"); break;
	case 0x77: printf("NOP"); break;
	case 0x78: printf("NOP"); break;
	case 0x79: printf("NOP"); break;
	case 0x7a: printf("NOP"); break;
	case 0x7b: printf("NOP"); break;
	case 0x7c: printf("NOP"); break;
	case 0x7d: printf("NOP"); break;
	case 0x7e: printf("NOP"); break;
	case 0x7f: printf("NOP"); break;
	case 0x80: printf("NOP"); break;
	case 0x81: printf("NOP"); break;
	case 0x82: printf("NOP"); break;
	case 0x83: printf("NOP"); break;
	case 0x84: printf("NOP"); break;
	case 0x85: printf("NOP"); break;
	case 0x86: printf("NOP"); break;
	case 0x87: printf("NOP"); break;
	case 0x88: printf("NOP"); break;
	case 0x89: printf("NOP"); break;
	case 0x8a: printf("NOP"); break;
	case 0x8b: printf("NOP"); break;
	case 0x8c: printf("NOP"); break;
	case 0x8d: printf("NOP"); break;
	case 0x8e: printf("NOP"); break;
	case 0x8f: printf("NOP"); break;
	case 0x90: printf("NOP"); break;
	case 0x91: printf("NOP"); break;
	case 0x92: printf("NOP"); break;
	case 0x93: printf("NOP"); break;
	case 0x94: printf("NOP"); break;
	case 0x95: printf("NOP"); break;
	case 0x96: printf("NOP"); break;
	case 0x97: printf("NOP"); break;
	case 0x98: printf("NOP"); break;
	case 0x99: printf("NOP"); break;
	case 0x9a: printf("NOP"); break;
	case 0x9b: printf("NOP"); break;
	case 0x9c: printf("NOP"); break;
	case 0x9d: printf("NOP"); break;
	case 0x9e: printf("NOP"); break;
	case 0x9f: printf("NOP"); break;
	case 0xa0: printf("NOP"); break;
	case 0xa1: printf("NOP"); break;
	case 0xa2: printf("NOP"); break;
	case 0xa3: printf("NOP"); break;
	case 0xa4: printf("NOP"); break;
	case 0xa5: printf("NOP"); break;
	case 0xa6: printf("NOP"); break;
	case 0xa7: printf("NOP"); break;
	case 0xa8: printf("NOP"); break;
	case 0xa9: printf("NOP"); break;
	case 0xaa: printf("NOP"); break;
	case 0xab: printf("NOP"); break;
	case 0xac: printf("NOP"); break;
	case 0xad: printf("NOP"); break;
	case 0xae: printf("NOP"); break;
	case 0xaf: printf("NOP"); break;
	case 0xb0: printf("NOP"); break;
	case 0xb1: printf("NOP"); break;
	case 0xb2: printf("NOP"); break;
	case 0xb3: printf("NOP"); break;
	case 0xb4: printf("NOP"); break;
	case 0xb5: printf("NOP"); break;
	case 0xb6: printf("NOP"); break;
	case 0xb7: printf("NOP"); break;
	case 0xb8: printf("NOP"); break;
	case 0xb9: printf("NOP"); break;
	case 0xba: printf("NOP"); break;
	case 0xbb: printf("NOP"); break;
	case 0xbc: printf("NOP"); break;
	case 0xbd: printf("NOP"); break;
	case 0xbe: printf("NOP"); break;
	case 0xbf: printf("NOP"); break;
	case 0xc0: printf("NOP"); break;
	case 0xc1: printf("NOP"); break;
	case 0xc2: printf("NOP"); break;
	case 0xc3: printf("NOP"); break;
	case 0xc4: printf("NOP"); break;
	case 0xc5: printf("NOP"); break;
	case 0xc6: printf("NOP"); break;
	case 0xc7: printf("NOP"); break;
	case 0xc8: printf("NOP"); break;
	case 0xc9: printf("NOP"); break;
	case 0xca: printf("NOP"); break;
	case 0xcb: printf("NOP"); break;
	case 0xcc: printf("NOP"); break;
	case 0xcd: printf("NOP"); break;
	case 0xce: printf("NOP"); break;
	case 0xcf: printf("NOP"); break;
	case 0xd0: printf("NOP"); break;
	case 0xd1: printf("NOP"); break;
	case 0xd2: printf("NOP"); break;
	case 0xd3: printf("NOP"); break;
	case 0xd4: printf("NOP"); break;
	case 0xd5: printf("NOP"); break;
	case 0xd6: printf("NOP"); break;
	case 0xd7: printf("NOP"); break;
	case 0xd8: printf("NOP"); break;
	case 0xd9: printf("NOP"); break;
	case 0xda: printf("NOP"); break;
	case 0xdb: printf("NOP"); break;
	case 0xdc: printf("NOP"); break;
	case 0xdd: printf("NOP"); break;
	case 0xde: printf("NOP"); break;
	case 0xdf: printf("NOP"); break;
	case 0xe0: printf("NOP"); break;
	case 0xe1: printf("NOP"); break;
	case 0xe2: printf("NOP"); break;
	case 0xe3: printf("NOP"); break;
	case 0xe4: printf("NOP"); break;
	case 0xe5: printf("NOP"); break;
	case 0xe6: printf("NOP"); break;
	case 0xe7: printf("NOP"); break;
	case 0xe8: printf("NOP"); break;
	case 0xe9: printf("NOP"); break;
	case 0xea: printf("NOP"); break;
	case 0xeb: printf("NOP"); break;
	case 0xec: printf("NOP"); break;
	case 0xed: printf("NOP"); break;
	case 0xee: printf("NOP"); break;
	case 0xef: printf("NOP"); break;
	case 0xf0: printf("NOP"); break;
	case 0xf1: printf("NOP"); break;
	case 0xf2: printf("NOP"); break;
	case 0xf3: printf("NOP"); break;
	case 0xf4: printf("NOP"); break;
	case 0xf5: printf("NOP"); break;
	case 0xf6: printf("NOP"); break;
	case 0xf7: printf("NOP"); break;
	case 0xf8: printf("NOP"); break;
	case 0xf9: printf("NOP"); break;
	case 0xfa: printf("NOP"); break;
	case 0xfb: printf("NOP"); break;
	case 0xfc: printf("NOP"); break;
	case 0xfd: printf("NOP"); break;
	case 0xfe: printf("NOP"); break;
	case 0xff: printf("NOP"); break;
	default:
		unimlemented_opcode(*opcode);
		break;
	}
}

// TODO return cycles later
int
execute(struct CPU *cpu) {
	uint8_t *opcode = &cpu->memory[cpu->pc];
	cpu->pc++;

	print_mnemonic(opcode);
	printf("\n");
	return 0;
}
