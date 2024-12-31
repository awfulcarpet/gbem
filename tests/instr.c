#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "../src/cpu.h"
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
		fprintf(stderr, "unable to open test file\n");
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

int
is_cpu_same(struct CPU *cpu1, struct CPU *cpu2)
{
	if (cpu1->pc != cpu2->pc) return 1;
	if (cpu1->sp != cpu2->sp) return 1;
	if (cpu1->a != cpu2->a) return 1;
	if (cpu1->b != cpu2->b) return 1;
	if (cpu1->c != cpu2->c) return 1;
	if (cpu1->d != cpu2->d) return 1;
	if (cpu1->e != cpu2->e) return 1;
	if (cpu1->h != cpu2->h) return 1;
	if (cpu1->l != cpu2->l) return 1;
	if (cpu1->pc != cpu2->pc) return 1;
	if (cpu1->f.flags != cpu2->f.flags) return 1;

	if (memcmp(cpu1->memory, cpu2->memory, 0xffff + 1) != 0) return 1;
	return 0;
}

void
cpu_from_json(const cJSON *json, struct CPU *cpu)
{
	cpu->pc = cJSON_GetObjectItem(json, "pc")->valueint;
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

	return test;
}

int
run_test(cJSON *json)
{
	int failed = 0;
	struct Test *test = test_from_json(json);


	int cycles = 0;
	while (cycles < test->cycles) {
		cycles += execute(&test->initial);
	}

	if (is_cpu_same(&test->initial, &test->final) == 0) {
		/*printf("test %s: success\n", test->name);*/
	} else {
		printf("test %s: fail\n", test->name);
		failed = 1;
	}

	free(test);

	return failed;
}

/* returns amount of tests failed */
int
run_opcode(int opcode)
{
	int failed = 0;
	char *filename = calloc(strlen("tests/sm83/v1/00.json") + 1, sizeof(char));
	sprintf(filename, "tests/sm83/v1/%02x.json", opcode);

	char *buf = read_test(filename);
	cJSON *json = cJSON_Parse(buf);
	cJSON *test = NULL;

	cJSON_ArrayForEach(test, json)
	{
		failed += run_test(test);
	}


	cJSON_Delete(json);
	free(buf);
	return failed;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "please provide at least one opcode\n");
		return 1;
	}

	if (argc == 2) {
		printf("opcode %02x: %d failures\n", atoi(argv[1]), run_opcode(atoi(argv[1])));
		return 0;
	}

	for (int i = atoi(argv[1]); i <= atoi(argv[2]); i++) {
		printf("opcode %02x: %d failures\n", i, run_opcode(i));
	}

	return 0;
}
