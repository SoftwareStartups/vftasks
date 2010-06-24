/* Copyright (c) 2010 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 *
 *
 * Example: straightforward functional partioning implemented with a FIFO channel from
 * vfStream.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <vfstream.h>


#define PROBLEM_SIZE         10000
#define EXPECTED_RESULT      100010000
#define FIFO_DEPTH           6000
#define LOW_WATER_MARK       1
#define HIGH_WATER_MARK      5000
#define DELAY_UNIT           50000
#define DELAY(DELAY_FACTOR)                           \
  {                                                   \
    volatile int i;                                   \
    for (i = 0; i < DELAY_FACTOR * DELAY_UNIT; ++i);  \
  }


/* application-specific data to be asscoiated with FIFO channel
 */
typedef struct info_s
{
  int data[PROBLEM_SIZE];  /* the data                  */
  pthread_mutex_t mutex;   /* mutex                     */
  pthread_cond_t cond;     /* resume-condition variable */
}
info_t;

/* very slow successor
 */
int succ(int n)
{
  DELAY(2);
  return n + 1;
}

/* slow double
 */
int dbl(int n)
{
  DELAY(1);
  return  2 * n;
}

/* sum
 */
int sum(int *data)
{
  unsigned int i;  /* index */
  int n;           /* sum   */

  /* compute sum */
  n = 0;
  for (i = 0; i < PROBLEM_SIZE; ++i) n += data[i];

  /* return sum */
  return n;
}

/* writer routine
 */
void *writer(void *arg)
{
  unsigned int i;         /* index      */
  vfstream_wport_t *wport;  /* write port */
  int *data;              /* data array */

  /* retrieve write port */
  wport = (vfstream_wport_t *)arg;

  /* retrieve data array */
  data = (int *)vfstream_get_chan_info(vfstream_chan_of_wport(wport));

  /* write data */
  for (i = 0; i < PROBLEM_SIZE; ++i) vfstream_write_int32(wport, succ(data[i]));

  /* flush */
  vfstream_flush_data(wport);

  /* exit */
  return NULL;
}

/* called when the writer is prompted to be suspended
 */
