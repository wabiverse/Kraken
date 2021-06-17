/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */

/* clang-format off */

#include "wabi/usd/usdUI/tokens.h"
#include <boost/python/class.hpp>

WABI_NAMESPACE_USING

namespace {

/**
 * Helper to return a static token as a string. We wrap tokens
 * as Python strings and for some reason simply wrapping the
 * token using def_readonly bypasses  to-Python   conversion,
 * leading to the error that there's no Python type for the
 * C++ TfToken type.  So we wrap this functor instead. */
class _WrapStaticToken {
 public:
  _WrapStaticToken(const TfToken *token) : _token(token)
  {}

  std::string operator()() const
  {
    return _token->GetString();
  }

 private:
  const TfToken *_token;
};

template<typename T> void _AddToken(T &cls, const char *name, const TfToken &token)
{
  cls.add_static_property(name,
    boost::python::make_function(
      _WrapStaticToken(&token),
      boost::python::return_value_policy<boost::python::return_by_value>(),
      boost::mpl::vector1<std::string>()));
}

}  /* anonymous */

void wrapUsdUITokens()
{
  boost::python::class_ < UsdUITokensType, boost::noncopyable >
    cls("Tokens", boost::python::no_init);
      _AddToken(cls, "areas", UsdUITokens->areas);
      _AddToken(cls, "bottom", UsdUITokens->bottom);
      _AddToken(cls, "closed", UsdUITokens->closed);
      _AddToken(cls, "copy", UsdUITokens->copy);
      _AddToken(cls, "cross", UsdUITokens->cross);
      _AddToken(cls, "default_", UsdUITokens->default_);
      _AddToken(cls, "detached", UsdUITokens->detached);
      _AddToken(cls, "dialog", UsdUITokens->dialog);
      _AddToken(cls, "dot", UsdUITokens->dot);
      _AddToken(cls, "edit", UsdUITokens->edit);
      _AddToken(cls, "embedded", UsdUITokens->embedded);
      _AddToken(cls, "eraser", UsdUITokens->eraser);
      _AddToken(cls, "eyedropper", UsdUITokens->eyedropper);
      _AddToken(cls, "fullscreen", UsdUITokens->fullscreen);
      _AddToken(cls, "hand", UsdUITokens->hand);
      _AddToken(cls, "horizontalSplit", UsdUITokens->horizontalSplit);
      _AddToken(cls, "hSplit", UsdUITokens->hSplit);
      _AddToken(cls, "knife", UsdUITokens->knife);
      _AddToken(cls, "left", UsdUITokens->left);
      _AddToken(cls, "maximized", UsdUITokens->maximized);
      _AddToken(cls, "minimized", UsdUITokens->minimized);
      _AddToken(cls, "none", UsdUITokens->none);
      _AddToken(cls, "normal", UsdUITokens->normal);
      _AddToken(cls, "open", UsdUITokens->open);
      _AddToken(cls, "paint", UsdUITokens->paint);
      _AddToken(cls, "paintBrush", UsdUITokens->paintBrush);
      _AddToken(cls, "right", UsdUITokens->right);
      _AddToken(cls, "stop", UsdUITokens->stop);
      _AddToken(cls, "swapArea", UsdUITokens->swapArea);
      _AddToken(cls, "textEdit", UsdUITokens->textEdit);
      _AddToken(cls, "top", UsdUITokens->top);
      _AddToken(cls, "uiAreaIcon", UsdUITokens->uiAreaIcon);
      _AddToken(cls, "uiAreaName", UsdUITokens->uiAreaName);
      _AddToken(cls, "uiAreaPos", UsdUITokens->uiAreaPos);
      _AddToken(cls, "uiAreaSize", UsdUITokens->uiAreaSize);
      _AddToken(cls, "uiDescription", UsdUITokens->uiDescription);
      _AddToken(cls, "uiDisplayGroup", UsdUITokens->uiDisplayGroup);
      _AddToken(cls, "uiDisplayName", UsdUITokens->uiDisplayName);
      _AddToken(cls, "uiNodegraphNodeDisplayColor", UsdUITokens->uiNodegraphNodeDisplayColor);
      _AddToken(cls, "uiNodegraphNodeExpansionState", UsdUITokens->uiNodegraphNodeExpansionState);
      _AddToken(cls, "uiNodegraphNodeIcon", UsdUITokens->uiNodegraphNodeIcon);
      _AddToken(cls, "uiNodegraphNodePos", UsdUITokens->uiNodegraphNodePos);
      _AddToken(cls, "uiNodegraphNodeSize", UsdUITokens->uiNodegraphNodeSize);
      _AddToken(cls, "uiNodegraphNodeStackingOrder", UsdUITokens->uiNodegraphNodeStackingOrder);
      _AddToken(cls, "uiScreenAlignment", UsdUITokens->uiScreenAlignment);
      _AddToken(cls, "uiScreenAreas", UsdUITokens->uiScreenAreas);
      _AddToken(cls, "uiScreenCollectionAreasIncludeRoot", UsdUITokens->uiScreenCollectionAreasIncludeRoot);
      _AddToken(cls, "uiWindowCursor", UsdUITokens->uiWindowCursor);
      _AddToken(cls, "uiWindowDpi", UsdUITokens->uiWindowDpi);
      _AddToken(cls, "uiWindowDpifac", UsdUITokens->uiWindowDpifac);
      _AddToken(cls, "uiWindowIcon", UsdUITokens->uiWindowIcon);
      _AddToken(cls, "uiWindowLinewidth", UsdUITokens->uiWindowLinewidth);
      _AddToken(cls, "uiWindowPixelsz", UsdUITokens->uiWindowPixelsz);
      _AddToken(cls, "uiWindowPos", UsdUITokens->uiWindowPos);
      _AddToken(cls, "uiWindowScale", UsdUITokens->uiWindowScale);
      _AddToken(cls, "uiWindowSize", UsdUITokens->uiWindowSize);
      _AddToken(cls, "uiWindowState", UsdUITokens->uiWindowState);
      _AddToken(cls, "uiWindowTitle", UsdUITokens->uiWindowTitle);
      _AddToken(cls, "uiWindowType", UsdUITokens->uiWindowType);
      _AddToken(cls, "uiWindowWidgetunit", UsdUITokens->uiWindowWidgetunit);
      _AddToken(cls, "uiWindowWorkspace", UsdUITokens->uiWindowWorkspace);
      _AddToken(cls, "uiWorkspaceName", UsdUITokens->uiWorkspaceName);
      _AddToken(cls, "uiWorkspaceScreen", UsdUITokens->uiWorkspaceScreen);
      _AddToken(cls, "verticalSplit", UsdUITokens->verticalSplit);
      _AddToken(cls, "vloop", UsdUITokens->vloop);
      _AddToken(cls, "vSplit", UsdUITokens->vSplit);
      _AddToken(cls, "wait", UsdUITokens->wait);
      _AddToken(cls, "xMove", UsdUITokens->xMove);
      _AddToken(cls, "yMove", UsdUITokens->yMove);
      _AddToken(cls, "zoomIn", UsdUITokens->zoomIn);
      _AddToken(cls, "zoomOut", UsdUITokens->zoomOut);
}
