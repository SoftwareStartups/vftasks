/* Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */
#ifndef __VFTASKS_H
#define __VFTASKS_H

/** \mainpage vfTasks library API documentation
 *
 * \section sec_intro Introduction
 *  This document describes the concepts behind Vector Fabrics' vfStream
 *  library and the programming interface to it.
 *  Vector Fabrics' vfStream is a library to communicate data items between
 *  different threads in a First-In-First-Out (FIFO) ordering.
 *  Communication between threads requires synchronization, and synchronization
 *  overhead may introduce significant slowdowns in a multi-threaded program.
 *  vfStream is designed such that the communication overheads can be tuned
 *  and hence avoid the slowdown brought on by excessive synchronization.
 *
 * \section sec_fifo_intro FIFO channels and cross-thread communication
 *  A FIFO queue can be regarded as uni-directional communication
 *  channel between writer and reader code.
 *  The FIFO queue ensures that the data items are consumed by readers
 *  in the same order as writers produced the data.
 *  In multithreaded programs, FIFO queues can be used to communicate
 *  writers and readers that run concurrently in different threads.
 *  @verbatim
                 +--------------+               +-----------+
                 |              |     FIFO      |           |
                 |   writer     |-------------->| reader    |
                 |  thread #1   |               | thread #2 |
                 |              |               |           |
                 +--------------+               +-----------+
    @endverbatim
 *  A FIFO channel between threads requires synchronization between
 *  the writer and the reader. If a FIFO becomes empty, the writer
 *  has to wait for data items to become available on the FIFO. This is
 *  usually implemented by blocking:
 *     - the reader thread blocks when the FIFO is empty,
 *     - when new data becomes available, the reader thread
 *       is awakened and resumes computation.
 *
 *  The same blocking behaviour can be used when the FIFO becomes full.
 *  FIFOs usually have a finite capacity for storing data items.
 *  If the writer finds that the FIFO is full, it will block until
 *  space in the FIFO becomes available as a result of the reader
 *  consuming data items.
 *
 * \section sec_overhead Synchronization overhead
 *  If the reader cannot keep up with the writer, i.e. the writer
 *  produces items faster than the reader can consume, the
 *  writer will frequently become blocked, waiting on the FIFO to
 *  become non-full.
 *
 *  In such situations, the following sequence of events can be observed:
 *     - the writer computes new data and blocks on a full FIFO,
 *     - the reader consumes a data item from the FIFO,
 *     - now that the FIFO is not full the writer
 *       awakens and pushes an item into the FIFO,
 *     - the sequence of events restarts. 
 *
 *  There is overhead associated
 *  with blocking and waking a thread. In the example just described the
 *  blocking/waking is too frequent. It follows that the overhead
 *  may be significant and slowdown the program noticeably.
 *
 * \section sec_watermarks Watermarks: avoiding synchronization overhead.
 *  The key idea behind vfStream is to use tunable parameters to reduce
 *  the frequency at which writer and reader threads are blocked and awakened.
 *  This allows the user of the library to keep synchronization overhead
 *  reasonable and therefore avoid a noticeable slowdown of the program
 *  using the library.
 *  @verbatim
                 +--------------+               +-----------+
                 |              |     FIFO      |           |
                 |   writer     |-------------->| reader    |
                 |  thread #1   | | 1| 2| 3| 4| | thread #2 |
                 |              |-------------->|           |
                 +--------------+               +-----------+
    @endverbatim
 *  Suppose the reader becomes blocked because the FIFO is currently
 *  empty. vfStream has a parameter that controls when the reader
 *  will be woken up, this parameter is called a reader watermark. The
 *  reader watermark parameter tells vfStream how many elements
 *  there must be in the FIFO in order to wake up the reader thread.
 *  In this way, vfStream can ensure that there is a reasonable
 *  amount of work for the reader to perform before it is blocked again.
 *
 *  Similarly, vfStream supports setting a writer watermark. This watermark
 *  specifies how many slots must be available in the FIFO before the
 *  writer thread is woken up.
 *
 *  Reader/writer watermarks are set using the functions vfstream_set_min_data() and
 *  vfstream_set_min_room().
 *
 * \section sec_block_spin Blocking or spinning
 *  vfStream allows the programmer to specify the behavior of reader/writer
 *  threads when the FIFO is empty/full. The default behavior is to let
 *  the thread to spin while the resource is unavailable. For instance,
 *  if the reader thread tries to consume an empty FIFO, it will loop
 *  (therefore using CPU resources) until data becomes available on
 *  the FIFO.
 *
 *  vfStream also allows the programmer to specify blocking behavior by
 *  means of hooks. The hooks can be set using the function
 *  vfstream_install_chan_hooks().
 *
 * \section sec_tokens_stuff Tokens, channels and ports
 *  Tokens are the basic unit of information that can be communicated
 *  between readers and writers. Tokens are associated with buffers
 *  that contain the data.
 *  Using tokens, the communication takes place as follows:
 *     - the writer acquires a token by calling vfstream_acquire_room(),
 *     - the writer writes the data into the token and releases it by
 *       calling vfstream_release_data(),
 *     - the reader receives a token of data from the writer by calling
 *       vfstream_acquire_data(),
 *     - the reader consumes the data and when done, releases the token
 *       by calling vfstream_release_room().
 *
 *  In order to use a vfStream FIFO channel, the program must create:
 *     - a channel, by means of vfstream_create_chan()
 *     - a read port, by means of vfstream_create_read_port()
 *     - a write port, by means of vfstream_create_write_port()
 *
 *  When creating a channel, the program must specify the amount of
 *  tokens it can contain, and the size of each token.
 *
 * \section sec_memory Custom memory management
 *  Channel and channel port functions take additional arguments
 *  for memory allocation and deallocation. In most common cases,
 *  it is sufficient to use C's malloc and free functions:
 *  @verbatim
  vfstream_malloc_t mem_mgr;
  mem_mgr.malloc = malloc;
  mem_mgr.free = free;
    @endverbatim
 *  In some cases, however, it is desirable to have additional control
 *  on what areas of memory are used for allocation.
 *  This could be useful, for example, when interfacing with hardware devices.
 * \section sec_error_handling Error handling
 *  By default, members of the vfSync API terminate the calling program when a failure
 *  is encountered.
 *  This behaviour can be overriden by compiling vfSync with the
 *  VFSYNC_ABORT_ON_FAILURE preprocessor symbols undefined.
 */


