// Copyright (c) 2016, libnumaapi authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// Author: Sergey Sharybin <sergey.vfx@gmail.com>

#include "KLI_numaapi.h"

#if OS_WIN

#  ifndef NOGDI
#    define NOGDI
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOCOMM
#    define NOCOMM
#  endif

#  include <stdlib.h>
#  include <stdint.h>
#  include <windows.h>

#  if ARCH_CPU_64_BITS
#    include <VersionHelpers.h>
#  endif

////////////////////////////////////////////////////////////////////////////////
// Initialization.

// Kernel library, from where the symbols come.
static HMODULE kernel_lib;

// Types of all symbols which are read from the library.

// NUMA function types.
typedef BOOL t_GetNumaHighestNodeNumber(PULONG highest_node_number);
typedef BOOL t_GetNumaNodeProcessorMask(UCHAR node, ULONGLONG *processor_mask);
typedef BOOL t_GetNumaNodeProcessorMaskEx(USHORT node, GROUP_AFFINITY *processor_mask);
typedef BOOL t_GetNumaProcessorNode(UCHAR processor, UCHAR *node_number);
typedef void *t_VirtualAllocExNuma(HANDLE process_handle,
                                   LPVOID address,
                                   SIZE_T size,
                                   DWORD allocation_type,
                                   DWORD protect,
                                   DWORD preferred);
typedef BOOL t_VirtualFree(void *address, SIZE_T size, DWORD free_type);
// Threading function types.
typedef BOOL t_SetProcessAffinityMask(HANDLE process_handle, DWORD_PTR process_affinity_mask);
typedef BOOL t_SetThreadGroupAffinity(HANDLE thread_handle,
                                      const GROUP_AFFINITY *group_affinity,
                                      GROUP_AFFINITY *PreviousGroupAffinity);
typedef BOOL t_GetThreadGroupAffinity(HANDLE thread_handle, GROUP_AFFINITY *group_affinity);
typedef DWORD t_GetCurrentProcessorNumber(void);
typedef void t_GetCurrentProcessorNumberEx(PROCESSOR_NUMBER *proc_number);
typedef DWORD t_GetActiveProcessorCount(WORD group_number);


// NUMA symbols.
static t_GetNumaHighestNodeNumber *_GetNumaHighestNodeNumber;
static t_GetNumaNodeProcessorMask *_GetNumaNodeProcessorMask;
static t_GetNumaNodeProcessorMaskEx *_GetNumaNodeProcessorMaskEx;
static t_GetNumaProcessorNode *_GetNumaProcessorNode;
static t_VirtualAllocExNuma *_VirtualAllocExNuma;
static t_VirtualFree *_VirtualFree;
// Threading symbols.
static t_SetProcessAffinityMask *_SetProcessAffinityMask;
static t_SetThreadGroupAffinity *_SetThreadGroupAffinity;
static t_GetThreadGroupAffinity *_GetThreadGroupAffinity;
static t_GetCurrentProcessorNumber *_GetCurrentProcessorNumber;
static t_GetCurrentProcessorNumberEx *_GetCurrentProcessorNumberEx;
static t_GetActiveProcessorCount *_GetActiveProcessorCount;

static void numaExit(void)
{
  // TODO(sergey): Consider closing library here.
}

