#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../src/cpu.h"

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

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "please give test file\n");
		return 1;
	}
	char *buf = read_test(argv[1]);

	printf("%s\n", buf);
	return 1;
}
