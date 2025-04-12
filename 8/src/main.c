#include <stdio.h>
#include <sys/stat.h>

#include "parser.h"
#include "init.h"

int main(int argc, char *argv[])
{
  // if there are no files to parse, success!
  if (argc < 2)
  {
    return 0;
  }

  struct stat path_stat;
  char *path = argv[1];

  if (stat(path, &path_stat) != 0)
  {
    perror("stat failed");
    return 1;
  }

  if (S_ISREG(path_stat.st_mode))
  {
    // fill the data
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
      perror("Error opening file");
      return 1;
    }

    init();
    int code = parse(file);
    return code;
  }
  else if (S_ISDIR(path_stat.st_mode))
  {
    printf("It's a directory.\n");
    return 1;
  }
  else
  {
    printf("It's something else.\n");
    return 1;
  }
}