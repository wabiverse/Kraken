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
 * COVAH Kernel.
 * Purple Underground.
 */

#pragma once

/** \file
 * \ingroup wke
 */

/**
 * The lines below use regex from scripts to extract their values,
 * Keep this in mind when modifying this file and keep this comment above the defines.
 *
 * \note Use #STRINGIFY() rather than defining with quotes.
 */

/* COVAH major and minor version. */
#define COVAH_VERSION_MAJOR 1
#define COVAH_VERSION_MINOR 50

/* COVAH major and minor version. */
#define COVAH_VERSION 150
/* COVAH patch version for bugfix releases. */
#define COVAH_VERSION_PATCH 0
/** COVAH release cycle stage: alpha/beta/rc/release. */
#define COVAH_VERSION_CYCLE alpha

/* COVAH file format version. */
#define COVAH_FILE_VERSION COVAH_VERSION
#define COVAH_FILE_SUBVERSION 1

#define COVAH_FILE_MIN_VERSION 150
#define COVAH_FILE_MIN_SUBVERSION 0
