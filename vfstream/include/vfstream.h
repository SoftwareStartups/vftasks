#ifndef __VFSTREAM_H
#define __VFSTREAM_H


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
 * Types
 * ***************************************************************************/

/** Represents a FIFO channel. 
 */
typedef struct vfstream_chan_s vfstream_chan_t;

/** Represents a write port on a FIFO channel.
 */
typedef struct vfstream_wport_s vfstream_wport_t;

/** Represents a read port on a FIFO channel.
 */
typedef struct vfstream_rport_s vfstream_rport_t;

/**
 * Represents a FIFO token.
 */
typedef struct vfstream_token_s vfstream_token_t;

/** Called when a writer that is connected to a FIFO channel might want to be
 *  suspended or resumed.
 *
 *  @param  wport  A pointer to the write port through which the reader is connected
 *                 to the channel.
 */
typedef void (*vfstream_writer_hook_t)(vfstream_wport_t *wport);

/** Called when a reader that is connected to a FIFO channel might want to be
 *  suspended or resumed.
 *
 *  @param  rport  A pointer to the read port through which the reader is connected
 *                 to the channel.
 */
typedef void (*vfstream_reader_hook_t)(vfstream_rport_t *rport);

/** Provides a memory-management implementation.
 */
typedef struct vfstream_malloc_s
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
vfstream_malloc_t;


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
vfstream_chan_t *vfstream_create_chan(int num_tokens,
                                  size_t token_size,
                                  vfstream_malloc_t *ctl_space,
                                  vfstream_malloc_t *buf_space);

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
vfstream_wport_t *vfstream_create_write_port(vfstream_chan_t *chan,
                                         vfstream_malloc_t *port_space);

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
vfstream_rport_t *vfstream_create_read_port(vfstream_chan_t *chan,
                                        vfstream_malloc_t *port_space);

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
void vfstream_destroy_chan(vfstream_chan_t *chan,
                         vfstream_malloc_t *ctl_space,
                         vfstream_malloc_t *buf_space);

/** Destroys a given write port.
 *
 *  @param  wport       A pointer to the port.
 *  @param  port_space  A pointer to the memory-management implementation that is to
 *                      be used to deallocate the memory that was allocated for the
 *                      port.
 */
void vfstream_destroy_write_port(vfstream_wport_t *wport,
                               vfstream_malloc_t *port_space);

/** Destroys a given read port.
 *
 *  @param  rport       A pointer to the port.
 *  @param  port_space  A pointer to the memory-management implementation that is to
 *                      be used to deallocate the memory that was allocated for the
 *                      port.
 */
void vfstream_destroy_read_port(vfstream_rport_t *rport,
                              vfstream_malloc_t *port_space);


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
void vfstream_install_chan_hooks(vfstream_chan_t *chan,
                               vfstream_writer_hook_t suspend_writer,
                               vfstream_writer_hook_t resume_writer,
                               vfstream_reader_hook_t suspend_reader,
                               vfstream_reader_hook_t resume_reader);

/* ***************************************************************************
 * Low- and high-water marks
 * ***************************************************************************/

/** Sets the ``low-water mark'' for a given FIFO channel.
 *
 *  The low-water mark signals the number of tokens that triggers writers to the
 *  channel to be prompted to resume their task.
 *  That is, if the number of tokens available for writing becomes equal to or
 *  greater than the low-water mark, writers are notified through a hook that
 *  was installed by vfstream_install_chan_hooks.
 *
 *  @param  chan      A pointer to the channel.
 *  @param  min_room  The new low-water mark.
 *
 *  @return
 *    On success, the new low-water mark.
 *    On failure, the old low-water mark.
 */
int vfstream_set_min_room(vfstream_chan_t *chan, int min_room);

/** Retrieves the ``low-water mark'' for a given FIFO channel.
 *
 *  The low-water mark signals the number of tokens that triggers writers to the
 *  channel to be prompted to resume their task.
 *  That is, if the number of tokens available for writing becomes equal to or
 *  greater than the low-water mark, writers are notified through a hook that
 *  was installed by vfstream_install_chan_hooks.
 *
 *  @param  chan  A pointer to the channel.
 *   
 *  @return
 *    The low-water mark.
 */
int vfstream_get_min_room(vfstream_chan_t *chan);

