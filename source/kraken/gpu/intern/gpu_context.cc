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
 * From the Blender GPU library. (source/blender/gpu).
 * 
 * With any additions or modifications specific to Kraken.
 * 
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * @ingroup GPU.
 * Pixel Magic.
 *
 * Manage GL vertex array IDs in a thread-safe way
 * Use these instead of glGenBuffers & its friends
 * - alloc must be called from a thread that is bound
 *   to the context that will be used for drawing with
 *   this VAO.
 * - free can be called from any thread
 */

#include "KLI_assert.h"
#include "KLI_utildefines.h"

#include "GPU_context.h"
#include "GPU_framebuffer.h"

#include "gpu_backend.hh"
#include "gpu_batch_private.hh"
#include "gpu_context_private.hh"
#include "gpu_matrix_private.h"
#include "gpu_private.h"

#ifdef WITH_METAL_BACKEND
#  include "mtl_backend.hh"
#endif

#include <mutex>
#include <vector>

using namespace kraken::gpu;

static thread_local Context *active_ctx = nullptr;

static std::mutex backend_users_mutex;
static int num_backend_users = 0;

static void gpu_backend_create();
static void gpu_backend_discard();

/* -------------------------------------------------------------------- */
/** \name gpu::Context methods
 * \{ */

namespace kraken::gpu
{

  int Context::context_counter = 0;
  Context::Context()
  {
    thread_ = pthread_self();
    is_active_ = false;
    matrix_state = GPU_matrix_state_create();

    context_id = Context::context_counter;
    Context::context_counter++;
  }

  Context::~Context()
  {
    GPU_matrix_state_discard(matrix_state);
    delete state_manager;
    delete front_left;
    delete back_left;
    delete front_right;
    delete back_right;
    delete imm;
  }

  bool Context::is_active_on_thread()
  {
    return (this == active_ctx) && pthread_equal(pthread_self(), thread_);
  }

  Context *Context::get()
  {
    return active_ctx;
  }

}  // namespace kraken::gpu

/** \} */

/* -------------------------------------------------------------------- */

GPUContext *GPU_context_create(void *anchor_window, void *anchor_context)
{
  {
    std::scoped_lock lock(backend_users_mutex);
    if (num_backend_users == 0) {
      /* Automatically create backend when first context is created. */
      gpu_backend_create();
    }
    num_backend_users++;
  }

  Context *ctx = GPUBackend::get()->context_alloc(anchor_window, anchor_context);

  GPU_context_active_set(wrap(ctx));
  return wrap(ctx);
}

void GPU_context_discard(GPUContext *ctx_)
{
  Context *ctx = unwrap(ctx_);
  delete ctx;
  active_ctx = nullptr;

  {
    std::scoped_lock lock(backend_users_mutex);
    num_backend_users--;
    KLI_assert(num_backend_users >= 0);
    if (num_backend_users == 0) {
      /* Discard backend when last context is discarded. */
      gpu_backend_discard();
    }
  }
}

void GPU_context_active_set(GPUContext *ctx_)
{
  Context *ctx = unwrap(ctx_);

  if (active_ctx) {
    active_ctx->deactivate();
  }

  active_ctx = ctx;

  if (ctx) {
    ctx->activate();
  }
}

GPUContext *GPU_context_active_get()
{
  return wrap(Context::get());
}

void GPU_context_begin_frame(GPUContext *ctx)
{
  kraken::gpu::Context *_ctx = unwrap(ctx);
  if (_ctx) {
    _ctx->begin_frame();
  }
}

void GPU_context_end_frame(GPUContext *ctx)
{
  kraken::gpu::Context *_ctx = unwrap(ctx);
  if (_ctx) {
    _ctx->end_frame();
  }
}

/* -------------------------------------------------------------------- */
/** \name Main context global mutex
 *
 * Used to avoid crash on some old drivers.
 * \{ */

static std::mutex main_context_mutex;

void GPU_context_main_lock()
{
  main_context_mutex.lock();
}

void GPU_context_main_unlock()
{
  main_context_mutex.unlock();
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name  GPU Begin/end work blocks
 *
 * Used to explicitly define a per-frame block within which GPU work will happen.
 * Used for global autoreleasepool flushing in Metal
 * \{ */

void GPU_render_begin()
{
  GPUBackend *backend = GPUBackend::get();
  KLI_assert(backend);
  /* WORKAROUND: Currently a band-aid for the heist production. Has no side effect for GL backend
   * but should be fixed for Metal. */
  if (backend) {
    backend->render_begin();
  }
}
void GPU_render_end()
{
  GPUBackend *backend = GPUBackend::get();
  KLI_assert(backend);
  backend->render_end();
}
void GPU_render_step()
{
  GPUBackend *backend = GPUBackend::get();
  KLI_assert(backend);
  backend->render_step();
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Backend selection
 * \{ */

/* NOTE: To enable Metal API, we need to temporarily change this to `GPU_BACKEND_METAL`.
 * Until a global switch is added, Metal also needs to be enabled in ANCHOR::Context:
 * `m_useMetalForRendering = true`. */
static const eGPUBackendType g_backend_type = GPU_BACKEND_METAL;
static GPUBackend *g_backend = nullptr;

bool GPU_backend_supported(void)
{
  switch (g_backend_type) {
    case GPU_BACKEND_METAL:
#ifdef WITH_METAL_BACKEND
      return MTLBackend::metal_is_supported();
#else
      return false;
#endif
    default:
      KLI_assert(false && "No backend specified");
      return false;
  }
}

static void gpu_backend_create()
{
  KLI_assert(g_backend == nullptr);
  KLI_assert(GPU_backend_supported());

  switch (g_backend_type) {
#ifdef WITH_METAL_BACKEND
    case GPU_BACKEND_METAL:
      g_backend = new MTLBackend;
      break;
#endif
    default:
      KLI_assert(0);
      break;
  }
}

void gpu_backend_delete_resources()
{
  KLI_assert(g_backend);
  g_backend->delete_resources();
}

void gpu_backend_discard()
{
  /* TODO: assert no resource left. */
  delete g_backend;
  g_backend = nullptr;
}

eGPUBackendType GPU_backend_get_type()
{

#ifdef WITH_METAL_BACKEND
  if (g_backend && dynamic_cast<MTLBackend *>(g_backend) != nullptr) {
    return GPU_BACKEND_METAL;
  }
#endif

  return GPU_BACKEND_NONE;
}

GPUBackend *GPUBackend::get()
{
  return g_backend;
}

/** \} */
