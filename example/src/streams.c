/* Example: straightforward functional partioning implemented with a FIFO channel.
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <vftasks.h>


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


/* application-specific data to be asscociated with FIFO channel
 */
typedef struct info_s
{
  pthread_mutex_t mutex;   /* mutex                     */
  pthread_cond_t cond;     /* resume-condition variable */
}
info_t;

/* Data local to the writer, but initialized by main() */
int writer_data[PROBLEM_SIZE];

/* Data communicated between the reader and main() */
int reader_data[PROBLEM_SIZE];

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
  vftasks_wport_t *wport;  /* write port */

  /* retrieve write port */
  wport = (vftasks_wport_t *)arg;

  /* write data */
  for (i = 0; i < PROBLEM_SIZE; ++i) vftasks_write_int32(wport, succ(writer_data[i]));

  /* flush */
  vftasks_flush_data(wport);

  /* exit */
  return NULL;
}

/* called when the writer is prompted to be suspended
 */
void suspend_writer(vftasks_wport_t *wport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vftasks_get_chan_info(vftasks_chan_of_wport(wport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* check whether suspend condition still applies */
  if (!vftasks_room_available(wport))
    /* condition still applies; suspend */
    if (pthread_cond_wait(&info->cond, &info->mutex))
    {
      fprintf(stderr, "writer: waiting for resume condition failed\n");
      pthread_mutex_unlock(&info->mutex);
      vftasks_flush_data(wport);
      pthread_exit(NULL);
    }

  /* unlock */
  pthread_mutex_unlock(&info->mutex);
}

/* called when the writer is prompted to resume
 */
void resume_writer(vftasks_wport_t *wport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vftasks_get_chan_info(vftasks_chan_of_wport(wport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* broadcast resume condition */
  if (pthread_cond_broadcast(&info->cond))
  {
    fprintf(stderr, "writer: broadcasting resume condition failed\n");
    pthread_mutex_unlock(&info->mutex);
    vftasks_flush_data(wport);
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
  vftasks_rport_t *rport;  /* read port  */

  /* retrieve read port */
  rport = (vftasks_rport_t *)arg;

  /* read data */
  for (i = 0; i < PROBLEM_SIZE; ++i) reader_data[i] = dbl(vftasks_read_int32(rport));

  /* flush */
  vftasks_flush_room(rport);

  /* exit */
  return NULL;
}

/* called when the reader is prompted to be suspended
 */
void suspend_reader(vftasks_rport_t *rport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vftasks_get_chan_info(vftasks_chan_of_rport(rport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* check whether suspend condition still applies */
  if (!vftasks_data_available(rport))
    /* condition still applies; suspend */
    if (pthread_cond_wait(&info->cond, &info->mutex))
    {
      fprintf(stderr, "reader: waiting for resume condition failed\n");
      pthread_mutex_unlock(&info->mutex);
      vftasks_flush_room(rport);
      pthread_exit(NULL);
    }

  /* unlock */
  pthread_mutex_unlock(&info->mutex);
}

/* called when the reader is prompted to resume
 */
void resume_reader(vftasks_rport_t *rport)
{
  info_t *info;  /* application data */

  /* retrieve application data */
  info = vftasks_get_chan_info(vftasks_chan_of_rport(rport));

  /* lock */
  pthread_mutex_lock(&info->mutex);

  /* broadcast resume condition */
  if (pthread_cond_broadcast(&info->cond))
  {
    fprintf(stderr, "reader: broadcasting resume condition failed\n");
    pthread_mutex_unlock(&info->mutex);
    vftasks_flush_room(rport);
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
  vftasks_malloc_t mem_mgr;  /* memory manager      */
  vftasks_chan_t *chan;      /* FIFO channel        */
  vftasks_wport_t *wport;    /* write port          */
  vftasks_rport_t *rport;    /* read port           */
  pthread_t writer_thread;   /* writer thread       */
  pthread_t reader_thread;   /* reader thread       */
  void *trv ;                /* thread return value */
  int result;                /* computed result     */

  /* intialize the writer data array */
  for (i = 0; i < PROBLEM_SIZE; ++i) writer_data[i] = i;

  /* initialize the memory manager */
  mem_mgr.malloc = malloc;
  mem_mgr.free = free;

  /* create channel */
  chan = vftasks_create_chan(FIFO_DEPTH, sizeof(int), &mem_mgr, &mem_mgr);
  if (chan == NULL)
  {
    fprintf(stderr, "channel creation failed\n");
    return 1;
  }

  /* create write port */
  wport = vftasks_create_write_port(chan, &mem_mgr);
  if (wport == NULL)
  {
    fprintf(stderr, "write-port creation failed\n");
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* create read port */
  rport = vftasks_create_read_port(chan, &mem_mgr);
  if (rport == NULL)
  {
    fprintf(stderr, "read-port creation failed\n");
    vftasks_destroy_write_port(wport, &mem_mgr);
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* install channel hooks */
  vftasks_install_chan_hooks(chan,
                             suspend_writer,
                             resume_writer,
                             suspend_reader,
                             resume_reader);

  /* set low- and high-water marks */
  vftasks_set_min_room(chan, LOW_WATER_MARK);
  vftasks_set_min_data(chan, HIGH_WATER_MARK);

  /* initialize mutex */
  rc = pthread_mutex_init(&info.mutex, NULL);
  if (rc)
  {
    fprintf(stderr, "mutex initialization failed\n");
    vftasks_destroy_write_port(wport, &mem_mgr);
    vftasks_destroy_read_port(rport, &mem_mgr);
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* initialize condition variable */
  rc = pthread_cond_init(&info.cond, NULL);
  if (rc)
  {
    fprintf(stderr, "condition-variable initialization failed\n");
    pthread_mutex_destroy(&info.mutex);
    vftasks_destroy_write_port(wport, &mem_mgr);
    vftasks_destroy_read_port(rport, &mem_mgr);
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* associate application data with channel */
  vftasks_set_chan_info(chan, &info);

  /* create and start writer thread */
  rc = pthread_create(&writer_thread, NULL, writer, wport);
  if (rc)
  {
    fprintf(stderr, "writer-thread creation failed\n");
    pthread_cond_destroy(&info.cond);
    pthread_mutex_destroy(&info.mutex);
    vftasks_destroy_write_port(wport, &mem_mgr);
    vftasks_destroy_read_port(rport, &mem_mgr);
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
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
    vftasks_destroy_write_port(wport, &mem_mgr);
    vftasks_destroy_read_port(rport, &mem_mgr);
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
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
    vftasks_destroy_write_port(wport, &mem_mgr);
    vftasks_destroy_read_port(rport, &mem_mgr);
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
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
    vftasks_destroy_write_port(wport, &mem_mgr);
    vftasks_destroy_read_port(rport, &mem_mgr);
    vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);
    return 1;
  }

  /* destroy condition variable, mutex, ports, and channel */
  pthread_cond_destroy(&info.cond);
  pthread_mutex_destroy(&info.mutex);
  vftasks_destroy_write_port(wport, &mem_mgr);
  vftasks_destroy_read_port(rport, &mem_mgr);
  vftasks_destroy_chan(chan, &mem_mgr, &mem_mgr);

  /* compute the result */
  result = sum(reader_data);

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