static NUMAAPI_Result loadNumaSymbols(void)
{
  // Prevent multiple initializations.
  static bool initialized = false;
  static NUMAAPI_Result result = NUMAAPI_NOT_AVAILABLE;
  if (initialized)
  {
    return result;
  }
  initialized = true;
  // Register de-initialization.
  const int error = atexit(numaExit);
  if (error)
  {
    result = NUMAAPI_ERROR_ATEXIT;
    return result;
  }
  // Load library.
  kernel_lib = LoadLibraryA("Kernel32.dll");
  // Load symbols.

#  define _LIBRARY_FIND(lib, name)                      \
    do                                                  \
    {                                                   \
      _##name = (t_##name *)GetProcAddress(lib, #name); \
    } while (0)
#  define KERNEL_LIBRARY_FIND(name) _LIBRARY_FIND(kernel_lib, name)

  // NUMA.
  KERNEL_LIBRARY_FIND(GetNumaHighestNodeNumber);
  KERNEL_LIBRARY_FIND(GetNumaNodeProcessorMask);
  KERNEL_LIBRARY_FIND(GetNumaNodeProcessorMaskEx);
  KERNEL_LIBRARY_FIND(GetNumaProcessorNode);
  KERNEL_LIBRARY_FIND(VirtualAllocExNuma);
  KERNEL_LIBRARY_FIND(VirtualFree);
  // Threading.
  KERNEL_LIBRARY_FIND(SetProcessAffinityMask);
  KERNEL_LIBRARY_FIND(SetThreadGroupAffinity);
  KERNEL_LIBRARY_FIND(GetThreadGroupAffinity);
  KERNEL_LIBRARY_FIND(GetCurrentProcessorNumber);
  KERNEL_LIBRARY_FIND(GetCurrentProcessorNumberEx);
  KERNEL_LIBRARY_FIND(GetActiveProcessorCount);

#  undef KERNEL_LIBRARY_FIND
#  undef _LIBRARY_FIND

  result = NUMAAPI_SUCCESS;
  return result;
}

NUMAAPI_Result numaAPI_Initialize(void)
{
#  if !ARCH_CPU_64_BITS
  // No NUMA on 32 bit platforms.
  return NUMAAPI_NOT_AVAILABLE;
#  else
  loadNumaSymbols();
  return NUMAAPI_SUCCESS;
#  endif
}

////////////////////////////////////////////////////////////////////////////////
// Internal helpers.

static int countNumSetBits(ULONGLONG mask)
{
  // TODO(sergey): There might be faster way calculating number of set bits.
  // NOTE: mask must be unsigned, there is undefined behavior for signed ints.
  int num_bits = 0;
  while (mask != 0)
  {
    num_bits += (mask & 1);
    mask = (mask >> 1);
  }
  return num_bits;
}

////////////////////////////////////////////////////////////////////////////////
// Topology query.

int numaAPI_GetNumNodes(void)
{
  ULONG highest_node_number;
  if (!_GetNumaHighestNodeNumber(&highest_node_number))
  {
    return 0;
  }
  // TODO(sergey): Resolve the type narrowing.
  // NOTE: This is not necessarily a total amount of nodes in the system.
  return (int)highest_node_number + 1;
}

bool numaAPI_IsNodeAvailable(int node)
{
  // Trick to detect whether the node is usable or not: check whether
  // there are any processors associated with it.
  //
  // This is needed because numaApiGetNumNodes() is not guaranteed to
  // give total amount of nodes and some nodes might be unavailable.
  GROUP_AFFINITY processor_mask = {0};
  if (!_GetNumaNodeProcessorMaskEx(node, &processor_mask))
  {
    return false;
  }
  if (processor_mask.Mask == 0)
  {
    return false;
  }
  return true;
}

int numaAPI_GetNumNodeProcessors(int node)
{
  GROUP_AFFINITY processor_mask = {0};
  if (!_GetNumaNodeProcessorMaskEx(node, &processor_mask))
  {
    return 0;
  }
  return countNumSetBits(processor_mask.Mask);
}

////////////////////////////////////////////////////////////////////////////////
// Topology helpers.

int numaAPI_GetNumCurrentNodesProcessors(void)
{
  HANDLE thread_handle = GetCurrentThread();
  GROUP_AFFINITY group_affinity;
  // TODO(sergey): Needs implementation.
  if (!_GetThreadGroupAffinity(thread_handle, &group_affinity))
  {
    return 0;
  }
  // First, count number of possible bits in the affinity mask.
  const int num_processors = countNumSetBits(group_affinity.Mask);
  // Then check that it's not exceeding number of processors in tjhe group.
  const int num_group_processors = _GetActiveProcessorCount(group_affinity.Group);
  if (num_group_processors < num_processors)
  {
    return num_group_processors;
  }
  return num_processors;
}

////////////////////////////////////////////////////////////////////////////////
// Affinities.

