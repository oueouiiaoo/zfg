// zfg_cereal.h

#ifndef ZFG_CEREAL_HEADER_INCLUDED
#define ZFG_CEREAL_HEADER_INCLUDED

typedef struct {
	const char *id1;
	const char *id2;
	int size;
	unsigned char *data;
} zfg_cereal_var_t;

void zfg_cereal_print_state (void);

void zfg_cereal_var (zfg_cereal_var_t *);

void zfg_cereal_write_file (void);

void zfg_cereal_write_file_ex (const char *);

#define ZFG_CEREAL_VAR(T, N, ...) \
static T N __VA_ARGS__; \
static int N ## zfg_cereal = 0; \
if (!N ## zfg_cereal) { \
	N ## zfg_cereal = 1; \
	zfg_cereal_var_t N ## zfg_cereal_var = { \
		__FUNCTION__, #T #N, sizeof (N), (unsigned char *) &N }; \
	zfg_cereal_var (&N ## zfg_cereal_var); \
} \
else { \
}

#endif // ZFG_CEREAL_HEADER_INCLUDED

#ifdef ZFG_CEREAL_IMPLEMENTATION

// todo: check the include guards here to save time ???
#include <stdio.h>

#ifndef ZFG_CEREAL_MAX_VARS
#define ZFG_CEREAL_MAX_VARS 16
#endif

#ifndef ZFG_CEREAL_MAX_TEXT
#define ZFG_CEREAL_MAX_TEXT 1024
#endif

#ifndef ZFG_CEREAL_FILE_NAME
#define ZFG_CEREAL_FILE_NAME "cereal.txt"
#endif

struct {
	int is_loaded;

	int text_next;
	char text[ZFG_CEREAL_MAX_TEXT];

	int vars_next;
	zfg_cereal_var_t vars[ZFG_CEREAL_MAX_VARS];
} zfg_cereal = { 0 };

// Good.
void zfg_cereal_print_error (const char *e) {
#ifndef ZFG_CEREAL_DONT_PRINT_ERRORS
	printf ("[zfg_cereal error] %s\n", e);
#endif
}

// Good, i think.
void zfg_cereal_print_state (void) {
	zfg_cereal_var_t *v = zfg_cereal.vars;

	printf ("[zfg_cereal state]\n");
	printf ("ZFG_CEREAL_MAX_VARS: %i\n", ZFG_CEREAL_MAX_VARS);
	printf ("ZFG_CEREAL_MAX_TEXT: %i\n\n", ZFG_CEREAL_MAX_TEXT);
	printf ("is_loaded: %i\n\n", zfg_cereal.is_loaded);
	printf ("text_next: %i\n", zfg_cereal.text_next);
	for (int i = 0; i < zfg_cereal.text_next; i++) {
		if (zfg_cereal.text[i] == '\0')
			fwrite ("<\\0>", 4, 1, stdout);
		else
			fwrite (&zfg_cereal.text[i], 1, 1, stdout);
	}
	printf ("vars_next: %i\n", zfg_cereal.vars_next);
	printf ("vars: \n");
	for (int i = 0; i < zfg_cereal.vars_next; i++, v++)
		printf ("%s:%s:%i\n", v->id1, v->id2, v->size);
}

// Good.
// This is mainly here in case i decide to add spaces into the hex code
// so that it will be easy to read as a human.
int zfg_cereal_hex_encoded_len (int bytes) {
	return bytes * 2;
}

// Good.
void zfg_cereal_hex_encode (const void *in, char *out, int bytes) {
	const char *hex = "0123456789abcdef";
	const unsigned char *i = in;

	for (int b = 0; b < bytes; b++) {
		*out++ = hex[*i / 16];
		*out++ = hex[*i++ % 16];
	}
}

// Good.
// Note: bytes is the number of bytes to be copied to out, NOT the number
// of characters to be read, any non hex characters will be read and ignored
// reading will continue until out has been filled completely, please pass
// the size of the type which you are trying to decoode as bytes.
void zfg_cereal_hex_decode (const char *in, void *out, int bytes) {
	unsigned char *o = out;

	for (int b = 0; b < bytes;) {
		if ((*in < '0' || *in > '9') && (*in < 'a' || *in > 'f')) {
			in++;
			continue;
		}

		*o = *in <= '9' ? (*in - '0') * 16 : ((*in - 'a') + 10) * 16;
		in++;

		*o += *in <= '9' ? (*in - '0') : ((*in - 'a') + 10);
		in++;

		o++;

		b++;
	}
}

// Good.
int zfg_cereal_str_len (const char *s) {
	int l = 0;
	for (; *s != '\0'; l++, s++);
	return l;
}

// Untested.
int zfg_cereal_str_eq (const char *a, const char *b) {
	if (a == NULL || b == NULL)
		return 0;

	if (a == b)
		return 1;

	for (; *a == *b; a++, b++)
		if (*a == '\0')
			return 1;

	return 0;
}

// Untested.
void zfg_cereal_copy (const void *in, void *out, int bytes) {
	unsigned char *i = (unsigned char *) in;
	unsigned char *o = (unsigned char *) out;

	for (int b = 0; b < bytes; b++) {
		*o = *i;
		*i++;
		*o++;
	}
}

// Untetsed.
int zfg_cereal_var_text_len (zfg_cereal_var_t *v) {
	return (
	zfg_cereal_str_len (v->id1) + 1 +
	zfg_cereal_str_len (v->id2) + 1 +
	zfg_cereal_hex_encoded_len (sizeof (v->size)) + 1 +
	zfg_cereal_hex_encoded_len (v->size) + 1);
}

// Untested.
void zfg_cereal_text_write_str (const char *s) {
	int l = zfg_cereal_str_len (s);
	zfg_cereal_copy (s, &zfg_cereal.text[zfg_cereal.text_next], l);
	zfg_cereal.text_next += l;
}

// Untested.
void zfg_cereal_text_write_hex (const void *h, int l) {
	zfg_cereal_hex_encode (h, &zfg_cereal.text[zfg_cereal.text_next], l);
	zfg_cereal.text_next += zfg_cereal_hex_encoded_len (l);
}

// Untested.
int zfg_cereal_text_write_var (zfg_cereal_var_t *v) {
	if (zfg_cereal.text_next + zfg_cereal_var_text_len (v)
	>= ZFG_CEREAL_MAX_TEXT)
		return 0;

	zfg_cereal_text_write_str (v->id1);
	zfg_cereal_text_write_str (":");

	zfg_cereal_text_write_str (v->id2);
	zfg_cereal_text_write_str (":");

	zfg_cereal_text_write_hex (&v->size, sizeof (v->size));
	zfg_cereal_text_write_str (":");

	zfg_cereal_text_write_hex (v->data, v->size);
	zfg_cereal_text_write_str ("\n");

	return 1;
}

// Untested.
int zfg_cereal_text_scan_for (char **p, char s) {
	for (; *p < &zfg_cereal.text[ZFG_CEREAL_MAX_TEXT]; (*p)++)
		if (**p == s)
			return 1;
	return 0;
}

// Untested.
// Note: this may segfault if text isnt formatted correctly
void zfg_cereal_create_vars_from_text (void) {
	zfg_cereal_var_t v = { 0 };
	char *p = zfg_cereal.text;

	for (;;) {
		char *pp = p;

		if (zfg_cereal_text_scan_for (&pp, '\n') == 0) {
			// there was no new line, invalid format or eof
			return;
		}

		v.id1 = p;
		zfg_cereal_text_scan_for (&p, ':');
		*p++ = '\0';

		v.id2 = p;
		zfg_cereal_text_scan_for (&p, ':');
		*p++ = '\0';

		zfg_cereal_hex_decode (p, &v.size, sizeof (v.size));
		p += zfg_cereal_hex_encoded_len (sizeof (v.size)) + 1;

		v.data = (unsigned char *) p;

		// go to the start of the next var
		zfg_cereal_text_scan_for (&p, '\n');
		p++;

		if (zfg_cereal.vars_next >= ZFG_CEREAL_MAX_VARS) {
			zfg_cereal_print_error
			("cannot decode var not out of var slots");
			return;
		}

		// append record
		zfg_cereal.vars[zfg_cereal.vars_next++] = v;
	}
}

// Untested.
void zfg_cereal_read_file (const char *path) {
	FILE *f = fopen (path, "r");

	if (f == NULL) {
		// file probably doesnt exist
		// TODO: check errno ?
		return; // 0;
	}

	zfg_cereal.text_next = // so we dont overwrite it with new text
	fread (zfg_cereal.text, 1, ZFG_CEREAL_MAX_TEXT, f);

	if (zfg_cereal.text_next != 0) {
		zfg_cereal_create_vars_from_text ();
	}

	if (fclose (f) != 0) {
		// TODO: check errno
		zfg_cereal_print_error
		("{fclose} error closing file after reading");
	}

	zfg_cereal.is_loaded = 1;
	return; // 1;
}

// change to var data in text ?
int zfg_cereal_is_pointer_in_text (void *p) {
	char *pp = (char *) p;
	return pp > &zfg_cereal.text[0] && pp
	< &zfg_cereal.text[ZFG_CEREAL_MAX_TEXT];
}

// Untested.
void zfg_cereal_var (zfg_cereal_var_t *var) {
	zfg_cereal_var_t *v = zfg_cereal.vars;

	if (!zfg_cereal.is_loaded) {
		zfg_cereal_read_file (ZFG_CEREAL_FILE_NAME);
	}

	// check for var in vars:
	for (int i = 0; i < ZFG_CEREAL_MAX_VARS; i++, v++) {
		if (v->size != var->size)
			continue;

		if (!zfg_cereal_str_eq (v->id1, var->id1))
			continue;

		if (!zfg_cereal_str_eq (v->id2, var->id2))
			continue;

		// if (v->data != var->data) {
		if (zfg_cereal_is_pointer_in_text (v->data)) {
			// decode var
			zfg_cereal_hex_decode (v->data, var->data, v->size);
			// update pointer
			v->data = var->data;
			// exit
			return;
		}
		// otherwise:
		//	exit
		return;
	}

	// is there not any space in vars:
	if (zfg_cereal.vars_next >= ZFG_CEREAL_MAX_VARS) {
		// print error message
		zfg_cereal_print_error
		("cannot store new var, no remaining var slots");
		// exit
		return;
	}
	// is there space in text:
	if (zfg_cereal_text_write_var (var) == 1) {
		// encode text
		// append var
		zfg_cereal.vars[zfg_cereal.vars_next] = *var;
		zfg_cereal.vars_next++;
		// exit
		return;
	}
	// otherwise:
	//	print error message
	zfg_cereal_print_error
	("cannot store new var, not enough remaining text space");
	//	exit
	return;
}

// Untested.
void zfg_cereal_write_file_ex (const char *path) {
	FILE *f = fopen (path, "w");

	if (f == NULL) {
		// TODO: check errno ?
		zfg_cereal_print_error
		("{fopen} unable to opening file for writing");
		return;
	}

	// update text with new data
	zfg_cereal.text_next = 0;
	for (int i = 0; i < zfg_cereal.vars_next; i++)
		if (!zfg_cereal_is_pointer_in_text (zfg_cereal.vars[i].data))
			zfg_cereal_text_write_var (&zfg_cereal.vars[i]);

	if (fwrite (zfg_cereal.text, 1, zfg_cereal.text_next, f)
	< zfg_cereal.text_next) {
		zfg_cereal_print_error
		("{fwrite} correct number of bytes was not written to file");
	}

	if (fclose (f) != 0) {
		// TODO: check errno
		zfg_cereal_print_error
		("{fclose} error closing file after writing");
		return;
	}
}

void zfg_cereal_write_file (void) {
	zfg_cereal_write_file_ex (ZFG_CEREAL_FILE_NAME);
}

#endif // ZFG_CEREAL_IMPLEMENTATION

#ifdef ZFG_CEREAL_INCLUDE_TESTS

// Good.
void zfg_cereal_hex_encode_decode_test (void) {
	char hex[20] = { 0 };

	for (unsigned short a = 0; a < (1 << 16) - 1; a++) {
		unsigned short b = 0;

		zfg_cereal_hex_encode (&a, hex, sizeof (a));
		zfg_cereal_hex_decode (hex, &b, sizeof (b));

		if (a != b) {
			printf ("\rfailed %i %i\n", (int) a, b);
			return;
		}

		printf ("\r passed %i %i", (int) a, b);
	}

	printf ("\npassed\n");
}

// Good.
void zfg_cereal_hex_bad_data_test (void) {
	unsigned short a;
	unsigned short b;
	const char *hex[] = {
		// whitespace
		"0c00", "0c 00",
		// letters
		"ffff", "ff putff",
		// special chars
		"1010", "\0\n10\n10\t"
	};

	for (int i = 0; i < sizeof (hex) / sizeof (hex[0]); i += 2) {
		zfg_cereal_hex_decode (hex[i], &a, sizeof (a));
		zfg_cereal_hex_decode (hex[i + 1], &b, sizeof (b));

		if (a != b) {
			printf ("failed %s :: %s\n", hex[i], hex[i + 1]);
			return;
		}
	}
	printf ("passed\n");
}

// Good.
void zfg_cereal_run_tests (void) {
	printf ("[zfg_cereal hex encode decode test]\n");
	zfg_cereal_hex_encode_decode_test ();
	printf ("[zfg_cereal hex bad data test]\n");
	zfg_cereal_hex_bad_data_test ();
}

#endif // ZFG_CEREAL_INCLUDE_TESTS