#include <stdint.h>    /* for int8_t, int16_t, ... */
#include <stddef.h>    /* for size_t, and NULL     */

/* Must fix the stuff below. false needs to be included from stdbool.h
 * however including stdbool.h is tricky, such code does not even compile
 * in some systems, look for nptypes.h in google code search to see why. */
#if defined(false) || defined(__cplusplus)
typedef bool bool_t;
#else
typedef int bool_t;
#define false 0
#define true 1
#endif


/* ***************************************************************************
 * Worker thread pools
 * ***************************************************************************/

/* ***************************************************************************
 * Types
 * ***************************************************************************/

/** Represents a worker-thread pool.
 */
typedef struct vftasks_pool_s vftasks_pool_t;

/** Represents a task that is to be executed in the worker-thread pool.
 */
typedef void *(vftasks_task_t)(void *);

/* ***************************************************************************
 * Creation and destruction of worker-thread pools
 * ***************************************************************************/

/** Creates a worker-thread pool of a given size.
 *
 *  @param  num_workers  The number of worker threads in the pool.
 *
 *  @return
 *    On success, a pointer to the pool.
 *    On failure, NULL.
 *
 *  NOTE: If vfTasks was compiled with the VFTASKS_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
 */
vftasks_pool_t *vftasks_create_pool(int num_workers);

/** Destroys a given worker-thread pool.
 *
 *  @param  pool  A pointer to the pool.
 */
void vftasks_destroy_pool(vftasks_pool_t *pool);

/* ***************************************************************************
 * Execution of parallel tasks
 * ***************************************************************************/

/** Submits a specified instance of a task to a given worker-thread pool.
 *
 *  Executes the task in parallel on a thread from the pool.
 *
 *  @param  pool         A pointer to the pool.
 *  @param  task         A pointer to the task.
 *  @param  args         A pointer to the arguments for the instance.
 *  @param  num_workers  The number of additional worker threads that will be required
 *                       for the execution of the task.
 *
 *  @return
 *    On success, 0
 *    On failure, a nonzero value.
 *
 *  NOTE: If vfTasks was compiled with the VFTASKS_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
 */
int vftasks_submit(vftasks_pool_t *pool,
                   vftasks_task_t *task,
                   void *args,
                   int num_workers);

/** Retrieves the result of a task that is being executed in a given worker-thread pool.
 *
 *  Blocks until execution of the task has completed and then stores the result in the
 *  location that is referenced by result.
 *
 *  @param  pool    A pointer to the pool.
 *  @param  result  A reference to the location into which the result is to be stored.
 *                  If result is NULL, the result is not stored.
 *
 *  @return
 *    On success, 0.
 *    On failure, a nonzero value.
 *
 *  NOTE: If vfTasks was compiled with the VFTASKS_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
 */
