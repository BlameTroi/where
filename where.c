// where -- is a program or script in my $PATH
// t.brumley
//
// it started simple but grew into an exercise to learn
// argp.

#define _GNU_SOURCE

#include <argp.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PATH_DELIM "/"
#define PATH_SEPARATOR ":"

///////////////////////////////////////////////////////////
// for argument parsing ...
const char *argp_program_version = "where v1.0";
const char *argp_program_bug_address = "/dev/null";
const char doc[] = "where -- where is a program or script in the path";
const char args_doc[] = "[script or program name]";
// options
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output", 0}, {0}};
// as parsed
struct arguments {
  int verbose;
  char *fname;
};
// parse a single option
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 'v':
    arguments->verbose = 1;
    break;
  case ARGP_KEY_ARG:
    if (arguments->fname != NULL) {
      argp_error(state, "extraneous: %s", arg);
    }
    arguments->fname = arg;
    break;
  case ARGP_KEY_END:
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
// argument parser configuration
static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

///////////////////////////////////////////////////////////
// main
int main(int argc, char *argv[]) {

  // parse arguments, can exit on error or help request
  struct arguments arguments;
  arguments.verbose = 0;
  arguments.fname = NULL;
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  // there better be a path to search
  char *path = getenv("PATH");
  if (!path) {
    printf("Error: no path");
    return 1;
  }

  // and for each entry in the path, start hunting
  printf("%s:\n", arguments.fname);
  char *pdir = strtok(path, PATH_SEPARATOR);
  while (pdir) {
    struct dirent **namelist;
    int n;
    n = scandir(pdir, &namelist, NULL, alphasort);
    if (n == -1) {
      switch (errno) {
      case ENOENT:
      case ENOTDIR:
        if (arguments.verbose) {
          printf("\tMissing or invalid directory in path: %s\n", pdir);
        }
        break;
      default:
        perror("scandir");
        puts(pdir);
        return 1;
      }
    } else {
      int j;
      for (j = 0; j < n; j++) {
        if (strcmp(namelist[j]->d_name, arguments.fname) == 0) {
          printf("\t%s%s%s\n", pdir, PATH_DELIM, namelist[j]->d_name);
        }
        free(namelist[j]);
      }
      free(namelist);
    }
    // get next directory until exhausted
    pdir = strtok(NULL, PATH_SEPARATOR);
  }

  // complete
  return 0;
}
