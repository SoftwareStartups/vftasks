include_directories(../include)

if (${CMAKE_USE_PTHREADS_INIT})
  set(libs vftasks ${CMAKE_THREAD_LIBS_INIT})
else (${CMAKE_USE_PTHREADS_INIT})
  set(libs vftasks)
endif (${CMAKE_USE_PTHREADS_INIT})

add_executable(2d src/2d.c)

add_executable(2dsync src/2dsync.c)
target_link_libraries(2dsync ${libs})

add_executable(loop src/loop.c)

add_executable(partitioned_loop src/partitioned_loop.c)
target_link_libraries(partitioned_loop ${libs})

if (${CMAKE_USE_PTHREADS_INIT})
  add_executable(streams src/streams.c)
  target_link_libraries(streams ${libs})
endif (${CMAKE_USE_PTHREADS_INIT})

