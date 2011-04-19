/* Copyright (c) 2010-2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

/** \mainpage vfStream library API documentation
 *
 * \section intro_sec Introduction
 *  A FIFO channel consists of a circular buffer that can hold N tokens plus one
 *  void token.
 *  The channel state includes a tail and a head pointer.
 *  If tail == head, then the channel is empty.
 *  The token just before head is always void (i.e., unused) so tail can never
 *  bump into head.
 *  The limit parameter points to the first address beyond the buffer space,
 *  and the base parameter is the first address of the buffer.
 *  The head pointer is advanced by vftasks_release_room.
 *  The tail pointer is advanced by vftasks_release_data.
 *  @verbatim
 *              base              head         tail        limit
 *               |                  |            |           |
 *               |                  |            |           |
 *               V                  V            V           V
 *               +--------------+---+------------+-----------+
 *               |              | v |            |           |
 *               |    room      | o |    data    |   room    |
 *               |              | i |            |           |
 *               |              | d |            |           |
 *               +--------------+---+------------+-----------+
 *                                        ^           ^
 *                                        |           |
 *                                        |           |
 *                                      data        room
 *  @endverbatim
 *  Normally two ports are connected to the channel.
 *  A write port maintains a room pointer and a read port maintains a data
 *  pointer.
 *  The data and room pointers are NOT part of the channel state.
 *  The data pointer is advanced by vftasks_acquire_data.
 *  The room pointer is advanced by vftasks_acquire_room.
 *  It is possible to acquire multiple data tokens before releasing room.
 *  Calling vftasks_release_room releases the oldest acquired data token.
 *  It is possible to acquire multiple room tokens before releasing new data.
 *  Calling vftasks_release_data releases the oldest acquired room token.
 */

#include "vftasks.h"

#include <stdlib.h>   /* for malloc, free, and abort  */
#include <stdio.h>    /* for printing to stderr */
#include <string.h>   /* for memcpy */


#ifndef __GNUC__            /* not all compilers recognize __inline__ */
#define __inline__
#endif


#define MAX(X,Y)  (((X) > (Y)) ? (X) : (Y))

/* ***************************************************************************
 * Types
 * ***************************************************************************/

typedef struct vftasks_state_s vftasks_state_t;
typedef struct vftasks_param_s vftasks_param_t;

/** channel state
 */
struct vftasks_state_s
{
  vftasks_token_t *head;  /** points to oldest data token in the FIFO  */
  vftasks_token_t *tail;  /** points to next available room token      */
};

/** channel parameters
 */
struct vftasks_param_s
{
  int min_data;           /** resume reader when #data tokens >= min_data */
  int min_room;           /** resume writer when #room tokens >= min_room */
  vftasks_token_t *limit; /** points to first byte beyond token buffer    */
  vftasks_token_t *base;  /** points to start of token buffer             */
};

/**
 * tokens
 */
struct vftasks_token_s
{
  size_t token_size;  /** size of space allocated for the token (power of 2) */
  char *token_base;   /** points to the first byte allocated for the token   */
};

/** channel
 */
struct vftasks_chan_s
{
  vftasks_state_t state;                 /** channel state                     */
  vftasks_param_t param;                 /** channel parameters                */
  vftasks_rport_t *rport;                /** points to read port connected to
                                             this channel                      */
  vftasks_wport_t *wport;                /** points to write port connected to
                                             this channel                      */
  void *info;                            /** points to additional,
                                             application-specific data         */
  vftasks_writer_hook_t suspend_writer;  /** suspend-writer hook               */
  vftasks_writer_hook_t resume_writer;   /** resume-writer hook                */
  vftasks_reader_hook_t suspend_reader;  /** suspend-reader hook               */
  vftasks_reader_hook_t resume_reader;   /** resume-reader hook                */
};

/** write port
 */
struct vftasks_wport_s
{
  vftasks_token_t *room;               /** points beyond tail, but not further
                                           than head                           */
  vftasks_state_t cached_state;        /** cached channel state                */
  vftasks_param_t param;               /** channel parameters                  */
  vftasks_token_t *wakeup_zone_start;  /** start of wake-up zone: wake reader
                                           if chan->tail points into wake-up
                                           zone                                */
  vftasks_token_t *wakeup_zone_end;    /** end of wake-up zone                 */
  vftasks_chan_t *chan;                /** points to channel                   */
};

/** read port
 */
