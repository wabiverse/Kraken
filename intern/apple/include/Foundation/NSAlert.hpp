//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Foundation/NSView.hpp
//
// Copyright 2020-2022 Apple Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "NSDefines.hpp"
#include "NSObject.hpp"
#include "NSString.hpp"
#include "NSTypes.hpp"

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace NS
{
  _NS_ENUM(NS::Integer, AlertStyle){
    StyleWarning = 0,
    StyleInformational = 1,
    StyleCritical = 2,
  };

  _NS_ENUM(NS::Integer, ModalResponse){
    ResponseOkay = 0,
    ResponseCancel = 1,
    ResponseContinue = 2,
    ResponseStop = 3,
    ResponseAbort = 4,
  };

  class Alert : public Copying<Alert>
  {
   public:

    static Alert *alloc();
    Alert *init();

    void addButtonWithTitle(const NS::String *title);
    void setMessageText(const NS::String *text);
    void setInformativeText(const NS::String *text);
    void setAlertStyle(NS::AlertStyle alertStyle);
    NS::ModalResponse runModal();
  };
}  // namespace NS

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_NS_INLINE NS::Alert *NS::Alert::alloc()
{
  return Object::alloc<Alert>(_NS_PRIVATE_CLS(NSAlert));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_NS_INLINE NS::Alert *NS::Alert::init()
{
  return Object::init<Alert>();
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_NS_INLINE void NS::Alert::addButtonWithTitle(const NS::String *title)
{
  Object::sendMessage<void>(this, _NS_PRIVATE_SEL(addButtonWithTitle_), title);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_NS_INLINE void NS::Alert::setMessageText(const NS::String *text)
{
  Object::sendMessage<void>(this, _NS_PRIVATE_SEL(setMessageText_), text);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_NS_INLINE void NS::Alert::setInformativeText(const NS::String *text)
{
  Object::sendMessage<void>(this, _NS_PRIVATE_SEL(setInformativeText_), text);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_NS_INLINE void NS::Alert::setAlertStyle(NS::AlertStyle alertStyle)
{
  Object::sendMessage<void>(this, _NS_PRIVATE_SEL(setAlertStyle_), alertStyle);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

_NS_INLINE NS::ModalResponse NS::Alert::runModal()
{
  return Object::sendMessage<NS::ModalResponse>(this, _NS_PRIVATE_SEL(runModal));
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------