#pragma once
#include <cstd/std.h>
#include <fixedstr/fixedstr.h>

/* SI-scaled fixed-precision */

struct si_prefix {
	char prefix[3];
	int exponent;
	double scale;
	bool uncommon;
};

#define si_prefix_count 22
extern struct si_prefix si_prefixes[si_prefix_count];

/*
 * Default si_prefix_fmt has thousands separator requested.
 * To provide thousands separator:
 *   setlocale(LC_ALL, "")
 *
 * (assuming user's locale has thousands separator)
 */
extern const char *si_noprefix_fmt;
extern const char *si_prefix_fmt;

/* Scale value for appropriate prefix */
double si_scale(double x, const char **prefix);

/* Round value to given number of significant figures, return appropriate printf precision */
double si_sigfig(double x, int sf, int *precision);

/*
 * +precision sets number of digits after radix.
 * -precision sets number of significant figures.
 */
void si_format(double x, struct fstr *out, int precision, const char *prefix, const char *unit);