int vftasks_get(vftasks_pool_t *pool, void **result);


/* ***************************************************************************
 * Two-dimensional synchronization between tasks
 * ***************************************************************************/

/** A handle that is to be used to manage two-dimensional synchronization between
 *  concurrent tasks.
 */
typedef struct vftasks_2d_sync_mgr_s vftasks_2d_sync_mgr_t;

/** Creates a handle for managing two-dimensional synchronization between concurrent
 *  tasks.
 *
 *  @param dim_x   The size of the first dimension of the joint iteration space of the
 *                 concurrent tasks.
 *  @param dim_y   The size of the second dimension of the joint iteration space of the
 *                 concurrent tasks.
 *  @param dist_x  The critical dependency distance along the first dimension of the
 *                 joint iteration space of the concurrent tasks.
 *  @param dist_y  The ciritial dependency distance along the second dimention of the
 *                 joint iteration space of the concurrent tasks.
 *
 *  @return
 *    On success, a pointer to the handle.
 *    On failure, NULL.
 *
 *  NOTE: If vfTasks was compiled with the VFTASKS_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
 */
vftasks_2d_sync_mgr_t *vftasks_create_2d_sync_mgr(int dim_x,
                                                  int dim_y,
                                                  int dist_x,
                                                  int dist_y);

/** Destroys a given handle for managing two-dimension synchronization between
 *  concurrent tasks.
 *
 *  @mgr  A pointer to the handle.
 */
void vftasks_destroy_2d_sync_mgr(vftasks_2d_sync_mgr_t *mgr);

/** Signals the completion of an inner iteration through a handle for managing
 *  two-dimensional synchronization between concurrent task.
 *
 *  @param mgr  A pointer to the handle.
 *  @param x    The iteration's first-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 *  @param y    The iteration's second-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 *
 *  @return
 *    On success, 0.
 *    On failure, a nonzero value.
 *
 *  NOTE: If vfTasks was compiled with the VFTASKS_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
 */
int vftasks_signal_2d(vftasks_2d_sync_mgr_t *mgr, int x, int y);

/** Synchronizes a task at the start of an inner iteration with the tasks it is
 *  depending on.
 *
 *  @param mgr  A pointer to the handle that manages synchronization.
 *  @param x    The iteration's first-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 *  @param y    The iteration's second-dimension index into the joint iteration space of
 *              the concurrent tasks for the iteration.
 *
 *  @return
 *    On success, 0.
 *    On failure, a nonzero value.
 *
 *  NOTE: If vfTasks was compiled with the VFTASKS_ABORT_ON_FAILURE preprocessor symbol
 *  defined (which is the default), the function does not return on failure and instead
 *  terminates the calling program.
 */
int vftasks_wait_2d(vftasks_2d_sync_mgr_t *mgr, int x, int y);


/* ***************************************************************************
 * FIFO channels
 * ***************************************************************************/

/* ***************************************************************************
 * Types
 * ***************************************************************************/

/** Represents a FIFO channel.
 */
typedef struct vftasks_chan_s vftasks_chan_t;

/** Represents a write port on a FIFO channel.
 */
typedef struct vftasks_wport_s vftasks_wport_t;

/** Represents a read port on a FIFO channel.
 */
typedef struct vftasks_rport_s vftasks_rport_t;

/**
 * Represents a FIFO token.
 */
typedef struct vftasks_token_s vftasks_token_t;

/** Called when a writer that is connected to a FIFO channel might want to be
 *  suspended or resumed.
 *
 *  @param  wport  A pointer to the write port through which the reader is connected
 *                 to the channel.
 */
typedef void (*vftasks_writer_hook_t)(vftasks_wport_t *wport);

/** Called when a reader that is connected to a FIFO channel might want to be
 *  suspended or resumed.
 *
 *  @param  rport  A pointer to the read port through which the reader is connected
 *                 to the channel.
 */
typedef void (*vftasks_reader_hook_t)(vftasks_rport_t *rport);

/** Provides a memory-management implementation.
 */
typedef struct vftasks_malloc_s
{
  /** Allocates a memory block of a specified size.
   *
   *  @param size  The size of the memory block
   *
   *  @return
   *    On success, a pointer to the memory block.
   *    On failure, NULL.
   */
  void *(*malloc)(size_t size);

  /** Deallocates a previously allocated block of memory.
   *
   *  @param  ptr  A pointer to the memory block.
   */
  void (*free)(void *ptr);
}
vftasks_malloc_t;