struct vftasks_rport_s
{
  vftasks_token_t *data;               /** points beyond head, but not further
                                           than tail                           */
  vftasks_state_t cached_state;        /** cached channel state                */
  vftasks_param_t param;               /** channel parameters                  */
  vftasks_token_t *wakeup_zone_start;  /** start of wake-up zone: wake writer
                                           if chan->tail points into wake-up
                                           zone                                */
  vftasks_token_t *wakeup_zone_end;    /** end of wake-up zone                 */
  vftasks_chan_t *chan;                /** points to channel                   */
};


/* ***************************************************************************
 * Creation and destruction of channels and ports
 * ***************************************************************************/

#ifndef VFPOLLING

/** default writer hook
 */
static void vftasks_default_writer_hook(vftasks_wport_t *wport)
{
  /* do nothing */
}

/** default reader hook
 */
static void vftasks_default_reader_hook(vftasks_rport_t *rport)
{
  /* do nothing */
}

/** create channel
 */
vftasks_chan_t *vftasks_create_chan(int num_tokens,
                                    size_t token_size,
                                    vftasks_malloc_t *ctl_space,
                                    vftasks_malloc_t *buf_space)
{
  int chan_size;              /* channel size (== #tokens + 1) */
  size_t overflow_size;       /* overflow size                 */
  char *raw_buf;              /* the raw buffer                */
  vftasks_token_t *token_buf; /* the token buffer              */
  vftasks_chan_t *chan;       /* pointer to the channel        */

  /* check #tokens and token size */
  if (num_tokens <= 0)
  {
    return NULL;
  }

  if (token_size <= 0)
  {
    return NULL;
  }

  /* if necessary, round up the token size to the next power of 2 */
  if ((token_size & (token_size - 1)) != 0)
  {
    int i;
    for (i = 0; token_size > 0; ++i) token_size >>= 1;
    token_size = 1 << i;
  }

  /* determine channel size */
  chan_size = num_tokens + 1;

  /* determinze overflow size */
  overflow_size = MAX(sizeof(int8_t),
                      MAX(sizeof(int16_t),
                          MAX(sizeof(int32_t),
                              MAX(sizeof(int64_t),
                                  MAX(sizeof(float),
                                      MAX(sizeof(double), sizeof(void *)))))));

  /* allocate raw buffer */
  raw_buf = buf_space->malloc(chan_size * (token_size + overflow_size));
  if (raw_buf == NULL)
  {
    return NULL;
  }

  /* allocate token buffer */
  token_buf = ctl_space->malloc(chan_size * sizeof(vftasks_token_t));
  if (token_buf == NULL)
  {
    return NULL;
  }

  /* fill token buffer */
  {
    char* raw_ix;              /* index into raw buffer   */
    vftasks_token_t* token_ix; /* index into token buffer */

    /* iterate through buffers */
    raw_ix = raw_buf;
    for (token_ix = token_buf; token_ix < token_buf + chan_size; ++token_ix)
    {
      token_ix->token_size = token_size;
      token_ix->token_base = raw_ix;
      raw_ix += (token_size + overflow_size);
    }
  }

  /* allocate channel */
  chan = ctl_space->malloc(sizeof(vftasks_chan_t));
  if (chan == NULL)
  {
    buf_space->free(raw_buf);
    ctl_space->free(token_buf);
    return NULL;
  }

  /* set parameters */
  chan->param.base = token_buf;
  chan->param.limit = token_buf + num_tokens + 1;
  chan->param.min_data = 1;
  chan->param.min_room = 1;

  /* initialize state */
  chan->state.tail = chan->param.base;
  chan->state.head = chan->param.base;
  chan->rport = NULL;
  chan->wport = NULL;

  /* initialize application-specific data */
  chan->info = NULL;

  /* install default hooks */
  chan->suspend_writer = vftasks_default_writer_hook;
  chan->resume_writer = vftasks_default_writer_hook;
  chan->suspend_reader = vftasks_default_reader_hook;
  chan->resume_reader = vftasks_default_reader_hook;

  /* return channel pointer */
  return chan;
}

/** create write port
 */
