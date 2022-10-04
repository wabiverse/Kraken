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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault. */

#include "KLI_lazy_threading.hh"
#include "KLI_vector.hh"

namespace kraken::lazy_threading
{

  /**
   * This is a #RawVector so that it can be destructed after Blender checks for memory leaks.
   */
  thread_local RawVector<FunctionRef<void()>, 0> hint_receivers;

  void send_hint()
  {
    for (const FunctionRef<void()> &fn : hint_receivers) {
      fn();
    }
  }

  HintReceiver::HintReceiver(const FunctionRef<void()> fn)
  {
    hint_receivers.append(fn);
  }

  HintReceiver::~HintReceiver()
  {
    hint_receivers.pop_last();
  }

}  // namespace kraken::lazy_threading