/* ***************************************************************************
 * Creation and destruction of channels and ports
 * ***************************************************************************/

/** Creates a FIFO channel with a buffer to accomodate a specified number of
 *  tokens of a given token size.
 *
 *  If the token size is not a power of two, it is rounded up to the next power
 *  of two.
 *
 *  @param  num_tokens  The number of tokens.
 *  @param  token_size  The token size.
 *  @param  ctl_space   A pointer to the memory-management implementation that is to
 *                      be used to allocate memory for the channel's control
 *                      structure.
 *  @param  buf_space   A pointer to the memory-management implementation that is to
 *                      be used to allocate memory for the channel's FIFO buffer.
 *
 *  @return
 *     On success, a pointer to the channel.
 *     On failure, NULL.
 */
vftasks_chan_t *vftasks_create_chan(int num_tokens,
                                    size_t token_size,
                                    vftasks_malloc_t *ctl_space,
                                    vftasks_malloc_t *buf_space);

/** Creates a write port and connects it to a given FIFO channel.
 *
 *  Fails if another write port is already connected to the channel.
 *
 *  @param   chan        A pointer to the FIFO channel.
 *  @param   port_space  A pointer to the memory-management implementation that is
 *                       to be used to allocate memory for the port.
 *
 *  @return
 *     On success, a pointer to the port.
 *     On failure, NULL.
 */
vftasks_wport_t *vftasks_create_write_port(vftasks_chan_t *chan,
                                           vftasks_malloc_t *port_space);

/** Creates a read port and connects it to a given FIFO channel.
 *
 *  Fails if another read port is already connected to the channel.
 *
 *  @param  chan        A pointer to the FIFO channel.
 *  @param  port_space  A pointer to the memory-management implementation that is
 *                      to be used to allocate memory for the port.
 *
 *  @return
 *      On success, a pointer to the port.
 *      On failure, NULL.
 */
vftasks_rport_t *vftasks_create_read_port(vftasks_chan_t *chan,
                                          vftasks_malloc_t *port_space);

/** Destroys a given FIFO channel.
 *
 *  @param  chan       A pointer to the channel.
 *  @param  ctl_space  A pointer to the memory-management implementation that is to
 *                     be used to deallocate the memory that was allocated for the
 *                     channel's control structure.
 *  @param  buf_space  A pointer to the memory-management implementation that is to
 *                     be used to deallocate the memory that was allocated for the
 *                     channel's FIFO buffer.
 */
void vftasks_destroy_chan(vftasks_chan_t *chan,
                          vftasks_malloc_t *ctl_space,
                          vftasks_malloc_t *buf_space);

/** Destroys a given write port.
 *
 *  @param  wport       A pointer to the port.
 *  @param  port_space  A pointer to the memory-management implementation that is to
 *                      be used to deallocate the memory that was allocated for the
 *                      port.
 */
void vftasks_destroy_write_port(vftasks_wport_t *wport,
                               vftasks_malloc_t *port_space);

/** Destroys a given read port.
 *
 *  @param  rport       A pointer to the port.
 *  @param  port_space  A pointer to the memory-management implementation that is to
 *                      be used to deallocate the memory that was allocated for the
 *                      port.
 */
void vftasks_destroy_read_port(vftasks_rport_t *rport,
                               vftasks_malloc_t *port_space);


/* ***************************************************************************
 * Channel hooks
 * ***************************************************************************/

/** Installs hooks into a FIFO channel.
 *
 *  @param  chan            A pointer to the channel.
 *  @param  suspend_writer  A pointer to the function that is to be called when a
 *                          writer connected to the channel might want to be
 *                          suspended.
 *  @param  resume_writer   A pointer to the function that is to be called when a
 *                          writer connected to the channel might want to be resumed.
 *  @param  suspend_reader  A pointer to the function that is to be called when a
 *                          reader connected to the channel might want to be
 *                          suspended.
 *  @param  resume_reader   A pointer to the function that is to be called when a
 *                          reader connected to the channel might want to be resumed.
 */
void vftasks_install_chan_hooks(vftasks_chan_t *chan,
                                vftasks_writer_hook_t suspend_writer,
                                vftasks_writer_hook_t resume_writer,
                                vftasks_reader_hook_t suspend_reader,
                                vftasks_reader_hook_t resume_reader);

/* ***************************************************************************
 * Low- and high-water marks
 * ***************************************************************************/

