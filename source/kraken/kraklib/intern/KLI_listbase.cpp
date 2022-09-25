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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * @ingroup KRAKEN Library.
 * Gadget Vault.
 */

#include "MEM_guardedalloc.h"

#include "KLI_listbase.h"

#include "KKE_context.h"

#include <wabi/base/tf/token.h>

KRAKEN_NAMESPACE_BEGIN

kContextStoreEntry *KLI_rfindtoken(const kContextStore *store, const wabi::TfToken &id)
{
  auto it = store->entries.rbegin();
  while (it != store->entries.rend()) {
    if (id == (*it)->name) {
      return *it;
    }
    it++;
  }

  return nullptr;
}

void KLI_freelistN(kContextStore *store)
{
  for (size_t i = 0; i < store->entries.size(); i++) 
  {       
    delete store->entries[i];    
  }    
  store->entries.clear();
}

kContextStore *KLI_pophead(std::vector<struct kContextStore*> contexts)
{
  kContextStore *store;
  if ((store = contexts.front())) {
    KLI_remlink(contexts, store);
  }
  return store;
}

void KLI_remlink(std::vector<struct kContextStore*> contexts, kContextStore *store)
{
  kContextStore *link = store;

  if (link == nullptr) {
    return;
  }

  contexts.erase(std::remove(contexts.begin(), contexts.end(), link), contexts.end());
}

KRAKEN_NAMESPACE_END