vftasks_wport_t *vftasks_create_write_port(vftasks_chan_t *chan,
                                           vftasks_malloc_t *port_space)
{
  vftasks_wport_t *wport;  /* pointer to the port */

  /* check if channel already has a write port */
  if (chan->wport != NULL)
  {
    return NULL;
  }

  /* allocate port */
  wport = port_space->malloc(sizeof(vftasks_wport_t));
  if (wport == NULL)
  {
    return NULL;
  }

  /* initialize port */
  memcpy(&wport->cached_state, &chan->state, sizeof(vftasks_state_t));
  memcpy(&wport->param, &chan->param, sizeof(vftasks_param_t));
  wport->room = wport->cached_state.tail;
  wport->wakeup_zone_start = NULL;
  wport->wakeup_zone_end = NULL;

  /* connect port to channel */
  wport->chan = chan;
  chan->wport = wport;

  /* return port pointer */
  return wport;
}

/** create read port
 */
vftasks_rport_t *vftasks_create_read_port(vftasks_chan_t *chan,
                                          vftasks_malloc_t *port_space)
{
  vftasks_rport_t *rport;  /* pointer to the port */

  /* check if channel already has a read port */
  if (chan->rport != NULL)
  {
    return NULL;
  }

  /* allocate the port */
  rport = port_space->malloc(sizeof(vftasks_rport_t));
  if (rport == NULL)
  {
    return NULL;
  }

  /* initialize port */
  memcpy(&rport->cached_state, &chan->state, sizeof(vftasks_state_t));
  memcpy(&rport->param, &chan->param, sizeof(vftasks_param_t));
  rport->data = rport->cached_state.head;
  rport->wakeup_zone_start = NULL;
  rport->wakeup_zone_end = NULL;

  /* connect port to channel */
  rport->chan = chan;
  chan->rport = rport;

  /* return port pointer */
  return rport;
}

/** destroy channel
 */
void vftasks_destroy_chan(vftasks_chan_t *chan,
                          vftasks_malloc_t *ctl_space,
                          vftasks_malloc_t *buf_space)
{
  /* deallocate raw buffer */
  buf_space->free(chan->param.base->token_base);

  /* deallocate token buffer */
  ctl_space->free(chan->param.base);

  /* deallocate channel */
  ctl_space->free(chan);
}

/** destroy write port
 */
void vftasks_destroy_write_port(vftasks_wport_t *wport,
                                vftasks_malloc_t *port_space)
{
  /* disconnect port from channel */
  wport->chan->wport = NULL;

  /* deallocate the port */
  port_space->free(wport);
}

/** destroy read port
 */
void vftasks_destroy_read_port(vftasks_rport_t *rport,
                               vftasks_malloc_t *port_space)
{
  /* disconnect port from channel */
  rport->chan->rport = NULL;

  /* deallocate the port */
  port_space->free(rport);
}

#endif /* VFPOLLING */

/* ***************************************************************************
 * Channel hooks
 * ***************************************************************************/

/** install hooks
 */
void vftasks_install_chan_hooks(vftasks_chan_t *chan,
                                vftasks_writer_hook_t suspend_writer,
                                vftasks_writer_hook_t resume_writer,
                                vftasks_reader_hook_t suspend_reader,
                                vftasks_reader_hook_t resume_reader)
{
  /* update hooks */
  chan->suspend_writer = suspend_writer;
  chan->resume_writer = resume_writer;
  chan->suspend_reader = suspend_reader;
  chan->resume_reader = resume_reader;
}

/* ***************************************************************************
 * Low- and high-water marks
 * ***************************************************************************/

/** set low-water mark
 */
int vftasks_set_min_room(vftasks_chan_t *chan, int min_room)
{
  vftasks_param_t *param;  /* pointer to the channel parameters */
  int chan_size;           /* channel size (== #tokens + 1)     */

  /* retrieve parameter pointer */
  param = &chan->param;

  /* retrieve the channel size */
  chan_size = param->limit - param->base;

  /* check if the new mark is within bounds;
     otherwise, do not update the mark */
  if (min_room <= 0 || min_room >= chan_size)
  {
    return param->min_room;
  }

  /* update the mark */
  param->min_room = min_room;

  /* update copies held by ports */
  if (chan->rport != NULL) chan->rport->param.min_room = min_room;
  if (chan->wport != NULL) chan->wport->param.min_room = min_room;

  /* return the new mark */
  return min_room;
}

/** get low-water mark
 */
int vftasks_get_min_room(vftasks_chan_t *chan)
{
  /* return mark */
  return chan->param.min_room;
}

/** set high-water mark
 */