/** Sets the ``low-water mark'' for a given FIFO channel.
 *
 *  The low-water mark signals the number of tokens that triggers writers to the
 *  channel to be prompted to resume their task.
 *  That is, if the number of tokens available for writing becomes equal to or
 *  greater than the low-water mark, writers are notified through a hook that
 *  was installed by vftasks_install_chan_hooks.
 *
 *  @param  chan      A pointer to the channel.
 *  @param  min_room  The new low-water mark.
 *
 *  @return
 *    On success, the new low-water mark.
 *    On failure, the old low-water mark.
 */
int vftasks_set_min_room(vftasks_chan_t *chan, int min_room);

/** Retrieves the ``low-water mark'' for a given FIFO channel.
 *
 *  The low-water mark signals the number of tokens that triggers writers to the
 *  channel to be prompted to resume their task.
 *  That is, if the number of tokens available for writing becomes equal to or
 *  greater than the low-water mark, writers are notified through a hook that
 *  was installed by vftasks_install_chan_hooks.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    The low-water mark.
 */
int vftasks_get_min_room(vftasks_chan_t *chan);

/** Sets the ``high-water mark'' for a given FIFO channel.
 *
 *  The high-water mark signals the number of tokens that triggers readers from
 *  the channel to be prompted to resume their task.
 *  That is, if the number of tokens available for reading becomes equal to or
 *  greater than the high-water mark, readers are notified through a hook that
 *  was installed by vftasks_install_chan_hooks.
 *
 *  @param  chan      A pointer to the channel.
 *  @param  min_data  The new high-water mark.
 *
 *  @return
 *    On success, the new high-water mark.
 *    On failure, the old high-water mark.
 */
int vftasks_set_min_data(vftasks_chan_t *chan, int min_data);

/** Retrieves the ``high-water mark'' for a given FIFO channel.
 *
 *  The high-water mark signals the number of tokens that triggers readers from
 *  the channel to be prompted to resume their task.
 *  That is, if the number of tokens available for reading becomes equal to or
 *  greater than the high-water mark, readers are notified through a hook that
 *  was installed by vftasks_install_chan_hooks.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    The high-water mark.
 */
int vftasks_get_min_data(vftasks_chan_t *chan);


/* ***************************************************************************
 * Application-specific data
 * ***************************************************************************/

/** Associates application-specific data with a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *  @param  info  A pointer to the application-specific data.
 */
void vftasks_set_chan_info(vftasks_chan_t *chan, void *info);

/** Retrieves application-specific data from a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    A pointer to the application-specific data.
 */
void *vftasks_get_chan_info(vftasks_chan_t *chan);


/* ***************************************************************************
 * Channel queries
 * ***************************************************************************/

/** Retrieves whether or not a given FIFO channel supports shared-memory
 *  operations.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    If the channel supports shared-memory operations, true.
 *    If the channel does not support shared-memory operations, false.
 */
bool_t vftasks_shmem_supported(vftasks_chan_t *chan);

/** Retrieves the number of tokens held by a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    The number of tokens.
 */
int vftasks_get_num_tokens(vftasks_chan_t *chan);

/** Retrieves the size of the tokens held by a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    The token size.
 */
size_t vftasks_get_token_size(vftasks_chan_t *chan);


/* ***************************************************************************
 * Port queries
 * ***************************************************************************/

/** Retrieves the FIFO channel that is connected to a given write port.
 *
 *  @param  wport  A pointer to the port.
 *
 *  @return
 *    A pointer to the channel.
 */
vftasks_chan_t *vftasks_chan_of_wport(vftasks_wport_t *wport);

/** Retrieves the FIFO channel that is connected to a given read port.
 *
 *  @param rport  A pointer to the port.
 *
 *  @return
 *    A pointer to the channel.
 */
vftasks_chan_t *vftasks_chan_of_rport(vftasks_rport_t *rport);


/* ***************************************************************************
 * Channel-state queries
 * ***************************************************************************/

/** Retrieves whether or not a connected FIFO channel has any tokens available 
 *  for acquisition through a given write port.
 *
 *  @param  wport  A pointer to the port.
 *
 *  @return
 *    If the channel has tokens available, true.
 *    If the channel does not have tokens available, false. *
 */
bool_t vftasks_room_available(vftasks_wport_t *wport);

/** Retrieves whether or not a connected FIFO channel has any tokens available 
 *  for acquisition through a given read port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    If the channel has tokens available, true.
 *    If the channel does not have tokens available, false.
 */
bool_t vftasks_data_available(vftasks_rport_t *rport);


/* ***************************************************************************
 * Producer operations
 * ***************************************************************************/

