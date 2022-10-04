#pragma once

/** 
 * @file
 * @ingroup KKE
 *
 * Resizable Icons for Kraken.
 *
 * There is some thread safety for this API but it is rather weak. Registering or unregistering
 * icons is thread safe, changing data of icons from multiple threads is not. Practically this
 * should be fine since only the main thread modifies icons. Should that change, more locks or a
 * different design need to be introduced.
 */

#include "KLI_compiler_attrs.h"

#include "USD_ID_enums.h"

typedef void (*DrawInfoFreeFP)(void *drawinfo);

enum {
  /** ID preview: obj is #ID. */
  ICON_DATA_ID = 0,
  /** Arbitrary Image buffer: obj is #ImBuf */
  ICON_DATA_IMBUF,
  /** Preview: obj is #PreviewImage */
  ICON_DATA_PREVIEW,
  /** 2D triangles: obj is #Icon_Geom */
  ICON_DATA_GEOM,
  /** Studiolight */
  ICON_DATA_STUDIOLIGHT,
  /** GPencil Layer color preview (annotations): obj is #bGPDlayer */
  ICON_DATA_GPLAYER,
};

/**
 * @note See comment at the top regarding thread safety.
 */
struct Icon {
  void *drawinfo;
  /**
   * Data defined by #obj_type
   * @note for #ICON_DATA_GEOM the memory is owned by the icon,
   * could be made into a flag if we want that to be optional.
   */
  void *obj;
  char obj_type;
  /** Internal use only. */
  char flag;
  /** #ID_Type or 0 when not used for ID preview. */
  short id_type;
  DrawInfoFreeFP drawinfo_free;
};

/** Used for #ICON_DATA_GEOM, assigned to #Icon.obj. */
struct Icon_Geom {
  int icon_id;
  int coords_len;
  int coords_range[2];
  unsigned char (*coords)[2];
  unsigned char (*colors)[4];
  /* when not NULL, the memory of coords and colors is a sub-region of this pointer. */
  const void *mem;
};

typedef struct LockfreeLinkNode {
  struct LockfreeLinkNode *next;
  /* NOTE: "Subclass" this structure to add custom-defined data. */
} LockfreeLinkNode;

typedef struct LockfreeLinkList {
  /* We keep a dummy node at the beginning of the list all the time.
   * This allows us to make sure head and tail pointers are always
   * valid, and saves from annoying exception cases in insert().
   */
  LockfreeLinkNode dummy_node;
  /* NOTE: This fields might point to a dummy node. */
  LockfreeLinkNode *head, *tail;
} LockfreeLinkList;

typedef struct Icon Icon;

struct KrakenDataReader;
struct USDWriter;
struct ID;
struct ImBuf;
struct PreviewImage;
struct StudioLight;
struct kGPDlayer;

void KKE_icons_init(int first_dyn_id);

/**
 * Retrieve icon for id.
 */
struct Icon *KKE_icon_get(int icon_id);

#define ICON_RENDER_DEFAULT_HEIGHT 32