void suspend_writer(vfstream_wport_t *wport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vfstream_get_chan_info(vfstream_chan_of_wport(wport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* check whether suspend condition still applies */
  if (!vfstream_room_available(wport))
    /* condition still applies; suspend */
    if (pthread_cond_wait(&info->cond, &info->mutex))
    {
      fprintf(stderr, "writer: waiting for resume condition failed\n");
      pthread_mutex_unlock(&info->mutex);
      vfstream_flush_data(wport);
      pthread_exit(NULL);
    }

  /* unlock */
  pthread_mutex_unlock(&info->mutex);
}

/* called when the writer is prompted to resume
 */
void resume_writer(vfstream_wport_t *wport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vfstream_get_chan_info(vfstream_chan_of_wport(wport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* broadcast resume condition */
  if (pthread_cond_broadcast(&info->cond))
  {
    fprintf(stderr, "writer: broadcasting resume condition failed\n");
    pthread_mutex_unlock(&info->mutex);
    vfstream_flush_data(wport);
    pthread_exit(NULL);
  }

  /* unlock */
  pthread_mutex_unlock(&info->mutex);
}

/* reader routine
 */
void *reader(void *arg)
{
  unsigned int i;         /* index      */
  vfstream_rport_t *rport;  /* read port  */
  int *data;              /* data array */

  /* retrieve read port */
  rport = (vfstream_rport_t *)arg;

  /* retrieve data array */
  data = (int *)vfstream_get_chan_info(vfstream_chan_of_rport(rport));

  /* read data */
  for (i = 0; i < PROBLEM_SIZE; ++i) data[i] = dbl(vfstream_read_int32(rport));

  /* flush */
  vfstream_flush_room(rport);

  /* exit */
  return NULL;
}

/* called when the reader is prompted to be suspended
 */
void suspend_reader(vfstream_rport_t *rport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vfstream_get_chan_info(vfstream_chan_of_rport(rport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* check whether suspend condition still applies */
  if (!vfstream_data_available(rport))
    /* condition still applies; suspend */
    if (pthread_cond_wait(&info->cond, &info->mutex))
    {
      fprintf(stderr, "reader: waiting for resume condition failed\n");
      pthread_mutex_unlock(&info->mutex);
      vfstream_flush_room(rport);
      pthread_exit(NULL);
    }

  /* unlock */
  pthread_mutex_unlock(&info->mutex);
}

/* called when the reader is prompted to resume
 */
void resume_reader(vfstream_rport_t *rport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vfstream_get_chan_info(vfstream_chan_of_rport(rport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* broadcast resume condition */
  if (pthread_cond_broadcast(&info->cond))
  {
    fprintf(stderr, "reader: broadcasting resume condition failed\n");
    pthread_mutex_unlock(&info->mutex);
    vfstream_flush_room(rport);
    pthread_exit(NULL);
  }

  /* unlock */
  pthread_mutex_unlock(&info->mutex);
}

/* entry point
 */
int main()
{
  int rc;                    /* return code         */
  unsigned int i;            /* index               */
  info_t info;               /* application data    */
  vfstream_malloc_t mem_mgr;   /* memory manager      */
  vfstream_chan_t *chan;       /* FIFO channel        */
  vfstream_wport_t *wport;     /* write port          */
  vfstream_rport_t *rport;     /* read port           */
  pthread_t writer_thread;   /* writer thread       */
  pthread_t reader_thread;   /* reader thread       */
  void *trv ;                /* thread return value */
  int result;                /* computed result     */

  /* intialize the data array */
  for (i = 0; i < PROBLEM_SIZE; ++i) info.data[i] = i;

  /* initialize the memory manager */
  mem_mgr.malloc = malloc;
  mem_mgr.free = free;

  /* create channel */
  chan = vfstream_create_chan(FIFO_DEPTH, sizeof(int), &mem_mgr, &mem_mgr);
  if (chan == NULL)
  {
    fprintf(stderr, "channel creation failed\n");
    return 1;
  }

  /* create write port */
  wport = vfstream_create_write_port(chan, &mem_mgr);
  if (wport == NULL)
  {
    fprintf(stderr, "write-port creation failed\n");
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* create read port */
  rport = vfstream_create_read_port(chan, &mem_mgr);
  if (rport == NULL)
  {
    fprintf(stderr, "read-port creation failed\n");
    vfstream_destroy_write_port(wport, &mem_mgr);
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* install channel hooks */
  vfstream_install_chan_hooks(chan,
			    suspend_writer,
			    resume_writer,
			    suspend_reader,
			    resume_reader);

  /* set low- and high-water marks */
  vfstream_set_min_room(chan, LOW_WATER_MARK);
  vfstream_set_min_data(chan, HIGH_WATER_MARK);

  /* initialize mutex */
  rc = pthread_mutex_init(&info.mutex, NULL);
  if (rc)
  {
    fprintf(stderr, "mutex initialization failed\n");
    vfstream_destroy_write_port(wport, &mem_mgr);
    vfstream_destroy_read_port(rport, &mem_mgr);
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* initialize condition variable */
  rc = pthread_cond_init(&info.cond, NULL);
  if (rc)
  {
    fprintf(stderr, "condition-variable initialization failed\n");
    pthread_mutex_destroy(&info.mutex);
    vfstream_destroy_write_port(wport, &mem_mgr);
    vfstream_destroy_read_port(rport, &mem_mgr);
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  } 

  /* associate application data with channel */
  vfstream_set_chan_info(chan, &info);

  /* create and start writer thread */
  rc = pthread_create(&writer_thread, NULL, writer, wport);
  if (rc)
  {
    fprintf(stderr, "writer-thread creation failed\n");
    pthread_cond_destroy(&info.cond);
    pthread_mutex_destroy(&info.mutex);
    vfstream_destroy_write_port(wport, &mem_mgr);
    vfstream_destroy_read_port(rport, &mem_mgr);
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* create and start reader thread */
  rc = pthread_create(&reader_thread, NULL, reader, rport);
  if (rc)
  {
    fprintf(stderr, "reader-thread creation failed\n");
    pthread_cancel(writer_thread);
    pthread_cond_destroy(&info.cond);
    pthread_mutex_destroy(&info.mutex);
    vfstream_destroy_write_port(wport, &mem_mgr);
    vfstream_destroy_read_port(rport, &mem_mgr);
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* join writer thread */
  rc = pthread_join(writer_thread, &trv);
  if (rc)
  {
    fprintf(stderr, "join of writer thread failed\n");
    pthread_cancel(writer_thread);
    pthread_cancel(reader_thread);
    pthread_cond_destroy(&info.cond);
    pthread_mutex_destroy(&info.mutex);
    vfstream_destroy_write_port(wport, &mem_mgr);
    vfstream_destroy_read_port(rport, &mem_mgr);
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* join reader thread */
  rc = pthread_join(reader_thread, &trv);
  if (rc)
  {
    fprintf(stderr, "join of reader thread failed\n");
    pthread_cancel(reader_thread);
    pthread_cond_destroy(&info.cond);
    pthread_mutex_destroy(&info.mutex);
    vfstream_destroy_write_port(wport, &mem_mgr);
    vfstream_destroy_read_port(rport, &mem_mgr);
    vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* destroy condition variable, mutex, ports, and channel */
  pthread_cond_destroy(&info.cond);
  pthread_mutex_destroy(&info.mutex);
  vfstream_destroy_write_port(wport, &mem_mgr);
  vfstream_destroy_read_port(rport, &mem_mgr);
  vfstream_destroy_chan(chan, &mem_mgr, &mem_mgr);

  /* compute the result */
  result = sum(info.data);

  /* check the result */
  if (result == EXPECTED_RESULT)
  {
    printf("PASSED: %d\n", result);
    return 0;
  }
  else
  {
    fprintf(stderr, "FAILED: %d\n", result);
    return 1;
  }
}