int vftasks_set_min_data(vftasks_chan_t *chan, int min_data)
{
  vftasks_param_t *param; /* pointer to the channel parameters */
  int chan_size;          /* channel size (== #tokens + 1)     */

  /* retrieve parameter pointer */
  param = &chan->param;

  /* retrieve the channel size */
  chan_size = param->limit - param->base;

  /* check if the new mark is within bounds;
     otherwise, do not update the mark */
  if (min_data <= 0 || min_data >= chan_size)
  {
    return param->min_data;
  }

  /* update the mark */
  param->min_data = min_data;

  /* update copies held by ports */
  if (chan->rport != NULL) chan->rport->param.min_data = min_data;
  if (chan->wport != NULL) chan->wport->param.min_data = min_data;

  /* return the new mark */
  return min_data;
}

/** get high-water mark
 */
int vftasks_get_min_data(vftasks_chan_t *chan)
{
  /* return mark */
  return chan->param.min_data;
}

/* ***************************************************************************
 * Application-specific data
 * ***************************************************************************/

/** set info
 */
void vftasks_set_chan_info(vftasks_chan_t *chan, void *info)
{
  /* update info */
  chan->info = info;
}

/** get info
 */
void *vftasks_get_chan_info(vftasks_chan_t *chan)
{
  /* return info */
  return chan->info;
}

/* ***************************************************************************
 * Channel queries
 * ***************************************************************************/

/** check for shared-memory suppert
 */
bool_t vftasks_shmem_supported(vftasks_chan_t *chan)
{
  return true;
}

/** get #tokens
 */
int vftasks_get_num_tokens(vftasks_chan_t *chan)
{
  int chan_size;  /* channel size (== #tokens + 1) */

  /* retrieve the channel size */
  chan_size = chan->param.limit - chan->param.base;

  /* return #tokens */
  return chan_size - 1;
}

/** get token size
 */
size_t vftasks_get_token_size(vftasks_chan_t *chan)
{
  return chan->param.base->token_size;
}

/* ***************************************************************************
 * Port queries
 * ***************************************************************************/

/** get channel of write port
 */
vftasks_chan_t *vftasks_chan_of_wport(vftasks_wport_t *wport)
{
  /* return channel */
  return wport->chan;
}

/** get channel of read port
 */
vftasks_chan_t *vftasks_chan_of_rport(vftasks_rport_t *rport)
{
  /* return channel */
  return rport->chan;
}

/* ***************************************************************************
 * Channel-state queries
 * ***************************************************************************/

/** check for available room
 */
bool_t vftasks_room_available(vftasks_wport_t *wport)
{
  vftasks_token_t *new_room;  /* new room pointer, if room would be acquired now */

  /* determine new room pointer; if it is out of bounds, wrap it */
  new_room = wport->room + 1;
  if (new_room == wport->param.limit) new_room = wport->param.base;

  /* check whether buffer is full */
  if (new_room == wport->cached_state.head)
  {
    /* update cache and recheck with updated cache */
    wport->cached_state.head = wport->chan->state.head;
    if (new_room == wport->cached_state.head) return false;
  }

  /* buffer still has room */
  return true;
}

/** check for available data
 */
bool_t vftasks_data_available(vftasks_rport_t *rport)
{
  vftasks_token_t* data;  /* current data pointer */

  /* retrieve data pointer */
  data = rport->data;

  /* check whether buffer is empty */
  if (data == rport->cached_state.tail)
  {
    /* update cache and recheck with updated cache */
    rport->cached_state.tail = rport->chan->state.tail;
    if (data == rport->cached_state.tail) return false;
  }

  /* buffer contains data */
  return true;
}

/* ***************************************************************************
 * Producer operations
 * ***************************************************************************/

/** acquire room (nonblocking)
 */
__inline__ vftasks_token_t *vftasks_acquire_room_nb(vftasks_wport_t *wport)
{
  vftasks_token_t *room;      /* the current room pointer */
  vftasks_token_t *new_room;  /* the new room pointer     */

  /* retrieve the current room pointer */
  room = wport->room;

  /* determine new room pointer; if it is out of bounds, wrap it */
  new_room = room + 1;
  if (new_room == wport->param.limit) new_room = wport->param.base;

  /* check whether buffer is full */
  if (new_room == wport->cached_state.head)
  {
    /* update cache and recheck with updated cache */
    wport->cached_state.head = wport->chan->state.head;
    if (new_room == wport->cached_state.head)
    {
      return NULL;
    }
  }

  /* buffer still has room; update room pointer */
  wport->room = new_room;

  /* return pointer to acquired token */
  return room;
}