bool numaAPI_RunProcessOnNode(int node)
{
  // TODO(sergey): Make sure requested node is within active CPU group.
  // Change affinity of the proces to make it to run on a given node.
  HANDLE process_handle = GetCurrentProcess();
  GROUP_AFFINITY processor_mask = {0};
  if (_GetNumaNodeProcessorMaskEx(node, &processor_mask) == 0)
  {
    return false;
  }
  // TODO: Affinity should respect processor group.
  if (_SetProcessAffinityMask(process_handle, processor_mask.Mask) == 0)
  {
    return false;
  }
  return true;
}

bool numaAPI_RunThreadOnNode(int node)
{
  HANDLE thread_handle = GetCurrentThread();
  GROUP_AFFINITY group_affinity = {0};
  if (_GetNumaNodeProcessorMaskEx(node, &group_affinity) == 0)
  {
    return false;
  }
  if (_SetThreadGroupAffinity(thread_handle, &group_affinity, NULL) == 0)
  {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Memory management.

void *numaAPI_AllocateOnNode(size_t size, int node)
{
  return _VirtualAllocExNuma(GetCurrentProcess(),
                             NULL,
                             size,
                             MEM_RESERVE | MEM_COMMIT,
                             PAGE_READWRITE,
                             node);
}

void *numaAPI_AllocateLocal(size_t size)
{
  UCHAR current_processor = (UCHAR)_GetCurrentProcessorNumber();
  UCHAR node;
  if (!_GetNumaProcessorNode(current_processor, &node))
  {
    return NULL;
  }
  return numaAPI_AllocateOnNode(size, node);
}

void numaAPI_Free(void *start, size_t size)
{
  if (!_VirtualFree(start, size, MEM_RELEASE))
  {
    // TODO(sergey): Throw an error!
  }
}

#elif OS_LINUX  // OS_WIN

#  include <stdlib.h>

#  ifndef WITH_DYNLOAD
#    include <numa.h>
#  else
#    include <dlfcn.h>
#  endif

#  ifdef WITH_DYNLOAD

// Descriptor numa library.
static void *numa_lib;

// Types of all symbols which are read from the library.
struct bitmask;
typedef int tnuma_available(void);
typedef int tnuma_max_node(void);
typedef int tnuma_node_to_cpus(int node, struct bitmask *mask);
typedef long tnuma_node_size(int node, long *freep);
typedef int tnuma_run_on_node(int node);
typedef void *tnuma_alloc_onnode(size_t size, int node);
typedef void *tnuma_alloc_local(size_t size);
typedef void tnuma_free(void *start, size_t size);
typedef struct bitmask *tnuma_bitmask_clearall(struct bitmask *bitmask);
typedef int tnuma_bitmask_isbitset(const struct bitmask *bitmask, unsigned int n);
typedef struct bitmask *tnuma_bitmask_setbit(struct bitmask *bitmask, unsigned int n);
typedef unsigned int tnuma_bitmask_nbytes(struct bitmask *bitmask);
typedef void tnuma_bitmask_free(struct bitmask *bitmask);
typedef struct bitmask *tnuma_allocate_cpumask(void);
typedef struct bitmask *tnuma_allocate_nodemask(void);
typedef void tnuma_free_cpumask(struct bitmask *bitmask);
typedef void tnuma_free_nodemask(struct bitmask *bitmask);
typedef int tnuma_run_on_node_mask(struct bitmask *nodemask);
typedef int tnuma_run_on_node_mask_all(struct bitmask *nodemask);
typedef struct bitmask *tnuma_get_run_node_mask(void);
typedef void tnuma_set_interleave_mask(struct bitmask *nodemask);
typedef void tnuma_set_localalloc(void);

// Actual symbols.
static tnuma_available *numa_available;
static tnuma_max_node *numa_max_node;
static tnuma_node_to_cpus *numa_node_to_cpus;
static tnuma_node_size *numa_node_size;
static tnuma_run_on_node *numa_run_on_node;
static tnuma_alloc_onnode *numa_alloc_onnode;
static tnuma_alloc_local *numa_alloc_local;
static tnuma_free *numa_free;
static tnuma_bitmask_clearall *numa_bitmask_clearall;
static tnuma_bitmask_isbitset *numa_bitmask_isbitset;
static tnuma_bitmask_setbit *numa_bitmask_setbit;
static tnuma_bitmask_nbytes *numa_bitmask_nbytes;
static tnuma_bitmask_free *numa_bitmask_free;
static tnuma_allocate_cpumask *numa_allocate_cpumask;
static tnuma_allocate_nodemask *numa_allocate_nodemask;
static tnuma_free_nodemask *numa_free_nodemask;
static tnuma_free_cpumask *numa_free_cpumask;
static tnuma_run_on_node_mask *numa_run_on_node_mask;
static tnuma_run_on_node_mask_all *numa_run_on_node_mask_all;
static tnuma_get_run_node_mask *numa_get_run_node_mask;
static tnuma_set_interleave_mask *numa_set_interleave_mask;
static tnuma_set_localalloc *numa_set_localalloc;

static void *findLibrary(const char **paths)
{
  int i = 0;
  while (paths[i] != NULL)
  {
    void *lib = dlopen(paths[i], RTLD_LAZY);
    if (lib != NULL)
    {
      return lib;
    }
    ++i;
  }
  return NULL;
}

static void numaExit(void)
{
  if (numa_lib == NULL)
  {
    return;
  }
  dlclose(numa_lib);
  numa_lib = NULL;
}

static NUMAAPI_Result loadNumaSymbols(void)
{
  // Prevent multiple initializations.
  static bool initialized = false;
  static NUMAAPI_Result result = NUMAAPI_NOT_AVAILABLE;
  if (initialized)
  {
    return result;
  }
  initialized = true;
  // Find appropriate .so library.
  const char *numa_paths[] = {"libnuma.so.1", "libnuma.so", NULL};
  // Register de-initialization.
  const int error = atexit(numaExit);
  if (error)
  {
    result = NUMAAPI_ERROR_ATEXIT;
    return result;
  }
  // Load library.
  numa_lib = findLibrary(numa_paths);
  if (numa_lib == NULL)
  {
    result = NUMAAPI_NOT_AVAILABLE;
    return result;
  }
  // Load symbols.

#    define _LIBRARY_FIND(lib, name)         \
      do                                     \
      {                                      \
        name = (t##name *)dlsym(lib, #name); \
      } while (0)
#    define NUMA_LIBRARY_FIND(name) _LIBRARY_FIND(numa_lib, name)

  NUMA_LIBRARY_FIND(numa_available);
  NUMA_LIBRARY_FIND(numa_max_node);
  NUMA_LIBRARY_FIND(numa_node_to_cpus);
  NUMA_LIBRARY_FIND(numa_node_size);
  NUMA_LIBRARY_FIND(numa_run_on_node);
  NUMA_LIBRARY_FIND(numa_alloc_onnode);
  NUMA_LIBRARY_FIND(numa_alloc_local);
  NUMA_LIBRARY_FIND(numa_free);
  NUMA_LIBRARY_FIND(numa_bitmask_clearall);
  NUMA_LIBRARY_FIND(numa_bitmask_isbitset);
  NUMA_LIBRARY_FIND(numa_bitmask_setbit);
  NUMA_LIBRARY_FIND(numa_bitmask_nbytes);
  NUMA_LIBRARY_FIND(numa_bitmask_free);
  NUMA_LIBRARY_FIND(numa_allocate_cpumask);
  NUMA_LIBRARY_FIND(numa_allocate_nodemask);
  NUMA_LIBRARY_FIND(numa_free_cpumask);
  NUMA_LIBRARY_FIND(numa_free_nodemask);
  NUMA_LIBRARY_FIND(numa_run_on_node_mask);
  NUMA_LIBRARY_FIND(numa_run_on_node_mask_all);
  NUMA_LIBRARY_FIND(numa_get_run_node_mask);
  NUMA_LIBRARY_FIND(numa_set_interleave_mask);
  NUMA_LIBRARY_FIND(numa_set_localalloc);

#    undef NUMA_LIBRARY_FIND
#    undef _LIBRARY_FIND

  result = NUMAAPI_SUCCESS;
  return result;
}
#  endif

////////////////////////////////////////////////////////////////////////////////
// Initialization.

NUMAAPI_Result numaAPI_Initialize(void)
{
#  ifdef WITH_DYNLOAD
  NUMAAPI_Result result = loadNumaSymbols();
  if (result != NUMAAPI_SUCCESS)
  {
    return result;
  }
#  endif
  if (numa_available() < 0)
  {
    return NUMAAPI_NOT_AVAILABLE;
  }
  return NUMAAPI_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// Topology query.

int numaAPI_GetNumNodes(void)
{
  return numa_max_node() + 1;
}

bool numaAPI_IsNodeAvailable(int node)
{
  return numaAPI_GetNumNodeProcessors(node) > 0;
}

int numaAPI_GetNumNodeProcessors(int node)
{
  struct bitmask *cpu_mask = numa_allocate_cpumask();
  numa_node_to_cpus(node, cpu_mask);
  const unsigned int num_bytes = numa_bitmask_nbytes(cpu_mask);
  const unsigned int num_bits = num_bytes * 8;
  // TODO(sergey): There might be faster way calculating number of set bits.
  int num_processors = 0;
  for (unsigned int bit = 0; bit < num_bits; ++bit)
  {
    if (numa_bitmask_isbitset(cpu_mask, bit))
    {
      ++num_processors;
    }
  }
#  ifdef WITH_DYNLOAD
  if (numa_free_cpumask != NULL)
  {
    numa_free_cpumask(cpu_mask);
  } else
  {
    numa_bitmask_free(cpu_mask);
  }
#  else
  numa_free_cpumask(cpu_mask);
#  endif
  return num_processors;
}

////////////////////////////////////////////////////////////////////////////////
// Topology helpers.

int numaAPI_GetNumCurrentNodesProcessors(void)
{
  struct bitmask *node_mask = numa_get_run_node_mask();
  const unsigned int num_bytes = numa_bitmask_nbytes(node_mask);
  const unsigned int num_bits = num_bytes * 8;
  int num_processors = 0;
  for (unsigned int bit = 0; bit < num_bits; ++bit)
  {
    if (numa_bitmask_isbitset(node_mask, bit))
    {
      num_processors += numaAPI_GetNumNodeProcessors(bit);
    }
  }
  numa_bitmask_free(node_mask);
  return num_processors;
}

////////////////////////////////////////////////////////////////////////////////
// Affinities.

bool numaAPI_RunProcessOnNode(int node)
{
  numaAPI_RunThreadOnNode(node);
  return true;
}

bool numaAPI_RunThreadOnNode(int node)
{
  // Construct bit mask from node index.
  struct bitmask *node_mask = numa_allocate_nodemask();
  numa_bitmask_clearall(node_mask);
  numa_bitmask_setbit(node_mask, node);
  numa_run_on_node_mask_all(node_mask);
  // TODO(sergey): The following commands are based on x265 code, we might want
  // to make those optional, or require to call those explicitly.
  //
  // Current assumption is that this is similar to SetThreadGroupAffinity().
  if (numa_node_size(node, NULL) > 0)
  {
    numa_set_interleave_mask(node_mask);
    numa_set_localalloc();
  }
#  ifdef WITH_DYNLOAD
  if (numa_free_nodemask != NULL)
  {
    numa_free_nodemask(node_mask);
  } else
  {
    numa_bitmask_free(node_mask);
  }
#  else
  numa_free_nodemask(node_mask);
#  endif
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Memory management.

void *numaAPI_AllocateOnNode(size_t size, int node)
{
  return numa_alloc_onnode(size, node);
}

void *numaAPI_AllocateLocal(size_t size)
{
  return numa_alloc_local(size);
}

void numaAPI_Free(void *start, size_t size)
{
  numa_free(start, size);
}

#endif /* OS_LINUX */