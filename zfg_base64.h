// zfg_base64.h

/*
 * Notes:
 *
 * Make sure that if an empty input is passed to an encoder
 * it outputs AA==, problems could happen if there were
 * more than two padding characters
*/

#ifndef ZFG_BASE64_H_INCLUDED
#define ZFG_BASE64_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

int zfg_base64_encode_ex
(const unsigned char *, char *, int, const char *);

int zfg_base64_encode
(const unsigned char *, char *, int);

int zfg_base64_decode_ex
(const char *, unsigned char *, int, const char *);

int zfg_base64_decode
(const char *, unsigned char *, int);

#ifdef __cplusplus
}
#endif

#endif // ZFG_BASE64_H_INCLUDED

#ifdef ZFG_BASE64_IMPLEMENTATION

// TODO: Use ? op on last 2 bytes for padding instead of loop

int zfg_base64_encode_ex
(const unsigned char *in, char *out, int tg, const char *b64)
{
	unsigned char i3[3] = { 0 }, o4[4] = { 0 };
	char *out2 = out;

	do {
		int pad = 0;

		for (int i = 0; i < 3; i++, tg--)
			i3[i] = tg > 0 ? *in++ : (pad++, 0);

		o4[0] = b64[ i3[0] >> 2 ];
		o4[1] = b64[ ((3 & i3[0]) << 4) | (i3[1] >> 4) ];
		o4[2] = b64[ ((15 & i3[1]) << 2) | ((192 & i3[2]) >> 6) ];
		o4[3] = b64[ 63 & i3[2] ];

		pad = pad > 2 ? 2 : pad;
		for (; pad > 0; pad--)
			o4[4 - pad] = b64[64];

		for (int i = 0; i < 4; i++)
			*out++ = o4[i];

	} while (tg > 0);

	return (int) (out - out2);
}

int zfg_base64_encode
(const unsigned char *in, char *out, int tg)
{
	static const char *b64 =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/=";

	return zfg_base64_encode_ex (in, out, tg, b64);
}

// TODO: Rewrite so that read isnt the total number of chars read
// but instead keep track of the number of bytes to print out
// in the decoding loop and reset each loop

/*
// TODO: Improve loc on decoding char by using a loop + array
// for the checks

start end offset \x1a \x34

for (int b = 0; b < len; b += 3)
	i4[i] = (*in >= b64[i] && *in <= b64[i + 1]) ?
	(b = 64, f = 0, b64[i + 3] + *in - b64[i]) : (f = 1);
*/

int zfg_base64_decode_ex
(const char *in, unsigned char *out, int tg, const char *b64)
{
	unsigned char *out2 = out;
	int read = 0;

	do {
		unsigned char i4[4] = { 0 }, o3[3] = { 0 };

		for (int r = read, f = 0; read < r + 4 && tg > 0; tg--, in++) {
			i4[read - r] =
			(*in >= b64[0] & *in <= b64[1]) ?      *in - b64[0] :
			(*in >= b64[2] & *in <= b64[3]) ? 26 + *in - b64[2] :
			(*in >= b64[4] & *in <= b64[5]) ? 52 + *in - b64[4] :
			(*in == b64[6]) ? 62 :
			(*in == b64[7]) ? 63 :
			(f++, 0);

			if (!f)
				read++;

			f = 0;
		}

		o3[0] = (i4[0] << 2) | (i4[1] >> 4);
		o3[1] = (i4[1] << 4) | (i4[2] >> 2);
		o3[2] = (i4[2] << 6) | (i4[3]);

		for (int i = 0; i < ((read - 1) % 4); i++)
			*out++ = o3[i];

	} while (tg > 0);

	return (int) (out - out2);
}

int zfg_base64_decode
(const char *in, unsigned char *out, int tg)
{
	const char *b64 = "AZaz09+/";
	// const char *b64 = "AZ\x00" "az\x1a" "09\x34" "++\x3E" "//\x3F";

	return zfg_base64_decode_ex (in, out, tg, b64);
}

#endif // ZFG_BASE64_IMPLEMENTATION

#ifdef ZFG_BASE64_TESTS

int zfg_base64_run_tests
(int print)
{
	// encode, decode then compare decode to original
	const char *ascii[] = {
		"Hello base64.",
		"the cake is good but also a lie",
		"J0kgYW0ga2luZyBzaGl0IG9mIGZ1Y2sgbW"
		"91bnRhaW4gZGFuIGF2aWRhbiEiIH4gRGFu"
		"IEF2aWRhbiwgR2FtZSBHcnVtcHMn"
	};
	// decode, encode, decode then compare decode to decode
	const char *base64[] = { // decode re encode, then decode again
		"U2VhZ3VsIGNyYXA=",
		"VHdpc3RlZCB0YWlsLCBBIDEwMDAgZXllcy4uLg==",
		"SSBy .___. ZWFsbHkgaG9wZSB0aGVzZ\n"
		"SB0Z\tXN0cy Bkb250IGZhaWwu"
	};

	// TODO: Finish tests

	return 0;
}

#endif // ZFG_BASE64_TESTS
