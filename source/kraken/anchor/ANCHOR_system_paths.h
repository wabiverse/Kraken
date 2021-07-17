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

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#pragma once

#include "ANCHOR_api.h"

class AnchorISystemPaths
{
 public:
  /**
   * Creates the one and only system.
   * @return An indication of success. */
  static eAnchorStatus create();

  /**
   * Disposes the one and only system.
   * @return An indication of success. */
  static eAnchorStatus dispose();

  /**
   * Returns a pointer to the one and only system (nil if it hasn't been created).
   * @return A pointer to the system. */
  static AnchorISystemPaths *get();

 protected:
  /**
   * Constructor.
   * Protected default constructor to force use of static createSystem member.
   */
  AnchorISystemPaths()
  {}

  /**
   * Destructor.
   * Protected default constructor to force use of static dispose member.
   */
  virtual ~AnchorISystemPaths()
  {}

 public:
  /**
   * Determine the base dir in which shared resources are located. It will first try to use
   * "unpack and run" path, then look for properly installed path, including versioning.
   * @return Unsigned char string pointing to system dir (eg /usr/share/kraken/). */
  virtual const AnchorU8 *getSystemDir(int version, const char *versionstr) const = 0;

  /**
   * Determine the base dir in which user configuration is stored, including versioning.
   * If needed, it will create the base directory.
   * @return Unsigned char string pointing to user dir (eg ~/.kraken/). */
  virtual const AnchorU8 *getUserDir(int version, const char *versionstr) const = 0;

  /**
   * Determine a special ("well known") and easy to reach user directory.
   * @return Unsigned char string pointing to user dir (eg `~/Documents/`). */
  virtual const AnchorU8 *getUserSpecialDir(eAnchorUserSpecialDirTypes type) const = 0;

  /**
   * Determine the directory of the current binary
   * @return Unsigned char string pointing to the binary dir */
  virtual const AnchorU8 *getBinaryDir() const = 0;

  /**
   * Add the file to the operating system most recently used files */
  virtual void addToSystemRecentFiles(const char *filename) const = 0;

 private:
  /**
   * The one and only system paths */
  static AnchorISystemPaths *m_systemPaths;
};

class AnchorSystemPaths : public AnchorISystemPaths
{
 protected:
  /**
   * Constructor.
   * Protected default constructor to force use of static createSystem member.
   */
  AnchorSystemPaths()
  {}

  /**
   * Destructor.
   * Protected default constructor to force use of static dispose member.
   */
  virtual ~AnchorSystemPaths()
  {}

 public:
  /**
   * Determine the base dir in which shared resources are located. It will first try to use
   * "unpack and run" path, then look for properly installed path, including versioning.
   * \return Unsigned char string pointing to system dir (eg /usr/share/kraken/).
   */
  virtual const AnchorU8 *getSystemDir(int version, const char *versionstr) const = 0;

  /**
   * Determine the base dir in which user configuration is stored, including versioning.
   * If needed, it will create the base directory.
   * \return Unsigned char string pointing to user dir (eg ~/.kraken/).
   */
  virtual const AnchorU8 *getUserDir(int version, const char *versionstr) const = 0;

  /**
   * Determine the directory of the current binary
   * \return Unsigned char string pointing to the binary dir
   */
  virtual const AnchorU8 *getBinaryDir() const = 0;

  /**
   * Add the file to the operating system most recently used files
   */
  virtual void addToSystemRecentFiles(const char *filename) const = 0;
};

#if defined(__linux__)
class AnchorSystemPathsUnix : public AnchorSystemPaths
{
 public:
  /**
   * Constructor
   * this class should only be instantiated by AnchorISystem. */
  AnchorSystemPathsUnix();

  /**
   * Destructor.
   */
  ~AnchorSystemPathsUnix();

  /**
   * Determine the base dir in which shared resources are located. It will first try to use
   * "unpack and run" path, then look for properly installed path, including versioning.
   * \return Unsigned char string pointing to system dir (eg `/usr/share/kraken/`).
   */
  const AnchorU8 *getSystemDir(int version, const char *versionstr) const;

  /**
   * Determine the base dir in which user configuration is stored, including versioning.
   * If needed, it will create the base directory.
   * \return Unsigned char string pointing to user dir (eg `~/.config/.kraken/`).
   */
  const AnchorU8 *getUserDir(int version, const char *versionstr) const;

  /**
   * Determine a special ("well known") and easy to reach user directory.
   * \return Unsigned char string pointing to user dir (eg `~/Documents/`).
   */
  const AnchorU8 *getUserSpecialDir(eAnchorUserSpecialDirTypes type) const;

