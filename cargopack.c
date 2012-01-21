/*
 * Copyright (C) 2012 Red Hat
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "utils.h"
#include "vfs.h"

static CargoVFSDirectoryEntry *
construct_from_dir (char *directory,
                    int   is_toplevel)
{
  CargoVFSDirectoryEntry *entry;
  DIR *dirp;
  struct dirent *dp;
  struct stat statbuf;
  char *childpath;

  entry = malloc (sizeof (CargoVFSDirectoryEntry));
  entry->subdirs = NULL; entry->files = NULL;
  entry->name = is_toplevel ? NULL : basename (directory);

  dirp = opendir (directory);
  if (dirp == NULL)
    {
      fprintf (stderr, "Could not open directory '%s': %s\n", directory, strerror (errno));
      return NULL;
    }

  while (1)
    {
      dp = readdir (dirp);
      if (dp == NULL)
        break;

      if (strcmp (dp->d_name, ".") == 0)
        continue;
      if (strcmp (dp->d_name, "..") == 0)
        continue;

      childpath = cargo_util_build_path (directory, dp->d_name);
      stat (childpath, &statbuf);
      if (S_ISDIR (statbuf.st_mode))
        cargo_vfs_directory_entry_append_subdir (entry, construct_from_dir (childpath, 0));
      else
        {
          CargoVFSFileEntry *file;
          file = malloc (sizeof (CargoVFSFileEntry));
          file->name = strdup (dp->d_name);
          file->unknown_1 = 0;
          cargo_vfs_directory_entry_append_file (entry, file);
        }

      free (childpath);
    }
  return entry;
}

static void
write_file (CargoVFSFileEntry *entry,
            char              *directory,
            FILE              *out)
{
  char buf[1024];
  char *temp, *path;
  size_t i;
  struct stat statbuf;
  FILE *in;

  temp = cargo_vfs_file_entry_path (entry);
  path = cargo_util_build_path (directory, temp);
  free (temp);

  stat (path, &statbuf);

  in = fopen (path, "rb");
  entry->length = i = statbuf.st_size;
  entry->offset = ftell (out);
  while (i >= 1024)
    {
      fread (buf, 1024, 1, in);
      fwrite (buf, 1024, 1, out);
      i -= 1024;
    }

  fread (buf, i, 1, in);
  fwrite (buf, i, 1, out);

  fclose (in);
  free (path);
}

static void
write_files (CargoVFSDirectoryEntry *entry,
             char                   *directory,
             FILE                   *out)
{
  CargoVFSDirectoryEntry *subdir;
  CargoVFSFileEntry *file;

  for (file = entry->files; file != NULL; file = file->next)
    write_file (file, directory, out);

  for (subdir = entry->subdirs; subdir != NULL; subdir = subdir->next)
    write_files (subdir, directory, out);
}

int
main (int argc, char **argv)
{
  CargoVFSDirectoryEntry *toplevel;
  FILE *out;

  if (argc != 3)
    {
      fprintf (stderr, "Usage: %s directory out\n", argv[0]);
      return 1;
    }

  toplevel = construct_from_dir (argv[1], 1);

  out = fopen(argv[2], "wb");

  /* XXX: a bit hacky
   * we write the headers twice -- once to just
   * calculate the sizes of everything so we know
   * the offsets, and another time for real */
  cargo_vfs_write_vfs_file (toplevel, out);
  write_files (toplevel, argv[1], out);
  fseek (out, 0, SEEK_SET);
  cargo_vfs_write_vfs_file (toplevel, out);
  fclose (out);

  return 0;
}