/** Sets the ``high-water mark'' for a given FIFO channel.
 *
 *  The high-water mark signals the number of tokens that triggers readers from
 *  the channel to be prompted to resume their task.
 *  That is, if the number of tokens available for reading becomes equal to or
 *  greater than the high-water mark, readers are notified through a hook that
 *  was installed by vfstream_install_chan_hooks.
 *
 *  @param  chan      A pointer to the channel.
 *  @param  min_data  The new high-water mark. 
 *   
 *  @return
 *    On success, the new high-water mark.
 *    On failure, the old high-water mark.
 */
int vfstream_set_min_data(vfstream_chan_t *chan, int min_data);

/** Retrieves the ``high-water mark'' for a given FIFO channel.
 *
 *  The high-water mark signals the number of tokens that triggers readers from
 *  the channel to be prompted to resume their task.
 *  That is, if the number of tokens available for reading becomes equal to or
 *  greater than the high-water mark, readers are notified through a hook that
 *  was installed by vfstream_install_chan_hooks.
 *
 *  @param  chan  A pointer to the channel.
 *   
 *  @return
 *    The high-water mark.
 */
int vfstream_get_min_data(vfstream_chan_t *chan);


/* ***************************************************************************
 * Application-specific data
 * ***************************************************************************/

/** Associates application-specific data with a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *  @param  info  A pointer to the application-specific data.
 */
void vfstream_set_chan_info(vfstream_chan_t *chan, void *info);

/** Retrieves application-specific data from a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    A pointer to the application-specific data.
 */
void *vfstream_get_chan_info(vfstream_chan_t *chan);


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
bool_t vfstream_shmem_supported(vfstream_chan_t *chan);

/** Retrieves the number of tokens held by a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    The number of tokens.
 */
int vfstream_get_num_tokens(vfstream_chan_t *chan);

/** Retrieves the size of the tokens held by a given FIFO channel.
 *
 *  @param  chan  A pointer to the channel.
 *
 *  @return
 *    The token size.
 */
size_t vfstream_get_token_size(vfstream_chan_t *chan);


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
vfstream_chan_t *vfstream_chan_of_wport(vfstream_wport_t *wport);

/** Retrieves the FIFO channel that is connected to a given read port.
 *
 *  @param rport  A pointer to the port.
 *
 *  @return
 *    A pointer to the channel.
 */
vfstream_chan_t *vfstream_chan_of_rport(vfstream_rport_t *rport);


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
bool_t vfstream_room_available(vfstream_wport_t *wport);

/** Retrieves whether or not a connected FIFO channel has any tokens available 
 *  for acquisition through a given read port.
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    If the channel has tokens available, true.
 *    If the channel does not have tokens available, false.
 */
bool_t vfstream_data_available(vfstream_rport_t *rport);


/* ***************************************************************************
 * Producer operations
 * ***************************************************************************/

/** Acquires a token from a FIFO channel through a given write port.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *  If there are no tokens available, writers are prompted to be suspended
 *  through a hook that that was installed by vfstream_install_chan_hooks.
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
vfstream_token_t *vfstream_acquire_room(vfstream_wport_t *wport);

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
vfstream_token_t *vfstream_acquire_room_nb(vfstream_wport_t *wport);

/** Releases a token that was previously acquired through a given write port.
 *
 *  If the number of available tokens for acquisition through a read port that
 *  is connected to the channel rises above the ``high-water mark'', readers are
 *  prompted to resume through a hook that was installed by
 *  vfstream_install_chan_hooks.
 *
 *  @param  wport  A pointer to the port.
 *  @param  token  A pointer to the token.
 */
void vfstream_release_data(vfstream_wport_t *wport, vfstream_token_t *token);


/* ***************************************************************************
 * Consumer operations
 * ***************************************************************************/

/** Acquires a token from a FIFO channel through a given read port.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *  If there are no tokens available, readers are prompted to be suspended
 *  through a hook that that was installed by vfstream_install_chan_hooks.
 *  Before actually being suspended, writers should make sure that the
 *  suspension condition still applies as concurrent producer operations may
 *  have made tokens available. 
 *
 *  @param  rport  A pointer to the port.
 *
 *  @return
 *    A pointer to the token.
 */
vfstream_token_t *vfstream_acquire_data(vfstream_rport_t *rport);

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
vfstream_token_t *vfstream_acquire_data_nb(vfstream_rport_t *rport);

