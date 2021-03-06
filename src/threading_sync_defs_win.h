#ifndef THREADING_SYNC_DEFS_WIN_H
#define THREADING_SYNC_DEFS_WIN_H

#include <windows.h>

#ifndef __cplusplus
#define inline __inline
#endif

typedef HANDLE thread_t;
typedef DWORD tls_key_t;
typedef HANDLE mutex_t;
typedef HANDLE semaphore_t;


#define THREAD_CREATE(THREAD,FUNC,ARG) \
  (!(((THREAD) = CreateThread(NULL, 0, FUNC, (ARG), 0, NULL)) != NULL))

#define THREAD_EXIT_SUCCESS 0
#define THREAD_EXIT() ExitThread(THREAD_EXIT_SUCCESS)

#define THREAD_JOIN(THREAD)                     \
  {                                             \
    WaitForSingleObject(THREAD, INFINITE);      \
    CloseHandle(THREAD);                        \
  }


#define TLS_CREATE(KEY) (!(((KEY) = TlsAlloc()) != TLS_OUT_OF_INDEXES))
#define TLS_DESTROY(KEY) TlsFree(KEY)
#define TLS_SET(KEY,VAL) (!TlsSetValue(KEY, VAL))
#define TLS_GET(KEY) TlsGetValue(KEY)


#define WORKER_PROTO(FUNC,ARG) DWORD WINAPI FUNC(LPVOID ARG)


#define MUTEX_LOCK(MUTEX) (!(WaitForSingleObject(MUTEX, INFINITE) == WAIT_OBJECT_0))
#define MUTEX_UNLOCK(MUTEX) ReleaseMutex(MUTEX)
#define MUTEX_CREATE(MUTEX) (!(((MUTEX) = CreateMutex(NULL, FALSE, NULL)) != NULL))

#define MUTEX_DESTROY(MUTEX) \
  {                          \
    MUTEX_UNLOCK(MUTEX);     \
    CloseHandle(MUTEX);      \
  }


#define SEMAPHORE_CREATE(SEM,VALUE,MAX)                                 \
  (!(((SEM) = CreateSemaphore(NULL, VALUE, MAX, NULL)) != NULL))

#define SEMAPHORE_DESTROY(SEM) CloseHandle(SEM)
#define SEMAPHORE_WAIT(SEM) (!(WaitForSingleObject(SEM, INFINITE) == WAIT_OBJECT_0))
#define SEMAPHORE_POST(SEM) (!ReleaseSemaphore(SEM, 1, NULL))

#endif /* THREADING_SYNC_DEFS_WIN_H */
