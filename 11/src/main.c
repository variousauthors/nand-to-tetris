#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <_stdio.h>
#include <libgen.h>

#include "parser.h"
#include "init.h"
#include <sys/syslimits.h>

char *currentFile;

void makeOutFileName(const char *filename)
{
  char *dot = strrchr(filename, '.');
  if (dot != NULL)
  {
    *dot++ = '.';
    *dot++ = 'v';
    *dot++ = 'm';
    *dot++ = '\0';
  }

  return;
}

void getModuleName(const char *filename, char *out, size_t outlen)
{
  strncpy(out, filename, outlen);

  char *dot = strrchr(out, '.');
  if (dot != NULL)
  {
    *dot = '\0'; // truncate at the last dot
  }

  return;
}

bool isVMFile(char *name, int namelen)
{
  return name[namelen - 1] == 'k' && name[namelen - 2] == 'c' && name[namelen - 3] == 'a' && name[namelen - 4] == 'j' && name[namelen - 5] == '.';
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
    char moduleName[PATH_MAX];
    getModuleName(basename(path), moduleName, PATH_MAX);

    char out_path[PATH_MAX];
    snprintf(out_path, sizeof(out_path), "%s/%s", path, "");

    currentFile = moduleName;

    fprintf(stderr, "processing %s -> %s\n", path, out_path);
    return parseSingleFile(path);
  }
  else if (S_ISDIR(path_stat.st_mode))
  {
    // we'll output the bootstrap code
    // emitBootstrap();

    // then parse each file
    DIR *dir = opendir(path);

    if (dir == NULL)
    {
      perror("Unable to open directory");
      return 1;
    }

    // first process Main.jack and if it is missing
    // we abort. Main needs to be the first namespace
    // in the VM code is why
    int code = 0;

    char moduleName[PATH_MAX];
    getModuleName("Main.jack", moduleName, PATH_MAX);

    currentFile = moduleName;
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, "Main.jack");

    fprintf(stderr, "processing %s\n", full_path);
    code |= parseSingleFile(full_path);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
      // Skip . and .. (current and parent directories)
      if (entry->d_name[0] == '.') {
        continue;
      }

      // skip files that do not end in .jack
      if (!isVMFile(entry->d_name, entry->d_namlen))
      {
        continue;
      }

      char moduleName[PATH_MAX];
      getModuleName(entry->d_name, moduleName, PATH_MAX);

      // skip Main.jack because we already did it
      if (strcmp("Main", moduleName) == 0)
      {
        continue;
      }

      currentFile = moduleName;
      char full_path[PATH_MAX];
      snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

      fprintf(stderr, "processing %s\n", full_path);
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