/** acquire room
 */
__inline__ vftasks_token_t *vftasks_acquire_room(vftasks_wport_t *wport)
{
  vftasks_token_t *token;    /* pointer to acquired token */

  /* try to acquire a token, until we have one */
  do
  {
    /* try to acquire a token */
    token = vftasks_acquire_room_nb(wport);
    if (token != NULL) return token;

#ifndef VFPOLLING
    /* acquisition failed; put writer to sleep */
    {
      vftasks_chan_t *chan;          /* pointer to the channel                */
      vftasks_token_t *room;         /* room pointer                          */
      vftasks_token_t *wakeup_mark;  /* writer resumes when the head hits the
                                        wake-up mark                          */
      int wrap;                      /* overflow                              */

      /* retrieve the channel and room pointers */
      chan = wport->chan;
      room = wport->room;

      /* compute the wake-up mark (the extra ``1'' accounts for the void token
         just before the head */
      wakeup_mark = room + (1 + wport->param.min_room);

      /* compute the overflow; if it is nonnegative, wrap the wake-up mark */
      wrap = wakeup_mark - wport->param.limit;
      if (wrap >= 0) wakeup_mark = wport->param.base + wrap;

      /* set wake-up zone on the read port */
      chan->rport->wakeup_zone_start = wakeup_mark;
      chan->rport->wakeup_zone_end = room + 1;

      /* suspend writer */
      (*chan->suspend_writer)(wport);

      /* clear wake-up zone */
      chan->rport->wakeup_zone_start = NULL;
      chan->rport->wakeup_zone_end = NULL;
    }
#endif /* VFPOLLING */
  }
  while (true);
}

/** release data
 */
__inline__ void vftasks_release_data(vftasks_wport_t *wport,
                                    vftasks_token_t *token)
{
  vftasks_chan_t *chan;       /* pointer to the channel   */
  vftasks_token_t* tail;      /* the current tail pointer */
  vftasks_token_t* new_tail;  /* the new tail pointer     */

  /* retrieve the channel */
  chan = wport->chan;

  /* retrieve the tail pointer */
  tail = wport->cached_state.tail;

  /* determine new tail pointer; if it is out of bounds, wrap it */
  new_tail = tail + 1;
  if (new_tail == wport->param.limit) new_tail = wport->param.base;

  /* update tail pointer */
  wport->cached_state.tail = new_tail;
  chan->state.tail = new_tail;

#ifndef VFPOLLING
  /* if necessary, resume reader */
  {
    vftasks_token_t *wakeup_zone_start;  /* start of wake-up zone */


    /* retrieve start of wake-up zone */
    wakeup_zone_start = wport->wakeup_zone_start;

    /* check wake-up zone */
    if (wakeup_zone_start != NULL)
    {
      vftasks_token_t *wakeup_zone_end;    /* end of wake-up zone   */

      /* retrieve end of wake-up zone */
      wakeup_zone_end = wport->wakeup_zone_end;

      /* check if tail points into wake-up zone */
      if ((wakeup_zone_start <= wakeup_zone_end &&
           (new_tail >= wakeup_zone_start && new_tail < wakeup_zone_end)) ||
          (wakeup_zone_start > wakeup_zone_end &&
           (new_tail >= wakeup_zone_start || new_tail < wakeup_zone_end)))
        /* tail points into wake-up zone: resume reader */
        (*chan->resume_reader)(chan->rport);
    }
  }
#endif
}

/* ***************************************************************************
 * Producer operations
 * ***************************************************************************/

/** acquire data (nonblocking)
 */
__inline__ vftasks_token_t *vftasks_acquire_data_nb(vftasks_rport_t *rport)
{
  vftasks_token_t *data;      /* the current data pointer */
  vftasks_token_t *new_data;  /* the new data pointer     */

  /* retrieve the current data pointer */
  data = rport->data;

  /* check whether buffer is empty */
  if (data == rport->cached_state.tail)
  {
    /* update cache and recheck with updated cache */
    rport->cached_state.tail = rport->chan->state.tail;
    if (data == rport->cached_state.tail)
    {
      return NULL;
    }
  }

  /* determine new data pointer; if it is out of bounds, wrap it */
  new_data = data + 1;
  if (new_data == rport->param.limit) new_data = rport->param.base;

  /* buffer has data: update data pointer */
  rport->data = new_data;

  /* return pointer to acquired token */
  return data;
}

