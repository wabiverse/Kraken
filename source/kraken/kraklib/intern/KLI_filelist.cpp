/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include <stdlib.h> /* malloc */
#include <string.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>

#include <zlib.h>

#ifdef WIN32
#  include <windows.h>

#  include "utfconv.h"
#  include <io.h>
#  include <shellapi.h>
#else
#  if defined(__APPLE__)
#    include <CoreFoundation/CoreFoundation.h>
#    include <objc/message.h>
#    include <objc/runtime.h>
#  endif
#  include <dirent.h>
#  include <sys/param.h>
#  include <sys/wait.h>
#  include <unistd.h>
#endif

#include "KLI_fileops.h"
#include "KLI_path_utils.h"
#include "KLI_string_utils.h"
#include "KLI_utildefines.h"

#ifdef WIN32
#  include "KLI_winstuff.h"
#endif /* WIN32 */


WABI_NAMESPACE_BEGIN

struct BuildDirCtx
{
  struct direntry *files; /* array[nrfiles] */
  int nrfiles;
};

/*
 * Ordering function for sorting lists of files/directories. Returns -1 if
 * entry1 belongs before entry2, 0 if they are equal, 1 if they should be swapped.
 */
static int kli_compare(struct direntry *entry1, struct direntry *entry2)
{
  /* type is equal to stat.st_mode */

  /* directories come before non-directories */
  if (S_ISDIR(entry1->type))
  {
    if (S_ISDIR(entry2->type) == 0)
    {
      return -1;
    }
  } else
  {
    if (S_ISDIR(entry2->type))
    {
      return 1;
    }
  }
  /* non-regular files come after regular files */
  if (S_ISREG(entry1->type))
  {
    if (S_ISREG(entry2->type) == 0)
    {
      return -1;
    }
  } else
  {
    if (S_ISREG(entry2->type))
    {
      return 1;
    }
  }
  /* arbitrary, but consistent, ordering of different types of non-regular files */
  if ((entry1->type & S_IFMT) < (entry2->type & S_IFMT))
  {
    return -1;
  }
  if ((entry1->type & S_IFMT) > (entry2->type & S_IFMT))
  {
    return 1;
  }

  /* OK, now we know their S_IFMT fields are the same, go on to a name comparison */
  /* make sure "." and ".." are always first */
  if (FILENAME_IS_CURRENT(entry1->relname))
  {
    return -1;
  }
  if (FILENAME_IS_CURRENT(entry2->relname))
  {
    return 1;
  }
  if (FILENAME_IS_PARENT(entry1->relname))
  {
    return -1;
  }
  if (FILENAME_IS_PARENT(entry2->relname))
  {
    return 1;
  }

  return (KLI_strcasecmp_natural(entry1->relname, entry2->relname));
}

/**
 * Scans the directory named *dirname and appends entries for its contents to files. */
