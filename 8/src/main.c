#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <_stdio.h>

#include "parser.h"
#include "init.h"
#include <sys/syslimits.h>

bool isVMFile(char *name, int namelen)
{
  return name[namelen - 1] == 'm' || name[namelen - 2] == 'v' || name[namelen - 3] == '.';
}

int parseSingleFile(char *path)
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
  fclose(file);
  return code;
}

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
    return parseSingleFile(path);
  }
  else if (S_ISDIR(path_stat.st_mode))
  {
    // we'll output the bootstrap code
    printf("@256\n");
    printf("D=A\n");
    printf("@SP\n");
    printf("A=M\n");
    printf("M=D\n"); // SP = 256

    printf("");

    // then parse each file
    DIR *dir = opendir(path);

    if (dir == NULL)
    {
      perror("Unable to open directory");
      return 1;
    }

    int code = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
      // Skip . and .. (current and parent directories)
      if (entry->d_name[0] == '.')
        continue;

      // skip files that do not end in .vm
      if (!isVMFile(entry->d_name, entry->d_namlen))
      {
        continue;
      }

      char full_path[PATH_MAX];
      snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

      code |= parseSingleFile(full_path);
    }

    closedir(dir);
    return 0;
  }
  else
  {
    printf("It's something else.\n");
    return 1;
  }
}