/** Acquires a token from a FIFO channel through a given write port.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *  If there are no tokens available, writers are prompted to be suspended
 *  through a hook that that was installed by vftasks_install_chan_hooks.
 *  Before actually being suspended, writers should make sure that the
 *  suspension condition still applies as concurrent consumer operations may
 *  have made tokens available.
 *
 *
 *  @param  wport  A pointer to the port.
 *
 *  @return
 *    A pointer to the token.
 */
vftasks_token_t *vftasks_acquire_room(vftasks_wport_t *wport);

/** Attempts to acquire a token from a FIFO channel through a given write port.
 *
 *  Fails if the channel does not have tokens available for acquisition through
 *  the port.
 *
 *  @param  wport  A pointer to the port.
 *
 *  @return
 *    On success, a pointer to the token.
 *    On failure, NULL.
 */
vftasks_token_t *vftasks_acquire_room_nb(vftasks_wport_t *wport);

/** Releases a token that was previously acquired through a given write port.
 *
 *  If the number of available tokens for acquisition through a read port that
 *  is connected to the channel rises above the ``high-water mark'', readers are
 *  prompted to resume through a hook that was installed by
 *  vftasks_install_chan_hooks.
 *
 *  @param  wport  A pointer to the port.
 *  @param  token  A pointer to the token.
 */
void vftasks_release_data(vftasks_wport_t *wport, vftasks_token_t *token);


/* ***************************************************************************
 * Consumer operations
 * ***************************************************************************/

/** Acquires a token from a FIFO channel through a given read port.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *  If there are no tokens available, readers are prompted to be suspended
 *  through a hook that that was installed by vftasks_install_chan_hooks.
 *  Before actually being suspended, writers should make sure that the
 *  suspension condition still applies as concurrent producer operations may
 *  have made tokens available.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    A pointer to the token.
 */
vftasks_token_t *vftasks_acquire_data(vftasks_rport_t *rport);

/** Attempts to acquire a token from a FIFO channel through a given read port.
 *
 *  Fails if the channel does not have tokens available for acquisition through
 *  the port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    On success, a pointer to the token.
 *    On failure, NULL.
 */
vftasks_token_t *vftasks_acquire_data_nb(vftasks_rport_t *rport);

/** Releases a token that was previously acquired through a given write port.
 *
 *  If the number of available tokens for acquisition through a write port that
 *  is connected to the channel rises above the ``low-water mark'', writers are
 *  prompted to resume through a hook that was installed by
 *  vftasks_install_chan_hooks.
 *
 *  @param  rport  A pointer to the port.
 *  @param  token  A pointer to the token.
 */
void vftasks_release_room(vftasks_rport_t *rport, vftasks_token_t *token);


/* ***************************************************************************
 * Shared-memory-mode operations
 * ***************************************************************************/

/** Retrieves a pointer to the start the FIFO-channel buffer range that is
 *  associated with a given token.
 *
 *  Fails if the channel does not support shared-memory operations.
 *
 *  @param  token  A pointer to the token.
 *
 *  @return
 *    On success, the pointer.
 *    On failure, NULL.
 */
void *vftasks_get_memaddr(vftasks_token_t *token);


/* ***************************************************************************
 * Low-level operations
 * ***************************************************************************/

/** Stores an 8-bit integer value at a specified offset in the FIFO-channel
 *  buffer range that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *  @param  data    The value.
 */
void vftasks_put_int8(vftasks_token_t* token, size_t offset, int8_t data);

/** Stores a 16-bit integer value at a specified offset in the FIFO-channel
 * buffer range that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value does not fit beyond the offset (i.e., if the size of the value
 *  is greater than the size of the tokens held by the channel minus the
 *  offset), the value is chunked into two parts: one part that exactly fits the
 *  space beyond the offset and one part that contains the remaining data.
 *  The first part is stored beyond the offset, the second part in a dedicated
 *  overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *  @param  data    The value.
 */
void vftasks_put_int16(vftasks_token_t *token, size_t offset, int16_t data);

/** Stores a 32-bit integer value at a specified offset in the FIFO-channel
 *  buffer range that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value does not fit beyond the offset (i.e., if the size of the value
 *  is greater than the size of the tokens held by the channel minus the
 *  offset), the value is chunked into two parts: one part that exactly fits the
 *  space beyond the offset and one part that contains the remaining data.
 *  The first part is stored beyond the offset, the second part in a dedicated
 *  overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *  @param  data    The value.
 */
void vftasks_put_int32(vftasks_token_t *token, size_t offset, int32_t data);