/** Releases a token that was previously acquired through a given write port.
 *
 *  If the number of available tokens for acquisition through a write port that
 *  is connected to the channel rises above the ``low-water mark'', writers are
 *  prompted to resume through a hook that was installed by
 *  vfstream_install_chan_hooks.
 *
 *  @param  rport  A pointer to the port.
 *  @param  token  A pointer to the token.
 */
void vfstream_release_room(vfstream_rport_t *rport, vfstream_token_t *token);


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
void *vfstream_get_memaddr(vfstream_token_t *token);


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
void vfstream_put_int8(vfstream_token_t* token, size_t offset, int8_t data);

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
void vfstream_put_int16(vfstream_token_t *token, size_t offset, int16_t data);

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
void vfstream_put_int32(vfstream_token_t *token, size_t offset, int32_t data);

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
void vfstream_put_int64(vfstream_token_t *token, size_t offset, int64_t data);

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
void vfstream_put_float(vfstream_token_t *token, size_t offset, float data);

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
void vfstream_put_double(vfstream_token_t *token, size_t offset, double data);

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
void vfstream_put_ptr(vfstream_token_t *token, size_t offset, void *data);

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
int8_t vfstream_get_int8(vfstream_token_t *token, size_t offset);

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
int16_t vfstream_get_int16(vfstream_token_t *token, size_t offset);

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
int32_t vfstream_get_int32(vfstream_token_t *token, size_t offset);

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
int64_t vfstream_get_int64(vfstream_token_t *token, size_t offset);

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
float vfstream_get_float(vfstream_token_t *token, size_t offset);

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
double vfstream_get_double(vfstream_token_t *token, size_t offset);

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
void *vfstream_get_ptr(vfstream_token_t *token, size_t offset);


/* ***************************************************************************
 * Flushing
 * ***************************************************************************/

/** Prompts readers of a FIFO channel that is connected with a given write port
 *  to resume, regardless of the ``high-water mark'' of the channel.
 *
 *  That is, readers are notified through a hook that was installed by
 *  vfstream_install_chan_hooks.
 *  Typically called by a writer to signal that it has completed its task and
 *  that no more tokens should be expected to become available for reading.
 *
 *  @param  wport  A pointer to the write port.
 */
void vfstream_flush_data(vfstream_wport_t *wport);

/** Prompts writers of a FIFO channel that is connected with a given read port
 *  to resume, regardless of the ``low-water mark'' of the channel.
 *
 *  That is, writers are notified through a hook that was installed by
 *  vfstream_install_chan_hooks.
 *  Typically called by a reader to signal that it has completed its task and
 *  that no more tokens should be expected to become available for writing.
 *
 *  @param  rport  A pointer to the read port.
 */
void vfstream_flush_room(vfstream_rport_t *rport);


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
void vfstream_write_int8(vfstream_wport_t *wport, int8_t data);

/** Writes a 16-bit integer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vfstream_write_int16(vfstream_wport_t *wport, int16_t data);

/** Writes a 32-bit integer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vfstream_write_int32(vfstream_wport_t *wport, int32_t data);

/** Writes a 64-bit integer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vfstream_write_int64(vfstream_wport_t *wport, int64_t data);

/** Writes a single-precision floating-point value through a given write port to
 *  a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vfstream_write_float(vfstream_wport_t *wport, float data);

/** Writes a double-precision floating-point value through a given write port to
 *  a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param wport  A pointer to the port.
 *  @param data   The value.
 */
void vfstream_write_double(vfstream_wport_t *wport, double data);

/** Writes a pointer value through a given write port to a FIFO channel.
 *
 *  Blocks until the channel has tokens available for acquisition through the
 *  port.
 *
 *  @param  wport  A pointer to the port.
 *  @param  data   The value.
 */
void vfstream_write_ptr(vfstream_wport_t *wport, void *data);

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
int8_t vfstream_read_int8(vfstream_rport_t *rport);

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
int16_t vfstream_read_int16(vfstream_rport_t *rport);

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
int32_t vfstream_read_int32(vfstream_rport_t *rport);

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
int64_t vfstream_read_int64(vfstream_rport_t *rport);

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
float vfstream_read_float(vfstream_rport_t *rport);

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
double vfstream_read_double(vfstream_rport_t *rport);

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
void *vfstream_read_ptr(vfstream_rport_t *rport);


#endif /* __VFSTREAM_H */
