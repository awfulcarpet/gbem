#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
	cpu->ie = cJSON_GetObjectItem(json, "ei")->valueint;

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
run_test(int opcode)
{
	char *filename = "tests/test.json";
	/*char *filename = NULL;*/
	/*sprintf(filename, "%02x.json", opcode);*/

	char *buf = read_test(filename);
	cJSON *json = cJSON_Parse(buf);

	struct Test *test = test_from_json(json);

	printf("%s %d\n", test->name, test->cycles);
	print_cpu_state(&test->initial);
	printf("\n");

	int cycles = 0;
	while (cycles < test->cycles) {
		cycles += execute(&test->initial);
	}

	print_cpu_state(&test->initial);
	print_cpu_state(&test->final);

	cJSON_Delete(json);
	free(buf);
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "please give test file\n");
		return 1;
	}

	return run_test(0);

	return 0;
}
