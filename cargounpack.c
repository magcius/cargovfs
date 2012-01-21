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
#include "utils.h"

int
main (int argc, char **argv)
{
  char *filename;
  char *destination;
  CargoVFSDirectoryEntry *toplevel;
  FILE *in;

  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s vfsfile [filename] [...].\n", argv[0]);
      return 1;
    }

  filename = argv[1];

  in = fopen (filename, "rb");
  if (in == NULL)
    {
      fprintf (stderr, "Could not open file '%s': %s\n", filename, strerror (errno));
      return 1;
    }

  toplevel = cargo_vfs_read_vfs_file (in);

  if (argc == 2)
    {
      destination = cargo_util_get_unpacked_directory (filename);
      cargo_vfs_directory_entry_extract (toplevel, destination, in);
    }
  else
    {
      size_t i;
      for (i = 2; i < argc; i ++)
        {
          CargoVFSFileEntry *entry;
          entry = cargo_vfs_directory_entry_get_path (toplevel, argv[i]);
          if (entry == NULL)
            {
              fprintf (stderr, "Could not find '%s' in '%s'\n", argv[i], filename);
              continue;
            }

          cargo_vfs_file_entry_extract (entry, "", in);
          printf("Extracted '%s'\n", argv[i]);
        }
    }

  fclose (in);

  return 0;
}
