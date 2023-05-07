/* SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup imbuf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "KLI_linklist.h"
#include "KLI_listbase.h" /* Needed due to import of KLO_readfile.h */
#include "KLI_utildefines.h"

#include "KKE_icons.h"
#include "KKE_idtype.h"
#include "KKE_main.h"

#include "USD_ID.h"
#include "USD_color_types.h"

#include "KLO_readfile.h"

#include "WM_reports.h"

#include "IMB_imbuf.h"
// #include "IMB_imbuf_types.h"
#include "IMB_thumbs.h"

#include "MEM_guardedalloc.h"

static ImBuf *imb_thumb_load_from_usd_id(const char *usd_path,
                                         const char *usd_group,
                                         const char *usd_id)
{
  ImBuf *ima = NULL;
  USDFileReadReport kf_reports = {.reports = NULL};


  return ima;
}

static ImBuf *imb_thumb_load_from_usdfile(const char *usd_path)
{
  KrakenThumbnail *data = KLO_thumbnail_from_file(usd_path);
  ImBuf *ima = /*KKE_main_thumbnail_to_imbuf(NULL, data)*/NULL;

  if (data) {
    MEM_freeN(data);
  }
  return ima;
}

ImBuf *IMB_thumb_load_usd(const char *usd_path, const char *usd_group, const char *usd_id)
{
  if (usd_group && usd_id) {
    return imb_thumb_load_from_usd_id(usd_path, usd_group, usd_id);
  }
  return imb_thumb_load_from_usdfile(usd_path);
}
