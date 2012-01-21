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

#include <errno.h>
#include <malloc.h>
#include <string.h>

#include "vfs.h"

static inline void
print_indent (int indent)
{
  while (indent --)
    printf (" ");
}

static void
list_directory (CargoVFSDirectoryEntry *entry,
                int                     indent)
{
  CargoVFSDirectoryEntry *subdir;
  CargoVFSFileEntry *file;

  for (subdir = entry->subdirs; subdir != NULL; subdir = subdir->next)
    {
      print_indent (indent);
      printf ("+ %s\n", subdir->name);
      list_directory (subdir, indent + 2);
    }

  for (file = entry->files; file != NULL; file = file->next)
    {
      print_indent (indent);
      printf ("- %s\n", file->name);
    }
}

int
main (int argc, char **argv)
{
  char *filename;
  CargoVFSDirectoryEntry *toplevel;
  FILE *in;

  if (argc == 1)
    {
      fprintf (stderr, "Needs a filename.\n");
      return 1;
    }

  filename = argv[1];

  in = fopen (filename, "rb");
  if (in == NULL)
    {
      fprintf (stderr, "Could not open file: %s.\n", strerror (errno));
      return 1;
    }

  toplevel = cargo_vfs_read_vfs_file (in);
  list_directory (toplevel, 0);

  fclose (in);

  return 0;
}
