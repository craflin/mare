
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern char *optarg; /* pointer to current option argument */
extern int optind; /* index for first non-option arg     */
extern int opterr; /* enable built-in error messages     */
extern int optopt; /* return value for option being evaluated   */

struct option
{
  const char* name;
  int has_arg;
  int* flag;
  int val;
};

#define	no_argument		0
#define required_argument	1
#define optional_argument	2

int getopt(int argc, char* const argv[], const char* optstring);
int getopt_long(int argc, char* const argv[], const char* optstring, const struct option* opts, int* index);
int getopt_long_only(int argc, char* const argv[], const char* optstring, const struct option* opts, int* index);

#ifdef __cplusplus
};
#endif
