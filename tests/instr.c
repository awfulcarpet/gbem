#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "../src/cpu.h"
#include "../src/opcode.h"
#include "cJSON.h"

struct Test {
	char *name;
	struct CPU initial;
	struct CPU final;
	int cycles;
};

char *
read_test(char *filename)
{
	char *buf = NULL;
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "unable to open test file: %s\n", filename);
		return NULL;
	}
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	buf = calloc(len + 1, sizeof(uint8_t));
	fread(buf, 1, len, f);
	fclose(f);
	return buf;
}

#define LOG(msg) fprintf(stderr, msg);

int
is_cpu_same(struct CPU *cpu1, struct CPU *cpu2)
{
	if (cpu1->pc != cpu2->pc) {
		LOG("pc differ\n")
		return 1;
	}

	if (cpu1->sp != cpu2->sp) {
		LOG("sp differ\n")
		return 1;
	}

	if (cpu1->a != cpu2->a) {
		LOG("a differ\n")
		return 1;
	}

	if (cpu1->b != cpu2->b) {
		LOG("b differ\n")
		return 1;
	}

	if (cpu1->c != cpu2->c) {
		LOG("c differ\n")
		return 1;
	}

	if (cpu1->d != cpu2->d) {
		LOG("d differ\n")
		return 1;
	}

	if (cpu1->e != cpu2->e) {
		LOG("e differ\n")
		return 1;
	}

	if (cpu1->h != cpu2->h) {
		LOG("h differ\n")
		return 1;
	}

	if (cpu1->l != cpu2->l) {
		LOG("l differ\n")
		return 1;
	}

	if (cpu1->pc != cpu2->pc) {
		LOG("pc differ\n")
		return 1;
	}

	if (cpu1->f.flags != cpu2->f.flags) {
		LOG("f.flags differ\n")
		return 1;
	}

	if (memcmp(cpu1->memory, cpu2->memory, 0xffff + 1) != 0) {
		LOG("mem differ\n")
		return 1;
	}
	return 0;
}

void
cpu_from_json(const cJSON *json, struct CPU *cpu)
{
#ifdef sm83
	cpu->pc = cJSON_GetObjectItem(json, "pc")->valueint;
#else
	/* why gbcputests is pc = pc + 1 */
	cpu->pc = cJSON_GetObjectItem(json, "pc")->valueint - 1;
#endif
	cpu->sp = cJSON_GetObjectItem(json, "sp")->valueint;
	cpu->a = cJSON_GetObjectItem(json, "a")->valueint;
	cpu->b = cJSON_GetObjectItem(json, "b")->valueint;
	cpu->c = cJSON_GetObjectItem(json, "c")->valueint;
	cpu->d = cJSON_GetObjectItem(json, "d")->valueint;
	cpu->e = cJSON_GetObjectItem(json, "e")->valueint;
	cpu->f.flags = cJSON_GetObjectItem(json, "f")->valueint;
	cpu->h = cJSON_GetObjectItem(json, "h")->valueint;
	cpu->l = cJSON_GetObjectItem(json, "l")->valueint;
	/*cpu->ie = cJSON_GetObjectItem(json, "ie")->valueint;*/

	const cJSON *ram = cJSON_GetObjectItem(json, "ram");
	const cJSON *content = NULL;

	cJSON_ArrayForEach(content, ram)
	{
		int adr = cJSON_GetArrayItem(content, 0)->valueint;
		int num = cJSON_GetArrayItem(content, 1)->valueint;
		cpu->memory[adr] = num;
	}
}

struct Test *
test_from_json(cJSON *json)
{
	struct Test *test = calloc(1, sizeof(struct Test));
	test->name = cJSON_GetObjectItem(json, "name")->valuestring;
	const cJSON *initial = cJSON_GetObjectItem(json, "initial");
	const cJSON *final = cJSON_GetObjectItem(json, "final");

	cpu_from_json(initial, &test->initial);
	cpu_from_json(final, &test->final);

	test->cycles = cJSON_GetArraySize(cJSON_GetObjectItem(json, "cycles"));
	test->final.mcycles = test->cycles;

	return test;
}

int
run_test(cJSON *json)
{
	int failed = 0;
	struct Test *test = test_from_json(json);


	struct CPU cpu = test->initial;
	while (cpu.mcycles < test->cycles) {
		cpu.mcycles += execute(&cpu);
	}

	if (is_cpu_same(&cpu, &test->final) != 0) {
		printf("FAIL\ntest %s failed\n\n", test->name);
		printf("%d\n", test->cycles);
		printf("initial\n");
		print_cpu_state(&test->initial);
		printf("\n");

		struct CPU cpu = test->initial;
		while (cpu.mcycles < test->cycles) {
			cpu.mcycles += execute(&cpu);
			print_cpu_state(&cpu);
		}

		printf("\nfinal\n");
		print_cpu_state(&test->final);
		printf("\n");
		failed = 1;
	}

	free(test);

	return failed;
}