/** acquire data
 */
__inline__ vftasks_token_t *vftasks_acquire_data(vftasks_rport_t *rport)
{
  vftasks_token_t *token;    /* pointer to the acquired token */

  /* try to acquire a token, until we have one */
  do
  {
    /* try to acquire a token */
    token = vftasks_acquire_data_nb(rport);
    if (token != NULL) return token;

#ifndef VFPOLLING
    /* acquisition failed; put reader to sleep */
    {
      vftasks_chan_t *chan;          /* pointer to the channel                */
      vftasks_token_t *data;         /* data pointer                          */
      vftasks_token_t *wakeup_mark;  /* reader resumes when the tail hits the
                                        wake-up mark                          */
      int wrap;                      /* overflow                              */

      /* retrieve the channel and data pointers */
      chan = rport->chan;
      data = rport->data;

      /* compute the wake-up mark */
      wakeup_mark = data + rport->param.min_data;

      /* compute the overflow; if it is nonnegative, wrap the wake-up mark */
      wrap = wakeup_mark - rport->param.limit;
      if (wrap >= 0) wakeup_mark = rport->param.base + wrap;

      /* set wake-up zone on the write port */
      chan->wport->wakeup_zone_start = wakeup_mark;
      chan->wport->wakeup_zone_end = data;

      /* suspend reader */
      (*chan->suspend_reader)(rport);

      /* clear wake-up zone */
      chan->wport->wakeup_zone_start = NULL;
      chan->wport->wakeup_zone_end = NULL;
    }
#endif /* VFPOLLING */
  }
  while (true);
}

/** release room
 */
__inline__ void vftasks_release_room(vftasks_rport_t *rport,
                                     vftasks_token_t *token)
{
  vftasks_chan_t *chan;       /* pointer to the channel   */
  vftasks_token_t* head;      /* the current head pointer */
  vftasks_token_t* new_head;  /* the new head pointer     */

  /* retrieve the channel */
  chan = rport->chan;

  /* retrieve the head pointer */
  head = rport->cached_state.head;

  /* determine new head pointer; if it is out of bounds, wrap it */
  new_head = head + 1;
  if (new_head == rport->param.limit) new_head = rport->param.base;

  /* update head pointer */
  rport->cached_state.head = new_head;
  chan->state.head = new_head;

#ifndef VFPOLLING
  /* if necessary, resume writer */
  {
    vftasks_token_t *wakeup_zone_start;  /* start of wake-up zone */

    /* retrieve start of wake-up zone */
    wakeup_zone_start = rport->wakeup_zone_start;

    /* check wake-up zone */
    if (wakeup_zone_start != NULL)
    {
      vftasks_token_t *wakeup_zone_end;  /* end of wake-up zone */

      /* retrieve end of wake-up zone */
      wakeup_zone_end = chan->rport->wakeup_zone_end;

      /* check if head points into wake-up zone */
      if ((wakeup_zone_start <= wakeup_zone_end &&
           (new_head >= wakeup_zone_start && new_head < wakeup_zone_end)) ||
          (wakeup_zone_start > wakeup_zone_end &&
           (new_head >= wakeup_zone_start || new_head < wakeup_zone_end)))
        /* tail points into wake-up zone: resume reader */
        (*chan->resume_writer)(chan->wport);
    }
  }
#endif
}

/* ***************************************************************************
 * Shared-memory-mode operations
 * ***************************************************************************/

/** get memory address for token
 */
void *vftasks_get_memaddr(vftasks_token_t *token)
{
  return token->token_base;
}

/* ***************************************************************************
 * Low-level operations
 * ***************************************************************************/

/** put data
 */
#define VFTASKS_PUT(TYPE, TOKEN, OFFSET, DATA)   \
  /* wrap offset */                              \
  OFFSET &= TOKEN->token_size - 1;               \
                                                 \
  /* put data */                                 \
  *(TYPE *)(TOKEN->token_base + OFFSET) = DATA

/** get data
 */
#define VFTASKS_GET(TYPE, TOKEN, OFFSET)        \
  /* wrap offset */                             \
  OFFSET &= TOKEN->token_size - 1;              \
                                                \
  /* return data */                             \
  return *(TYPE *)(TOKEN->token_base + OFFSET)

/** put 8-bit int
 */
