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

#ifndef __CARGO_VFS_VFS_H__
#define __CARGO_VFS_VFS_H__

#include <stdint.h>

#define CARGO_VFS_FILE_MAGIC "LP3C"

typedef struct _CargoVFSDirectoryEntry CargoVFSDirectoryEntry;
typedef struct _CargoVFSFileEntry CargoVFSFileEntry;

struct _CargoVFSFileEntry
{
  CargoVFSDirectoryEntry *parent;
  char *name;
  int32_t offset;
  int32_t unknown_1;
  int32_t length;
  CargoVFSFileEntry *next;
};

CargoVFSFileEntry * cargo_vfs_file_entry_read (CargoVFSDirectoryEntry *parent,
                                               FILE                   *in);
void cargo_vfs_file_entry_write   (CargoVFSFileEntry *entry,
                                   FILE      *out);
void cargo_vfs_file_entry_extract (CargoVFSFileEntry *entry,
                                   char      *parent,
                                   FILE      *in);
char * cargo_vfs_file_entry_path (CargoVFSFileEntry *entry);

struct _CargoVFSDirectoryEntry
{
  CargoVFSDirectoryEntry *parent;
  char *name;
  int32_t subdir_count;
  int32_t file_count;
  CargoVFSFileEntry *files;
  CargoVFSDirectoryEntry *subdirs;
  CargoVFSDirectoryEntry *next;
};

CargoVFSDirectoryEntry * cargo_vfs_directory_entry_read (CargoVFSDirectoryEntry *parent,
                                                         FILE                   *in,
                                                         int                     has_name);
void cargo_vfs_directory_entry_write (CargoVFSDirectoryEntry *entry,
                                      FILE                   *out);

void cargo_vfs_directory_entry_extract (CargoVFSDirectoryEntry *entry,
                                        char                   *parent,
                                        FILE                   *in);
char * cargo_vfs_directory_entry_path (CargoVFSDirectoryEntry *entry);

CargoVFSFileEntry * cargo_vfs_directory_entry_get_file (CargoVFSDirectoryEntry *entry,
                                                        char                   *filename);
CargoVFSFileEntry * cargo_vfs_directory_entry_get_path (CargoVFSDirectoryEntry *entry,
                                                        char                   *path);
void cargo_vfs_directory_entry_append_subdir (CargoVFSDirectoryEntry *entry,
                                              CargoVFSDirectoryEntry *subdir);
void cargo_vfs_directory_entry_append_file (CargoVFSDirectoryEntry *entry,
                                            CargoVFSFileEntry      *file);


CargoVFSDirectoryEntry * cargo_vfs_read_vfs_file  (FILE *in);
void cargo_vfs_write_vfs_file (CargoVFSDirectoryEntry *toplevel,
                               FILE                   *out);

#endif /* __CARGO_VFS_VFS_H__ */