/* returns amount of tests failed */
int
run_opcode(int opcode)
{
#ifdef sm83
	char *filename = calloc(strlen("tests/sm83/v1/00.json") + 1, sizeof(char));
	sprintf(filename, "tests/sm83/v1/%02x.json", opcode);
#else
	char *filename = calloc(strlen("tests/GameboyCPUTests/v2/00.json") + 1, sizeof(char));
	sprintf(filename, "tests/GameboyCPUTests/v2/%02x.json", opcode);
#endif

	char *buf = read_test(filename);

	cJSON *json = cJSON_Parse(buf);
	cJSON *test = NULL;

	char *mnemonic = get_mnemonic(opcode);
	int len = 20 - strlen(mnemonic);
	printf("%02x: %s%*.*s", opcode, mnemonic, len, len, "...........................");
	free(mnemonic);

	cJSON_ArrayForEach(test, json)
	{
		if (run_test(test)) {
			exit(1);
		}
	}

	printf("pass\n");

	cJSON_Delete(json);
	free(buf);

	return 0;
}

int main(int argc, char *argv[])
{

	if (argc == 2) {
		run_opcode(atoi(argv[1]));
		return 0;
	}

	if (argc > 2) {
		for (int i = 1; i < argc; i++) {
			uint8_t opcode = atoi(argv[i]);
			run_opcode(opcode);
		}
		return 0;
	}

#ifdef sm83
	printf("running sm83 tests\n");
#else
	printf("running GameboyCPUTests tests\n");
#endif

	/* does not test EI, DI, STOP, or HALT */
	run_opcode(0x00);

	/* ld r16, imm16 */
	for (int i = 0x01; i <= 0x31; i += 0x10) {
		run_opcode(i);
	}

	/* ld [r16mem], a */
	for (int i = 0x02; i <= 0x32; i += 0x10) {
		run_opcode(i);
	}

	/* ld a, [r16mem] */
	for (int i = 0x0A; i <= 0x3A; i += 0x10) {
		run_opcode(i);
	}

	/* ld [imm16], sp */
	run_opcode(0x08);

	/* inc r16 */
	for (int i = 0x03; i <= 0x33; i += 0x10) {
		run_opcode(i);
	}

	/* dec r16 */
	for (int i = 0x0b; i <= 0x3b; i += 0x10) {
		run_opcode(i);
	}

	/* add hl, r16*/
	for (int i = 0x09; i <= 0x39; i += 0x10) {
		run_opcode(i);
	}

	/* dec r8 */
	for (int i = 0x05; i <= 0x3c; i += 0x08) {
		run_opcode(i);
	}

	/* inc r8 */
	for (int i = 0x04; i <= 0x3c; i += 0x08) {
		run_opcode(i);
	}

	/* ld r8 imm8 */
	for (int i = 0x06; i <= 0x36; i += 0x08) {
		run_opcode(i);
	}

	/* bitshifts */
	run_opcode(0x07);
	run_opcode(0x0F);
	run_opcode(0x17);
	run_opcode(0x1F);
	run_opcode(0x27);
	run_opcode(0x2F);
	run_opcode(0x37);
	run_opcode(0x3F);

	/* ld r8 r8 */
	for (int i = 0x40; i <= 0x7F; i += 0x01) {
		if (i != 0x76)
			run_opcode(i);
	}

	/* 8 bit arith */
	for (int i = 0x80; i <= 0x87; i++)
		run_opcode(i);
	for (int i = 0x88; i <= 0x8f; i++)
		run_opcode(i);
	for (int i = 0x90; i <= 0x97; i++)
		run_opcode(i);
	for (int i = 0x98; i <= 0x9f; i++)
		run_opcode(i);
	for (int i = 0xa0; i <= 0xa7; i++)
		run_opcode(i);
	for (int i = 0xa8; i <= 0xaf; i++)
		run_opcode(i);
	for (int i = 0xb0; i <= 0xb7; i++)
		run_opcode(i);
	for (int i = 0xb8; i <= 0xbf; i++)
		run_opcode(i);

	/* 8 bit arith imm8 */
	run_opcode(0xc6);
	run_opcode(0xce);
	run_opcode(0xd6);
	run_opcode(0xde);
	run_opcode(0xe6);
	run_opcode(0xee);
	run_opcode(0xf6);
	run_opcode(0xfe);

	run_opcode(0xc9);
	run_opcode(0xd9);

	run_opcode(0xc0);
	run_opcode(0xc8);
	run_opcode(0xd0);
	run_opcode(0xd8);

	run_opcode(0xc3);
	run_opcode(0xe9);

	run_opcode(0xc2);
	run_opcode(0xca);
	run_opcode(0xd2);
	run_opcode(0xda);

	run_opcode(0xcd);

	run_opcode(0xc4);
	run_opcode(0xcc);
	run_opcode(0xd4);
	run_opcode(0xdc);

	run_opcode(0xc6);
	run_opcode(0xcf);
	run_opcode(0xd6);
	run_opcode(0xdf);
	run_opcode(0xe6);
	run_opcode(0xef);
	run_opcode(0xf6);
	run_opcode(0xff);
	return 0;
}
