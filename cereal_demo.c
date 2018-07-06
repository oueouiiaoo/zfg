////bin/bash -c "rm a.out; gcc cereal_demo.c; ./a.out"; exit

#include <stdio.h>

#define ZFG_CEREAL_IMPLEMENTATION
// #define ZFG_CEREAL_FILE_NAME "gunk.txt"
#include "zfg_cereal.h"

int x () {
	ZFG_CEREAL_VAR (float, a, = 12.0f);
	a += 1;
	return (int) a;
}

struct V {
	short a;
	short b;
};

int y () {
	ZFG_CEREAL_VAR (struct V, a, = { 50, 100 });
	a.a += a.b;
	a.b++;
	return (int) a.a + (int) a.b;
}

int main (int argc, char **argv) {
	while (1) {
		ZFG_CEREAL_VAR (int, a, [2] = { 12, 13 });

		a[0] += x();
		a[1] += y();

		if (a[0] > a[1]) {

			printf ("a : %i, %i\n", a[0], a[1]);
			a[0] += a[1];
			a[1] += a[0];
			printf ("a : %i, %i\n", a[0], a[1]);

			break;
		}
	}
	// zfg_cereal_print_state ();
	zfg_cereal_write_file ();
	return 0;
}
