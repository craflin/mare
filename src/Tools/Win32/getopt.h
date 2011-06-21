
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern char *optarg; /* Global argument pointer. */
extern int optind; /* Global argv index. */
extern int getopt(int argc, char *argv[], const char *optstring);

#ifdef __cplusplus
};
#endif
