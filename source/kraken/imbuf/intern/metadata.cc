/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2005 Blender Foundation */

/** 
 * @file
 * @ingroup imbuf
 */

#include <stdlib.h>
#include <string.h>

#include "KLI_listbase.h"
#include "KLI_string.h"
#include "KLI_utildefines.h"

#include "KKE_idprop.h"

#include "USD_ID.h" /* ID property definitions. */

#include "MEM_guardedalloc.h"

#include "IMB_imbuf.h"
// #include "IMB_imbuf_types.h"

#include "IMB_metadata.h"

void IMB_metadata_ensure(struct IDProperty **metadata)
{
  if (*metadata != nullptr) {
    return;
  }

  IDPropertyTemplate val = {0};
  *metadata = IDP_New(IDP_GROUP, &val, wabi::TfToken("metadata"));
}

void IMB_metadata_free(struct IDProperty *metadata)
{
  if (metadata == nullptr) {
    return;
  }

  IDP_FreeProperty(metadata);
}

bool IMB_metadata_get_field(struct IDProperty *metadata,
                            const char *key,
                            char *field,
                            const size_t len)
{
  IDProperty *prop;

  if (metadata == nullptr) {
    return false;
  }

  prop = IDP_GetPropertyFromGroup(metadata, wabi::TfToken(key));

  if (prop && prop->type == IDP_STRING) {
    KLI_strncpy(field, IDP_String(prop), len);
    return true;
  }
  return false;
}

void IMB_metadata_copy(struct ImBuf *dimb, struct ImBuf *simb)
{
  KLI_assert(dimb != simb);
  if (simb->metadata) {
    IMB_metadata_free(dimb->metadata);
    dimb->metadata = IDP_CopyProperty(simb->metadata);
  }
}

void IMB_metadata_set_field(struct IDProperty *metadata, const char *key, const char *value)
{
  KLI_assert(metadata);
  IDProperty *prop = IDP_GetPropertyFromGroup(metadata, wabi::TfToken(key));

  if (prop != nullptr && prop->type != IDP_STRING) {
    IDP_FreeFromGroup(metadata, prop);
    prop = nullptr;
  }

  if (prop) {
    IDP_AssignString(prop, value);
  }
  else if (prop == nullptr) {
    prop = IDP_NewString(wabi::TfToken(value), wabi::TfToken(key), sizeof(value));
    IDP_AddToGroup(metadata, prop);
  }
}

void IMB_metadata_foreach(struct ImBuf *ibuf, IMBMetadataForeachCb callback, void *userdata)
{
  if (ibuf->metadata == nullptr) {
    return;
  }
  LISTBASE_FOREACH (IDProperty *, prop, &ibuf->metadata->data.group) {
    callback(prop->name, IDP_String(prop), userdata);
  }
}