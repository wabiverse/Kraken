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

#include "wabi/base/work/dispatcher.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

WorkDispatcher::WorkDispatcher()
    : _context(tbb::task_group_context::isolated,
               tbb::task_group_context::concurrent_wait | tbb::task_group_context::default_traits)
{
  // The concurrent_wait flag used with the task_group_context ensures
  // the ref count will remain at 1 after all predecessor tasks are
  // completed, so we don't need to keep resetting it in Wait().
  _rootTask = new (tbb::task::allocate_root(_context)) tbb::empty_task;
  _rootTask->set_ref_count(1);
}

WorkDispatcher::~WorkDispatcher()
{
  Wait();
  tbb::task::destroy(*_rootTask);
}

void WorkDispatcher::Wait()
{
  _rootTask->wait_for_all();

  if (_context.is_group_execution_cancelled()) {
    _context.reset();
  }

  // Post all diagnostics to this thread's list.
  for (auto &et : _errors)
    et.Post();

  _errors.clear();
}

void WorkDispatcher::Cancel()
{
  _context.cancel_group_execution();
}

/* static */
void WorkDispatcher::_TransportErrors(const TfErrorMark &mark, _ErrorTransports *errors)
{
  TfErrorTransport transport = mark.Transport();
  errors->grow_by(1)->swap(transport);
}

WABI_NAMESPACE_END
