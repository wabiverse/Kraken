/* SPDX-License-Identifier: GPL-2.0-or-later */

/** 
 * @file
 * @ingroup edinterface
 *
 * Code to manage views as part of the regular screen hierarchy. E.g. managing ownership of views
 * inside blocks (#uiBlock.views), looking up items in the region, passing WM notifiers to views,
 * etc.
 *
 * Blocks and their contained views are reconstructed on every redraw. This file also contains
 * functions related to this recreation of views inside blocks. For example to query state
 * information before the view is done reconstructing (#AbstractView.is_reconstructed() returns
 * false), it may be enough to query the previous version of the block/view/view-item. Since such
 * queries rely on the details of the UI reconstruction process, they should remain internal to
 * `interface/` code.
 */

#include <memory>
#include <type_traits>
#include <variant>

#include "USD_space_types.h"

#include "KKE_screen.h"

#include "KLI_listbase.h"
#include "KLI_map.hh"
#include "KLI_string_ref.hh"

#include "ED_screen.h"

#include "interface_intern.h"

#include "UI_interface.hh"

#include "UI_abstract_view.hh"
// #include "UI_grid_view.hh"
// #include "UI_tree_view.hh"

#include "interface_intern.h"

using namespace kraken;
using namespace kraken::ui;

/**
 * Wrapper to store views in a #ListBase, addressable via an identifier.
 */
struct ViewLink : public Link 
{
  std::string idname;
  std::unique_ptr<AbstractView> view;

  static void views_bounds_calc(const uiBlock &block);
};


void ViewLink::views_bounds_calc(const uiBlock &block)
{
  Map<AbstractView *, rcti> views_bounds;

  rcti minmax;
  KLI_rcti_init_minmax(&minmax);
  LISTBASE_FOREACH (ViewLink *, link, &block.views) {
    views_bounds.add(link->view.get(), minmax);
  }

  LISTBASE_FOREACH (uiBut *, but, &block.buttons) {
    if (but->type != UI_BTYPE_VIEW_ITEM) {
      continue;
    }
    uiButViewItem *view_item_but = static_cast<uiButViewItem *>(but);
    if (!view_item_but->view_item) {
      continue;
    }

    /* Get the view from the button. */
    AbstractViewItem &view_item = reinterpret_cast<AbstractViewItem &>(*view_item_but->view_item);
    AbstractView &view = view_item.get_view();

    rcti &bounds = views_bounds.lookup(&view);
    rcti but_rcti{};
    KLI_rcti_rctf_copy_round(&but_rcti, &view_item_but->rect);
    KLI_rcti_do_minmax_rcti(&bounds, &but_rcti);
  }

  for (const auto item : views_bounds.items()) {
    const rcti &bounds = item.value;
    if (KLI_rcti_is_empty(&bounds)) {
      continue;
    }

    AbstractView &view = *item.key;
    view.bounds_ = bounds;
  }
}


static StringRef ui_block_view_find_idname(const uiBlock &block, const AbstractView &view)
{
  /* First get the idname the of the view we're looking for. */
  LISTBASE_FOREACH (ViewLink *, view_link, &block.views) {
    if (view_link->view.get() == &view) {
      return view_link->idname;
    }
  }

  return {};
}


template<class T>
static T *ui_block_view_find_matching_in_old_block_impl(const uiBlock &new_block,
                                                        const T &new_view)
{
  uiBlock *old_block = new_block.oldblock;
  if (!old_block) {
    return nullptr;
  }

  StringRef idname = ui_block_view_find_idname(new_block, new_view);
  if (idname.is_empty()) {
    return nullptr;
  }

  LISTBASE_FOREACH (ViewLink *, old_view_link, &old_block->views) {
    if (old_view_link->idname == idname) {
      return dynamic_cast<T *>(old_view_link->view.get());
    }
  }

  return nullptr;
}

uiViewHandle *ui_block_view_find_matching_in_old_block(const uiBlock *new_block,
                                                       const uiViewHandle *new_view_handle)
{
  KLI_assert(new_block && new_view_handle);
  const AbstractView &new_view = reinterpret_cast<const AbstractView &>(*new_view_handle);

  AbstractView *old_view = ui_block_view_find_matching_in_old_block_impl(*new_block, new_view);
  return reinterpret_cast<uiViewHandle *>(old_view);
}
