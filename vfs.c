/*
 * Copyright(C) 2012 Red Hat
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "vfs.h"

static void
write_name (FILE *out,
            char *name)
{
  int8_t name_length;

  name_length = strlen (name);
  fwrite (&name_length, 1, 1, out);
  fwrite (name, name_length, 1, out);
}

static char *
read_name (FILE *in)
{
  int8_t name_length;
  char *name;

  fread (&name_length, 1, 1, in);

  name = malloc (name_length + 1);
  fread (name, name_length, 1, in);
  name[name_length] = '\0';

  return name;
}

CargoVFSFileEntry *
cargo_vfs_file_entry_read (CargoVFSDirectoryEntry *parent,
                           FILE                   *in)
{
  CargoVFSFileEntry *entry;

  entry = malloc (sizeof (CargoVFSFileEntry));
  entry->parent = parent;
  entry->name = read_name (in);

  fread (&entry->offset, 4, 1, in);
  fread (&entry->unknown_1, 4, 1, in);
  fread (&entry->length, 4, 1, in);

  entry->next = NULL;
  return entry;
}

void
cargo_vfs_file_entry_write (CargoVFSFileEntry *entry,
                            FILE              *out)
{
  write_name (out, entry->name);
  fwrite (&entry->offset, 4, 1, out);
  fwrite (&entry->unknown_1, 4, 1, out);
  fwrite (&entry->length, 4, 1, out);
}

void
cargo_vfs_file_entry_extract (CargoVFSFileEntry *entry,
                              char              *parent,
                              FILE              *in)
{
  size_t i;
  char *path;
  char buf[1024];
  FILE *out;

  path = cargo_util_build_path (parent, entry->name);
  out = fopen (path, "wb");

  fseek (in, entry->offset, SEEK_SET);

  i = entry->length;
  while (i >= 1024)
    {
      fread (buf, 1024, 1, in);
      fwrite (buf, 1024, 1, out);
      i -= 1024;
    }

  fread (buf, i, 1, in);
  fwrite (buf, i, 1, out);

  fclose (out);
  free (path);
}

char *
cargo_vfs_file_entry_path (CargoVFSFileEntry *entry)
{
  return cargo_util_build_path (cargo_vfs_directory_entry_path (entry->parent),
                                entry->name);
}



CargoVFSDirectoryEntry *
cargo_vfs_directory_entry_read (CargoVFSDirectoryEntry *parent,
                                FILE                   *in,
                                int                     has_name)
{
  CargoVFSDirectoryEntry *entry;
  CargoVFSFileEntry **currfile;
  CargoVFSDirectoryEntry **currdir;
  int32_t i;

  entry = malloc (sizeof (CargoVFSDirectoryEntry));
  entry->parent = parent;
  entry->name = has_name ? read_name (in) : NULL;
  fread (&entry->subdir_count, 4, 1, in);
  fread (&entry->file_count, 4, 1, in);

  currfile = &entry->files;
  for (i = 0; i < entry->file_count; i ++)
    {
      *currfile = cargo_vfs_file_entry_read (parent, in);
      currfile = &((*currfile)->next);
    }

  currdir = &entry->subdirs;
  for (i = 0; i < entry->subdir_count; i ++)
    {
      /* all subdirs have names -- only toplevel doesn't */
      *currdir = cargo_vfs_directory_entry_read (parent, in, 1);
      currdir = &((*currdir)->next);
    }

  entry->next = NULL;

  return entry;
}

void
cargo_vfs_directory_entry_write (CargoVFSDirectoryEntry *entry,
                                 FILE                   *out)
{
  CargoVFSFileEntry *file;
  CargoVFSDirectoryEntry *subdir;

  if (entry->name)
    write_name (out, entry->name);

  fwrite (&entry->subdir_count, 4, 1, out);
  fwrite (&entry->file_count, 4, 1, out);

  for (file = entry->files; file != NULL; file = file->next)
    cargo_vfs_file_entry_write (file, out);

  for (subdir = entry->subdirs; subdir != NULL; subdir = subdir->next)
    cargo_vfs_directory_entry_write (subdir, out);
}

void
cargo_vfs_directory_entry_extract (CargoVFSDirectoryEntry *entry,
                                   char                   *parent,
                                   FILE                   *in)
{
  CargoVFSFileEntry *file;
  CargoVFSDirectoryEntry *subdir;
  char *path;

  path = cargo_util_build_path (parent, entry->name);

  if (cargo_util_ensure_directory_exists (path) == -1)
    return;

  for (file = entry->files; file != NULL; file = file->next)
    cargo_vfs_file_entry_extract (file, path, in);

  for (subdir = entry->subdirs; subdir != NULL; subdir = subdir->next)
    cargo_vfs_directory_entry_extract (subdir, path, in);

  free (path);
}

CargoVFSFileEntry *
cargo_vfs_directory_entry_get_file (CargoVFSDirectoryEntry *entry,
                                    char                   *filename)
{
  CargoVFSFileEntry *file;

  for (file = entry->files; file != NULL; file = file->next)
    if (strcmp (filename, file->name) == 0)
      return file;

  return NULL;
}

void
cargo_vfs_directory_entry_append_file (CargoVFSDirectoryEntry *entry,
                                       CargoVFSFileEntry      *file)
{
  if (entry->files != NULL)
    {
      CargoVFSFileEntry *curr;
      curr = entry->files;
      while (curr->next != NULL)
        curr = curr->next;
      curr->next = file;
    }
  else
    entry->files = file;

  file->parent = entry;
  entry->file_count ++;
}

void
cargo_vfs_directory_entry_append_subdir (CargoVFSDirectoryEntry *entry,
                                         CargoVFSDirectoryEntry *subdir)
{
  if (entry->subdirs != NULL)
    {
      CargoVFSDirectoryEntry *curr;
      curr = entry->subdirs;
      while (curr->next != NULL)
        curr = curr->next;
      curr->next = subdir;
    }
  else
    entry->subdirs = subdir;

  subdir->parent = entry;
  entry->subdir_count ++;
}

CargoVFSFileEntry *
cargo_vfs_directory_entry_get_path (CargoVFSDirectoryEntry *entry,
                                    char                   *path)
{
  CargoVFSDirectoryEntry *subdir;
  size_t len;
  char *slash;

  slash = strstr (path, "/");

  /* No slash, so do a file lookup instead. */
  if (slash == NULL)
    return cargo_vfs_directory_entry_get_file (entry, path);

  len = slash - path;
  for (subdir = entry->subdirs; subdir != NULL; subdir = subdir->next)
    if (strncmp (path, subdir->name, len) == 0)
      return cargo_vfs_directory_entry_get_path (subdir, slash + 1);

  return NULL;
}

char *
cargo_vfs_directory_entry_path (CargoVFSDirectoryEntry *entry)
{
  if (entry->parent == NULL)
    return (entry->name != NULL) ? strdup (entry->name) : NULL;

  return cargo_util_build_path (cargo_vfs_directory_entry_path (entry->parent),
                                entry->name);
}


CargoVFSDirectoryEntry *
cargo_vfs_read_vfs_file (FILE *in)
{
  char magic[4];

  fread (magic, 4, 1, in);

  if (strncmp (magic, CARGO_VFS_FILE_MAGIC, 4) != 0)
    return NULL;

  return cargo_vfs_directory_entry_read (NULL, in, 0);
}

void
cargo_vfs_write_vfs_file (CargoVFSDirectoryEntry *toplevel,
                          FILE                   *out)
{
  fwrite (CARGO_VFS_FILE_MAGIC, 4, 1, out);
  cargo_vfs_directory_entry_write (toplevel, out);
}