void vftasks_put_int8(vftasks_token_t *token, size_t offset, int8_t data)
{
  VFTASKS_PUT(int8_t, token, offset, data);
}

/** put 16-bit int
 */
void vftasks_put_int16(vftasks_token_t *token, size_t offset, int16_t data)
{
  VFTASKS_PUT(int16_t, token, offset, data);
}

/** put 32-bit int
 */
void vftasks_put_int32(vftasks_token_t *token, size_t offset, int32_t data)
{
  VFTASKS_PUT(int32_t, token, offset, data);
}

/** put 64-bit int
 */
void vftasks_put_int64(vftasks_token_t *token, size_t offset, int64_t data)
{
  VFTASKS_PUT(int64_t, token, offset, data);
}

/** put float
 */
void vftasks_put_float(vftasks_token_t *token, size_t offset, float data)
{
  VFTASKS_PUT(float, token, offset, data);
}

/** put double
 */
void vftasks_put_double(vftasks_token_t *token, size_t offset, double data)
{
  VFTASKS_PUT(double, token, offset, data);
}

/** put pointer
 */
void vftasks_put_ptr(vftasks_token_t *token, size_t offset, void *data)
{
  VFTASKS_PUT(void *, token, offset, data);
}

/** get 8-bit int
 */
int8_t vftasks_get_int8(vftasks_token_t *token, size_t offset)
{
  VFTASKS_GET(int8_t, token, offset);
}

/** get 16-bit int
 */
int16_t vftasks_get_int16(vftasks_token_t *token, size_t offset)
{
  VFTASKS_GET(int16_t, token, offset);
}

/** get 32-bit int
 */
int32_t vftasks_get_int32(vftasks_token_t *token, size_t offset)
{
  VFTASKS_GET(int32_t, token, offset);
}

/** get 64-bit int
 */
int64_t vftasks_get_int64(vftasks_token_t *token, size_t offset)
{
  VFTASKS_GET(int64_t, token, offset);
}

/** get float
 */
float vftasks_get_float(vftasks_token_t *token, size_t offset)
{
  VFTASKS_GET(float, token, offset);
}

/** get double
 */
double vftasks_get_double(vftasks_token_t *token, size_t offset)
{
  VFTASKS_GET(double, token, offset);
}

/** get pointer
 */
void *vftasks_get_ptr(vftasks_token_t *token, size_t offset)
{
  VFTASKS_GET(void *, token, offset);
}

/* ***************************************************************************
 * Flushing
 * ***************************************************************************/

/** flush data
 */
void vftasks_flush_data(vftasks_wport_t *wport)
{
#ifndef VFPOLLING
  vftasks_token_t *wakeup_zone_start;  /* start of wake-up zone */

  /* retrieve start of wake-up zone */
  wakeup_zone_start = wport->wakeup_zone_start;

  /* check wake-up zone */
  if (wakeup_zone_start != NULL)
  {
      vftasks_chan_t *chan;  /* pointer to the channel */

      /* retrieve channel pointer */
      chan = wport->chan;

      /* resume reader */
      (*chan->resume_reader)(chan->rport);
  }
#endif /* VFPOLLING */
}

/** flush room
 */
void vftasks_flush_room(vftasks_rport_t *rport)
{
#ifndef VFPOLLING
  vftasks_token_t *wakeup_zone_start;  /* start of wake-up zone */

  /* retrieve start of wake-up zone */
  wakeup_zone_start = rport->wakeup_zone_start;

  /* check wake-up zone */
  if (wakeup_zone_start != NULL)
  {
      vftasks_chan_t *chan;  /* pointer to the channel */

      /* retrieve channel pointer */
      chan = rport->chan;

      /* resume writer */
      (*chan->resume_writer)(chan->wport);
  }
#endif /* VFPOLLING */
}

/* ***************************************************************************
 * High-level, Kahn-style operations
 * ***************************************************************************/

/** write 8-bit int
 */
void vftasks_write_int8(vftasks_wport_t *wport, int8_t data)
{
  vftasks_token_t *token;  /* pointer to room token */

  /* write data */
  token = vftasks_acquire_room(wport);
  vftasks_put_int8(token, 0, data);
  vftasks_release_data(wport, token);
}

/** write 16-bit int
 */
