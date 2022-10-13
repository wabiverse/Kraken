/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2010 Blender Foundation. All rights reserved. */

#import <Foundation/Foundation.h>

#include "ANCHOR_system_paths.h"

#pragma mark initialization/finalization

AnchorSystemPathsCocoa::AnchorSystemPathsCocoa() {}

AnchorSystemPathsCocoa::~AnchorSystemPathsCocoa() {}

#pragma mark Base directories retrieval

const char *AnchorSystemPathsCocoa::GetApplicationSupportDir(const char *versionstr,
                                                             const int shim_mask,
                                                             char *tempPath,
                                                             const std::size_t len_tempPath)
{
  NSSearchPathDomainMask mask;
  switch(shim_mask)
  {
    case 1:
      mask = NSUserDomainMask;
      break;
    default:
      mask = NSLocalDomainMask;
      break;
  }

  @autoreleasepool {
    const NSArray *const paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory,
                                                                     mask,
                                                                     YES);

    if ([paths count] == 0) {
      return NULL;
    }
    const NSString *const basePath = [paths objectAtIndex:0];

    snprintf(tempPath,
             len_tempPath,
             "%s/Kraken/%s",
             [basePath cStringUsingEncoding:NSASCIIStringEncoding],
             versionstr);
  }
  return tempPath;
}

const char *AnchorSystemPathsCocoa::getSystemDir(int, const char *versionstr) const
{
  static char tempPath[512] = "";
  return GetApplicationSupportDir(versionstr, 0/*NSLocalDomainMask*/, tempPath, sizeof(tempPath));
}

const char *AnchorSystemPathsCocoa::getUserDir(int, const char *versionstr) const
{
  static char tempPath[512] = "";
  return GetApplicationSupportDir(versionstr, 1/*NSUserDomainMask*/, tempPath, sizeof(tempPath));
}

const char *AnchorSystemPathsCocoa::getUserSpecialDir(eAnchorUserSpecialDirTypes type) const
{
  static char tempPath[512] = "";
  @autoreleasepool {
    NSSearchPathDirectory ns_directory;

    switch (type) {
      case ANCHOR_UserSpecialDirDesktop:
        ns_directory = NSDesktopDirectory;
        break;
      case ANCHOR_UserSpecialDirDocuments:
        ns_directory = NSDocumentDirectory;
        break;
      case ANCHOR_UserSpecialDirDownloads:
        ns_directory = NSDownloadsDirectory;
        break;
      case ANCHOR_UserSpecialDirMusic:
        ns_directory = NSMusicDirectory;
        break;
      case ANCHOR_UserSpecialDirPictures:
        ns_directory = NSPicturesDirectory;
        break;
      case ANCHOR_UserSpecialDirVideos:
        ns_directory = NSMoviesDirectory;
        break;
      case ANCHOR_UserSpecialDirCaches:
        ns_directory = NSCachesDirectory;
        break;
      default:
        ANCHOR_ASSERT("Anchor -- Invalid enum value for type parameter");
        return NULL;
    }

    const NSArray *const paths = NSSearchPathForDirectoriesInDomains(ns_directory,
                                                                     NSUserDomainMask,
                                                                     YES);
    if ([paths count] == 0) {
      return NULL;
    }
    const NSString *const basePath = [paths objectAtIndex:0];

    strncpy(tempPath, [basePath cStringUsingEncoding:NSASCIIStringEncoding], sizeof(tempPath));
  }
  return tempPath;
}

const char *AnchorSystemPathsCocoa::getBinaryDir() const
{
  static char tempPath[512] = "";

  @autoreleasepool {
    const NSString *const basePath = [[NSBundle mainBundle] bundlePath];

    if (basePath == nil) {
      return NULL;
    }

    strcpy(tempPath, [basePath cStringUsingEncoding:NSASCIIStringEncoding]);
  }
  return tempPath;
}

void AnchorSystemPathsCocoa::addToSystemRecentFiles(const char *filename) const
{
  /* TODO: implement for macOS */
}
