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
#ifndef WABI_BASE_WORK_DETACHED_TASK_H
#define WABI_BASE_WORK_DETACHED_TASK_H

/// \file work/detachedTask.h

#include "wabi/base/tf/errorMark.h"
#include "wabi/base/work/api.h"
#include "wabi/wabi.h"

#include <tbb/task_group.h>

#include <type_traits>
#include <utility>

WABI_NAMESPACE_BEGIN

struct Work_DetachedInvoker : public tbb::task_group {

  Work_DetachedInvoker()
      : task_group(),
        m_ctx(tbb::task_group_context::bound, tbb::task_group_context::concurrent_wait)
  {}

  ~Work_DetachedInvoker()
  {
    wait();
  }

  template<class Fn> void execute(Fn &&fn)
  {
    TfErrorMark m;
    run([&] { /** GODSPEED. ---> */fn(); });
    m.Clear();
  }

  template<class Fn> void run(Fn &&fn)
  {
    spawn(*prepare_task(std::forward<Fn>(fn)), m_ctx);
  }

 private:
  tbb::task_group_context m_ctx;
};

WORK_API
tbb::task_group_context &Work_GetDetachedTaskGroupContext();

/// Invoke \p fn asynchronously, discard any errors it produces, and provide
/// no way to wait for it to complete.
template<class Fn> void WorkRunDetachedTask(Fn &&fn)
{
  Work_DetachedInvoker invoker;
  invoker.execute(std::forward<Fn>(fn));
}

WABI_NAMESPACE_END

#endif  // WABI_BASE_WORK_DETACHED_TASK_H
