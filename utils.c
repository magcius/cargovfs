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
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>

char *
cargo_util_build_path (char *s1, char *s2)
{
  char *destination;
  size_t s1_len, s2_len;

  if (s1 == NULL || *s1 == '\0')
      return strdup (s2);

  if (s2 == NULL || *s2 == '\0')
      return strdup (s1);

  s1_len = strlen (s1); s2_len = strlen (s2);

  /* one for the slash, one for the null byte */
  destination = malloc (s1_len + 1 + s2_len + 1);

  strcpy (destination, s1);
  destination[s1_len] = '/';
  strcpy (destination + s1_len + 1, s2);
  destination[s1_len + 1 + s2_len] = '\0';

  return destination;
}

int
cargo_util_ensure_directory_exists (char *directory)
{
  if (mkdir (directory, 0775) == -1)
    {
      if (errno != EEXIST)
        {
          return -1;
        }
    }
  return 0;
}