  /**
   * Determine the directory of the current binary
   * \return Unsigned char string pointing to the binary dir
   */
  const AnchorU8 *getBinaryDir() const;

  /**
   * Add the file to the operating system most recently used files
   */
  void addToSystemRecentFiles(const char *filename) const;
};
#elif defined(WIN32)
/**
 * WIN32 Implementation of AnchorSystemPaths class.
 * \see AnchorSystemPaths.
 */
class AnchorSystemPathsWin32 : public AnchorSystemPaths
{
 public:
  /**
   * Constructor.
   */
  AnchorSystemPathsWin32();

  /**
   * Destructor.
   */
  ~AnchorSystemPathsWin32();

  /**
   * Determine the base dir in which shared resources are located. It will first try to use
   * "unpack and run" path, then look for properly installed path, including versioning.
   * \return Unsigned char string pointing to system dir (eg /usr/share/).
   */
  const AnchorU8 *getSystemDir(int version, const char *versionstr) const;

  /**
   * Determine the base dir in which user configuration is stored, including versioning.
   * If needed, it will create the base directory.
   * \return Unsigned char string pointing to user dir (eg ~/).
   */
  const AnchorU8 *getUserDir(int version, const char *versionstr) const;

  /**
   * Determine a special ("well known") and easy to reach user directory.
   * \return Unsigned char string pointing to user dir (eg `~/Documents/`).
   */
  const AnchorU8 *getUserSpecialDir(eAnchorUserSpecialDirTypes type) const;

  /**
   * Determine the directory of the current binary
   * \return Unsigned char string pointing to the binary dir
   */
  const AnchorU8 *getBinaryDir() const;

  /**
   * Add the file to the operating system most recently used files
   */
  void addToSystemRecentFiles(const char *filename) const;
};
#elif defined(__APPLE__)
class AnchorSystemPathsCocoa : public AnchorSystemPaths
{
 public:
  /**
   * Constructor.
   */
  AnchorSystemPathsCocoa();

  /**
   * Destructor.
   */
  ~AnchorSystemPathsCocoa();

  /**
   * Determine the base dir in which shared resources are located. It will first try to use
   * "unpack and run" path, then look for properly installed path, including versioning.
   * \return Unsigned char string pointing to system dir (eg /usr/share/kraken/).
   */
  const AnchorU8 *getSystemDir(int version, const char *versionstr) const;

  /**
   * Determine the base dir in which user configuration is stored, including versioning.
   * If needed, it will create the base directory.
   * \return Unsigned char string pointing to user dir (eg ~/.kraken/).
   */
  const AnchorU8 *getUserDir(int version, const char *versionstr) const;

  /**
   * Determine a special ("well known") and easy to reach user directory.
   * \return Unsigned char string pointing to user dir (eg `~/Documents/`).
   */
  const AnchorU8 *getUserSpecialDir(eAnchorUserSpecialDirTypes type) const;

  /**
   * Determine the directory of the current binary
   * \return Unsigned char string pointing to the binary dir
   */
  const AnchorU8 *getBinaryDir() const;

  /**
   * Add the file to the operating system most recently used files
   */
  void addToSystemRecentFiles(const char *filename) const;
};
#endif

ANCHOR_DECLARE_HANDLE(AnchorSystemPathsHandle);

/**
 * Creates the one and only instance of the system path access.
 * @return An indication of success. */
eAnchorStatus ANCHOR_CreateSystemPaths(void);

/**
 * Disposes the one and only system.
 * @return An indication of success. */
eAnchorStatus ANCHOR_DisposeSystemPaths(void);

/**
 * Determine the base dir in which shared resources are located. It will first try to use
 * "unpack and run" path, then look for properly installed path, including versioning.
 * @return Unsigned char string pointing to system dir (eg /usr/share/kraken/). */
const AnchorU8 *ANCHOR_getSystemDir(int version, const char *versionstr);

/**
 * Determine the base dir in which user configuration is stored, including versioning.
 * @return Unsigned char string pointing to user dir (eg ~). */
const AnchorU8 *ANCHOR_getUserDir(int version, const char *versionstr);

/**
 * Determine a special ("well known") and easy to reach user directory.
 * @return Unsigned char string pointing to user dir (eg `~/Documents/`). */
const AnchorU8 *ANCHOR_getUserSpecialDir(eAnchorUserSpecialDirTypes type);

/**
 * Determine the dir in which the binary file is found.
 * @return Unsigned char string pointing to binary dir (eg ~/usr/local/bin/). */
const AnchorU8 *ANCHOR_getBinaryDir(void);

/**
 * Add the file to the operating system most recently used files */
void ANCHOR_addToSystemRecentFiles(const char *filename);