void vftasks_write_int16(vftasks_wport_t *wport, int16_t data)
{
  vftasks_token_t *token;  /* pointer to room token */

  /* write data */
  token = vftasks_acquire_room(wport);
  vftasks_put_int16(token, 0, data);
  vftasks_release_data(wport, token);
}

/** write 32-bit int
 */
void vftasks_write_int32(vftasks_wport_t *wport, int32_t data)
{
  vftasks_token_t *token;  /* pointer to room token */

  /* write data */
  token = vftasks_acquire_room(wport);
  vftasks_put_int32(token, 0, data);
  vftasks_release_data(wport, token);
}

/** write 64-bit int
 */
void vftasks_write_int64(vftasks_wport_t *wport, int64_t data)
{
  vftasks_token_t *token;  /* pointer to room token */

  /* write data */
  token = vftasks_acquire_room(wport);
  vftasks_put_int64(token, 0, data);
  vftasks_release_data(wport, token);
}

/** write float
 */
void vftasks_write_float(vftasks_wport_t *wport, float data)
{
  vftasks_token_t *token;  /* pointer to room token */

  /* write data */
  token = vftasks_acquire_room(wport);
  vftasks_put_float(token, 0, data);
  vftasks_release_data(wport, token);
}

/** write double
 */
void vftasks_write_double(vftasks_wport_t *wport, double data)
{
  vftasks_token_t *token;  /* pointer to room token */

  /* write data */
  token = vftasks_acquire_room(wport);
  vftasks_put_double(token, 0, data);
  vftasks_release_data(wport, token);
}

/** write pointer
 */
void vftasks_write_ptr(vftasks_wport_t *wport, void *data)
{
  vftasks_token_t *token;  /** pointer to room token */

  /** write data */
  token = vftasks_acquire_room(wport);
  vftasks_put_ptr(token, 0, data);
  vftasks_release_data(wport, token);
}

/** read 8-bit int
 */
int8_t vftasks_read_int8(vftasks_rport_t *rport)
{
  vftasks_token_t *token; /* pointer to data token */
  int8_t data;            /* the data   */

  /* read data */
  token = vftasks_acquire_data(rport);
  data = vftasks_get_int8(token, 0);
  vftasks_release_room(rport, token);

  /* return data */
  return data;
}

/** read 16-bit int
 */
int16_t vftasks_read_int16(vftasks_rport_t *rport)
{
  vftasks_token_t* token; /* pointer to data token */
  int16_t data;           /* the data   */

  /* read data */
  token = vftasks_acquire_data(rport);
  data = vftasks_get_int16(token, 0);
  vftasks_release_room(rport, token);

  /* return data */
  return data;
}

/** read 32-bit int
 */
int32_t vftasks_read_int32(vftasks_rport_t *rport)
{
  vftasks_token_t *token; /* pointer to data token */
  int32_t data;           /* the data   */

  /* read data */
  token = vftasks_acquire_data(rport);
  data = vftasks_get_int32(token, 0);
  vftasks_release_room(rport, token);

  /* return data */
  return data;
}

/** read 64-bit int
 */
int64_t vftasks_read_int64(vftasks_rport_t *rport)
{
  vftasks_token_t *token; /* pointer to data token */
  int64_t data;           /* the data   */

  /* read data */
  token = vftasks_acquire_data(rport);
  data = vftasks_get_int64(token, 0);
  vftasks_release_room(rport, token);

  /* return data */
  return data;
}

/** read float
 */
float vftasks_read_float(vftasks_rport_t *rport)
{
  vftasks_token_t *token; /* pointer to data token */
  float data;             /* the data   */

  /* read data */
  token = vftasks_acquire_data(rport);
  data = vftasks_get_float(token, 0);
  vftasks_release_room(rport, token);

  /* return data */
  return data;
}

/** read double
 */
double vftasks_read_double(vftasks_rport_t *rport)
{
  vftasks_token_t *token; /* pointer to data token */
  double data;            /* the data   */

  /* read data */
  token = vftasks_acquire_data(rport);
  data = vftasks_get_double(token, 0);
  vftasks_release_room(rport, token);

  /* return data */
  return data;
}

/** read pointer
 */
void *vftasks_read_ptr(vftasks_rport_t *rport)
{
  vftasks_token_t *token; /* pointer to data token */
  void* data;             /* the data   */

  /* read data */
  token = vftasks_acquire_data(rport);
  data = vftasks_get_ptr(token, 0);
  vftasks_release_room(rport, token);

  /* return data */
  return data;
}
