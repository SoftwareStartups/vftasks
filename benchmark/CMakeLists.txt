include_directories(../include)

add_executable(measure_loop partitioned_loop.c)

if (${CMAKE_USE_PTHREADS_INIT})
  set(libs vftasks ${CMAKE_THREAD_LIBS_INIT} rt)
else (${CMAKE_USE_PTHREADS_INIT})
  set(libs vftasks)
endif (${CMAKE_USE_PTHREADS_INIT})

target_link_libraries(measure_loop ${libs})
