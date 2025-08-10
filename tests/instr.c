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
		for (int i = 0; i <= 0xffff; i++) {
			if (cpu1->memory[i] != cpu2->memory[i]) {
				fprintf(stderr, "%d: %d %d\n", i, cpu1->memory[i], cpu2->memory[i]);
			}
		}
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
	cpu->memory = calloc(1 << 16, sizeof(uint8_t));

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
	test->initial.log = stderr;
	test->final.log = stderr;


	struct CPU cpu = test->initial;
	cpu.memory = calloc(1 << 16, sizeof(uint8_t));
	memcpy(cpu.memory, test->initial.memory, 1 << 16);
	while (cpu.mcycles < test->cycles) {
		cpu.mcycles += execute_opcode(&cpu);
	}

	if (is_cpu_same(&cpu, &test->final) != 0) {
		fprintf(stderr, "FAIL\ntest %s failed\n\n", test->name);
		fprintf(stderr, "%d\n", test->cycles);
		fprintf(stderr, "initial\n");
		print_cpu_state(&test->initial);
		fprintf(stderr, "\n");

		free(cpu.memory);

		struct CPU cpu = test->initial;
		cpu.memory = calloc(1 << 16, sizeof(uint8_t));
		memcpy(cpu.memory, test->initial.memory, 1 << 16);
		while (cpu.mcycles < test->cycles) {
			cpu.mcycles += execute_opcode(&cpu);
			print_cpu_state(&cpu);
		}

		fprintf(stderr, "\nfinal\n");
		print_cpu_state(&test->final);
		fprintf(stderr, "\n");
		free(cpu.memory);
		failed = 1;
	} else {
		free(cpu.memory);
	}
	free(test->initial.memory);
	free(test->final.memory);

	free(test);

	return failed;
}

int
run_opcode(uint8_t opcode)
{
	char *filename = calloc(strlen("tests/GameboyCPUTests/v2/00.json") + 1, sizeof(char));
	sprintf(filename, "tests/GameboyCPUTests/v2/%02x.json", opcode);

	char *buf = read_test(filename);

	cJSON *json = cJSON_Parse(buf);
	cJSON *test = NULL;

	char *mnemonic = get_mnemonic(&opcode);
	int len = 20 - strlen(mnemonic);
	printf("%02x: %s%*.*s", opcode, mnemonic, len, len, "...........................");

	cJSON_ArrayForEach(test, json)
	{
		if (run_test(test)) {
			printf("fail\n");
			fprintf(stderr, "%02x: %s%*.*sfail\n", opcode, mnemonic, len, len, "...........................");
			cJSON_Delete(json);
			free(buf);
			free(mnemonic);
			return 1;
		}
	}
	free(mnemonic);

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

	int tests = 0;
	int passed = 0;
	for (int i = 0x00; i <= 0xFF; i++) {
		/* illegal opcodes */
		if (i == 0xD3 || i == 0xDB || i == 0xDD
			|| i == 0xE3 || i == 0xE4 || i == 0xEB || i == 0xEC || i == 0xED
			|| i == 0xF4 || i == 0xFC || i == 0xFD)
			continue;

		/* untested opcodes (STOP, HALT, DI, EI)*/
		if (i == 0x10 || i == 0x76 || i == 0xf3 || i == 0xfb)
			continue;

		tests++;
		passed += !run_opcode(i);
	}

	fprintf(stderr, "passed %d/%d tests\n", passed, tests);

	return 0;
}