/** Stores a 64-bit integer value at a specified offset in the FIFO-channel
 *  buffer range that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value does not fit beyond the offset (i.e., if the size of the value
 *  is greater than the size of the tokens held by the channel minus the
 *  offset), the value is chunked into two parts: one part that exactly fits the
 *  space beyond the offset and one part that contains the remaining data.
 *  The first part is stored beyond the offset, the second part in a dedicated
 *  overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *  @param  data    The value.
 */
void vftasks_put_int64(vftasks_token_t *token, size_t offset, int64_t data);

/** Stores a single-precision floating-point value at a specified offset in the
 *  FIFO-channel buffer range that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value does not fit beyond the offset (i.e., if the size of the value
 *  is greater than the size of the tokens held by the channel minus the
 *  offset), the value is chunked into two parts: one part that exactly fits the
 *  space beyond the offset and one part that contains the remaining data.
 *  The first part is stored beyond the offset, the second part in a dedicated
 *  overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *  @param  data    The value.
 */
void vftasks_put_float(vftasks_token_t *token, size_t offset, float data);

/** Stores a double-precision floating-point value at a specified offset in the
 *  FIFO-channel buffer range that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value does not fit beyond the offset (i.e., if the size of the value
 *  is greater than the size of the tokens held by the channel minus the
 *  offset), the value is chunked into two parts: one part that exactly fits the
 *  space beyond the offset and one part that contains the remaining data.
 *  The first part is stored beyond the offset, the second part in a dedicated
 *  overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *  @param  data    The value.
 */
void vftasks_put_double(vftasks_token_t *token, size_t offset, double data);

/** Stores a pointer value at a specified offset in the FIFO-channel buffer
 *  range that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value does not fit beyond the offset (i.e., if the size of the value
 *  is greater than the size of the tokens held by the channel minus the
 *  offset), the value is chunked into two parts: one part that exactly fits the
 *  space beyond the offset and one part that contains the remaining data.
 *  The first part is stored beyond the offset, the second part in a dedicated
 *  overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *  @param  data    The value.
 */
void vftasks_put_ptr(vftasks_token_t *token, size_t offset, void *data);

/** Loads an 8-bit integer value from a specified offset in the FIFO-channel
 *  buffer that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *
 *  @return
 *    The value.
 */
int8_t vftasks_get_int8(vftasks_token_t *token, size_t offset);

/** Loads a 16-bit integer value from a specified offset in the FIFO-channel
 *  buffer that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value requires more space than is available beyond the offset (i.e.,
 *  if the size of the value is greater than the size of the tokens held by the
 *  channel minus the offset), the value is retrieved in two parts: one part
 *  that exactly fits the space beyond the offset and one part that contains
 *  the remaining data.
 *  The first part is retrieved from beyond the offset, the second part from a
 *  a dedicated overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *
 *  @return
 *    The value.
 */
int16_t vftasks_get_int16(vftasks_token_t *token, size_t offset);

/** Loads a 32-bit integer value from a specified offset in the FIFO-channel
 *  buffer that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value requires more space than is available beyond the offset (i.e.,
 *  if the size of the value is greater than the size of the tokens held by the
 *  channel minus the offset), the value is retrieved in two parts: one part
 *  that exactly fits the space beyond the offset and one part that contains
 *  the remaining data.
 *  The first part is retrieved from beyond the offset, the second part from a
 *  dedicated overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *
 *  @return
 *    The value.
 */
int32_t vftasks_get_int32(vftasks_token_t *token, size_t offset);

/** Loads a 64-bit integer value from a specified offset in the FIFO-channel
 *  buffer that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value requires more space than is available beyond the offset (i.e.,
 *  if the size of the value is greater than the size of the tokens held by the
 *  channel minus the offset), the value is retrieved in two parts: one part
 *  that exactly fits the space beyond the offset and one part that contains
 *  the remaining data.
 *  The first part is retrieved from beyond the offset, the second part from a
 *  dedicated overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *
 *  @return
 *    The value.
 */
int64_t vftasks_get_int64(vftasks_token_t *token, size_t offset);

/** Loads a single-precision floating-point value from a specified offset in the
 *  FIFO-channel buffer that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value requires more space than is available beyond the offset (i.e.,
 *  if the size of the value is greater than the size of the tokens held by the
 *  channel minus the offset), the value is retrieved in two parts: one part
 *  that exactly fits the space beyond the offset and one part that contains
 *  the remaining data.
 *  The first part is retrieved from beyond the offset, the second part from a
 *  dedicated overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *
 *  @return
 *    The value.
 */
