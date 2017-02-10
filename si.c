#if 0
(
set -euo pipefail
declare -r tmp="$(mktemp)"
gcc -std=gnu11 -Wall -Wextra -Werror -I./c_modules -DSIMPLE_LOGGING -DTEST_si -lm -O0 -g -o "$tmp" $(find -name '*.c')
valgrind --quiet --leak-check=full --track-origins=yes "$tmp"
)
exit 0
#endif
#include "si.h"

struct si_prefix si_prefixes[si_prefix_count] = {
	{ .prefix = "Y", .exponent = 24, .scale = 1e24 },
	{ .prefix = "Z", .exponent = 21, .scale = 1e21 },
	{ .prefix = "E", .exponent = 18, .scale = 1e18 },
	{ .prefix = "P", .exponent = 15, .scale = 1e15 },
	{ .prefix = "T", .exponent = 12, .scale = 1e12 },
	{ .prefix = "G", .exponent = 9, .scale = 1e9 },
	{ .prefix = "M", .exponent = 6, .scale = 1e6 },
	{ .prefix = "k", .exponent = 3, .scale = 1e3 },
	{ .prefix = "ha", .exponent = 2, .scale = 1e2, .uncommon = true },
	{ .prefix = "da", .exponent = 1, .scale = 1e1, .uncommon = true },
	{ .prefix = "", .exponent = 0, .scale = 1e0 },
	{ .prefix = "d", .exponent = -1, .scale = 1e-1, .uncommon = true },
	{ .prefix = "c", .exponent = -2, .scale = 1e-2, .uncommon = true },
	{ .prefix = "m", .exponent = -3, .scale = 1e-3 },
	{ .prefix = "u", .exponent = -6, .scale = 1e-6 },
	{ .prefix = "n", .exponent = -9, .scale = 1e-9 },
	{ .prefix = "p", .exponent = -12, .scale = 1e-12 },
	{ .prefix = "f", .exponent = -15, .scale = 1e-15 },
	{ .prefix = "a", .exponent = -18, .scale = 1e-18 },
	{ .prefix = "z", .exponent = -21, .scale = 1e-21 },
	{ .prefix = "y", .exponent = -24, .scale = 1e-24 },
	{ .prefix = "!", .exponent = 0, .scale = 0 }
};

static const struct si_prefix *unit = &si_prefixes[10];

const char *si_noprefix_fmt = "%.*E %s";
const char *si_prefix_fmt = "%'.*f %s%s";

double si_scale(double x, const char **prefix)
{
	if (!isfinite(x)) {
		*prefix = NULL;
		return x;
	}
	if (x == 0) {
		*prefix = unit->prefix;
		return x;
	}
	bool neg = x < 0;
	x = fabs(x);
	bool automatic = *prefix == NULL;
	struct si_prefix *p;
	for (p = si_prefixes; p->scale != 0; p++) {
		if (automatic) {
			if (!p->uncommon) {
				double mant = x / p->scale;
				if (mant < 1e3 && mant >= 1) {
					*prefix = p->prefix;
					return neg ? -mant : mant;
				}
			}
		} else {
			if (strcmp(p->prefix, *prefix) == 0) {
				return (neg ? -x : x) / p->scale;
			}
		}
	}
	*prefix = NULL;
	return neg ? -x : x;
}

double si_sigfig(double x, int sf, int *precision)
{
	if (!isfinite(x)) {
		if (precision) {
			*precision = 0;
		}
		return x;
	}
	if (x == 0) {
		if (precision) {
			*precision = sf - 1;
		}
		return x;
	}
	/* Digits left of radix */
	int dl = (int) floor(log10(fabs(x))) + 1;
	/* Digits to cut */
	int cut = dl - sf;
	/* Adjust printf precision */
	if (precision != NULL) {
		*precision = sf > dl ? sf - dl : 0;
	}
	/* Trim number */
	double trim = pow(10, cut);
	return round(x / trim) * trim;
}

static void prepare(double *x, int *precision, const char **prefix, const char **unit)
{
	*x = si_scale(*x, prefix);
	if (*precision < 0) {
		*x = si_sigfig(*x, -*precision, *prefix ? precision : NULL);
	}
	*unit = *unit ? *unit : "";
}

void si_format(double x, struct fstr *out, int precision, const char *prefix, const char *unit)
{
	prepare(&x, &precision, &prefix, &unit);
	if (prefix == NULL) {
		fstr_format(out, si_noprefix_fmt, precision, x, unit ? unit : "");
	} else {
		fstr_format(out, si_prefix_fmt, precision, x, prefix, unit ? unit : "");
	}
}

#if defined TEST_si
int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	struct val {
		double x;
		const char *prefix;
	};
	const struct val values[] = {
		{ .x = 1 },
		{ .x = 0.1234, .prefix = "" },
		{ .x = 1234, .prefix = "" },
		{ .x = 3.45678e-10 },
		{ .x = 3.45678e10 },
		{ .x = 3.45678e-10, .prefix = "" },
		{ .x = 3.45678e10, .prefix = "" },
		{ .x = -3.45678e10 },
		{ .x = -3.45678e-10 },
		{ .x = -3.45678e30 },
		{ .x = -3.45678e-30 },
		{ .x = -3.45678e30 },
		{ .x = -3.45678e-30 },
		{ .x = -3.45678e30, .prefix="Z" },
		{ .x = -3.45678e-30, .prefix="z" },
		{ .x = -3.45678e26 },
		{ .x = -3.45678e-24 },
		{ .x = +0 },
		{ .x = -0 },
		{ .x = INFINITY },
		{ .x = -INFINITY },
		{ .x = NAN },
		{ .x = -NAN },
	};
	struct fstr fs;
	fstr_init(&fs);
	for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); i++) {
		const struct val *v = &values[i];
		si_format(v->x, &fs, -3, v->prefix, "Ω");
		printf("#%zd: " PRIfs "\n", i, prifs(&fs));
	}
	fstr_destroy(&fs);
	return 0;
}
#endif