static void kli_builddir(struct BuildDirCtx *dir_ctx, const char *dirname)
{
  struct std::vector<void *> dirbase
  {
    NULL, NULL
  };
  int newnum = 0;
  DIR *dir;

  if ((dir = opendir(dirname)) != NULL)
  {
    const struct dirent *fname;
    bool has_current = false, has_parent = false;

    while ((fname = readdir(dir)) != NULL)
    {
      struct dirlink *const dlink = (struct dirlink *)malloc(sizeof(struct dirlink));
      if (dlink != NULL)
      {
        dlink->name = KLI_strdup(fname->d_name);
        if (FILENAME_IS_PARENT(dlink->name))
        {
          has_parent = true;
        } else if (FILENAME_IS_CURRENT(dlink->name))
        {
          has_current = true;
        }
        dirbase.insert(dirbase.begin(), dlink);
        newnum++;
      }
    }

    if (!has_parent)
    {
      char pardir[FILE_MAXDIR];

      KLI_strncpy(pardir, dirname, sizeof(pardir));
      if (KLI_path_parent_dir(pardir) && (KLI_access(pardir, R_OK) == 0))
      {
        struct dirlink *const dlink = (struct dirlink *)malloc(sizeof(struct dirlink));
        if (dlink != NULL)
        {
          dlink->name = KLI_strdup(FILENAME_PARENT);
          dirbase.insert(dirbase.begin(), dlink);
          newnum++;
        }
      }
    }
    if (!has_current)
    {
      struct dirlink *const dlink = (struct dirlink *)malloc(sizeof(struct dirlink));
      if (dlink != NULL)
      {
        dlink->name = KLI_strdup(FILENAME_CURRENT);
        dirbase.insert(dirbase.begin(), dlink);
        newnum++;
      }
    }

    if (newnum)
    {
      if (dir_ctx->files)
      {
        void *const tmp = realloc(dir_ctx->files, (dir_ctx->nrfiles + newnum) * sizeof(struct direntry));
        if (tmp)
        {
          dir_ctx->files = (struct direntry *)tmp;
        } else
        { /* realloc fail */
          free(dir_ctx->files);
          dir_ctx->files = NULL;
        }
      }

      if (dir_ctx->files == NULL)
      {
        dir_ctx->files = (struct direntry *)malloc(newnum * sizeof(struct direntry));
      }

      if (dir_ctx->files)
      {
        struct dirlink *dlink = (struct dirlink *)dirbase.at(0);
        struct direntry *file = &dir_ctx->files[dir_ctx->nrfiles];
        while (dlink)
        {
          char fullname[PATH_MAX];
          memset(file, 0, sizeof(struct direntry));
          file->relname = dlink->name;
          file->path = KLI_strdupcat(dirname, dlink->name);
          KLI_join_dirfile(fullname, sizeof(fullname), dirname, dlink->name);
          if (KLI_stat(fullname, &file->s) != -1)
          {
            file->type = file->s.st_mode;
          } else if (FILENAME_IS_CURRPAR(file->relname))
          {
            /* Hack around for UNC paths on windows:
             * does not support stat on '\\SERVER\foo\..', sigh... */
            file->type |= S_IFDIR;
          }
          dir_ctx->nrfiles++;
          file++;
          dlink = dlink->next;
        }
      } else
      {
        printf("Couldn't get memory for dir\n");
        exit(1);
      }

      std::vector<void *>().swap(dirbase);
      if (dir_ctx->files)
      {
        qsort(dir_ctx->files,
              dir_ctx->nrfiles,
              sizeof(struct direntry),
              (int (*)(const void *, const void *))kli_compare);
      }
    } else
    {
      printf("%s empty directory\n", dirname);
    }

    closedir(dir);
  } else
  {
    printf("%s non-existent directory\n", dirname);
  }
}


/**
 * Scans the contents of the directory named *dirname, and allocates and fills in an
 * array of entries describing them in *filelist.
 *
 * \return The length of filelist array. */
unsigned int KLI_filelist_dir_contents(const char *dirname, struct direntry **r_filelist)
{
  struct BuildDirCtx dir_ctx;

  dir_ctx.nrfiles = 0;
  dir_ctx.files = NULL;

  kli_builddir(&dir_ctx, dirname);

  if (dir_ctx.files)
  {
    *r_filelist = dir_ctx.files;
  } else
  {
    /* Keep Kraken happy. Kraken stores this in a variable
     * where 0 has special meaning..... */
    *r_filelist = (direntry *)malloc(sizeof(**r_filelist));
  }

  return dir_ctx.nrfiles;
}


/**
 * frees storage for a single direntry, not the direntry itself.
 */
void KLI_filelist_entry_free(struct direntry *entry)
{
  if (entry->relname)
  {
    free((void *)entry->relname);
  }
  if (entry->path)
  {
    free((void *)entry->path);
  }
}

/**
 * frees storage for an array of direntries, including the array itself.
 */
void KLI_filelist_free(struct direntry *filelist, const unsigned int nrentries)
{
  unsigned int i;
  for (i = 0; i < nrentries; i++)
  {
    KLI_filelist_entry_free(&filelist[i]);
  }

  if (filelist != NULL)
  {
    free(filelist);
  }
}


WABI_NAMESPACE_END