float vftasks_get_float(vftasks_token_t *token, size_t offset);

/** Loads an double-precision floating-point value from a specified offset in
 *  the FIFO-channel buffer that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value requires more space than is available beyond the offset (i.e.,
 *  if the size of the value is greater than the size of the tokens held by the
 *  channel minus the offset), the value is retrieved in two parts: one part
 *  that exactly fits the space beyond the offset and one part that contains
 *  the remaining data.
 *  The first part is retrieved from beyond the offset, the second part from a
 *  dedicated overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *
 *  @return
 *    The value.
 */
double vftasks_get_double(vftasks_token_t *token, size_t offset);

/** Loads a pointer value from a specified offset in the FIFO-channel buffer
 *  that is associated with a given token.
 *
 *  If the offset is out of range (i.e., if it is either less than zero or
 *  otherwise greater than or equal to the size of the tokens held by the
 *  channel), it is wrapped around appositely.
 *
 *  If the value requires more space than is available beyond the offset (i.e.,
 *  if the size of the value is greater than the size of the tokens held by the
 *  channel minus the offset), the value is retrieved in two parts: one part
 *  that exactly fits the space beyond the offset and one part that contains
 *  the remaining data.
 *  The first part is retrieved from beyond the offset, the second part from a
 *  dedicated overflow buffer.
 *
 *  @param  token   A pointer to the token.
 *  @param  offset  The offset.
 *
 *  @return
 *    The value.
 */
void *vftasks_get_ptr(vftasks_token_t *token, size_t offset);


/* ***************************************************************************
 * Flushing
 * ***************************************************************************/

/** Prompts readers of a FIFO channel that is connected with a given write port
 *  to resume, regardless of the ``high-water mark'' of the channel.
 *
 *  That is, readers are notified through a hook that was installed by
 *  vftasks_install_chan_hooks.
 *  Typically called by a writer to signal that it has completed its task and
 *  that no more tokens should be expected to become available for reading.
 *
 *  @param  wport  A pointer to the write port.
 */
void vftasks_flush_data(vftasks_wport_t *wport);

/** Prompts writers of a FIFO channel that is connected with a given read port
 *  to resume, regardless of the ``low-water mark'' of the channel.
 *
 *  That is, writers are notified through a hook that was installed by
 *  vftasks_install_chan_hooks.
 *  Typically called by a reader to signal that it has completed its task and
 *  that no more tokens should be expected to become available for writing.
 *
 *  @param  rport  A pointer to the read port.
 */
void vftasks_flush_room(vftasks_rport_t *rport);


/* ***************************************************************************
 * High-level, Kahn-style operations
 * ***************************************************************************/

/** Writes an 8-bit integer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vftasks_write_int8(vftasks_wport_t *wport, int8_t data);

/** Writes a 16-bit integer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vftasks_write_int16(vftasks_wport_t *wport, int16_t data);

/** Writes a 32-bit integer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vftasks_write_int32(vftasks_wport_t *wport, int32_t data);

/** Writes a 64-bit integer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vftasks_write_int64(vftasks_wport_t *wport, int64_t data);

/** Writes a single-precision floating-point value through a given write port to
 *  a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vftasks_write_float(vftasks_wport_t *wport, float data);

/** Writes a double-precision floating-point value through a given write port to
 *  a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param wport  A pointer to the port.
 *  @param data   The value.
 */
void vftasks_write_double(vftasks_wport_t *wport, double data);

/** Writes a pointer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vftasks_write_ptr(vftasks_wport_t *wport, void *data);

/** Reads an 8-bit integer value through a given read port from a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    The value.
 */
int8_t vftasks_read_int8(vftasks_rport_t *rport);

/** Reads a 16-bit integer value through a given read port from a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    The value.
 */
int16_t vftasks_read_int16(vftasks_rport_t *rport);

/** Reads a 32-bit integer value through a given read port from a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    The value.
 */
int32_t vftasks_read_int32(vftasks_rport_t *rport);

/** Reads a 64-bit integer value through a given read port from a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    The value.
 */
int64_t vftasks_read_int64(vftasks_rport_t *rport);

/** Reads a single-precision floating-point value through a given read port from
 *  a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    The value.
 */
float vftasks_read_float(vftasks_rport_t *rport);

/** Reads an double-precision floating-point value through a given read port
 *  from a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    The value.
 */
double vftasks_read_double(vftasks_rport_t *rport);

/** Reads a pointer value through a given read port from a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    The value.
 */
void *vftasks_read_ptr(vftasks_rport_t *rport);


#endif /* __VFTASKS_H */

