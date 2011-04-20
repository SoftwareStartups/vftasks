/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

/* Example: usage of worker thread pools.
 * Computations on an array are partitioned into 4 tasks which are distributed
 * among a pool of worker threads.
 */

#include <stdio.h>

#define M 1024

int a[M];

int go()
{
  int i;

  for (i = 0; i < M; i++)
    a[i] = i * i;

  return i;
}

int test(int result)
{
  int i, acc = 0;

  for (i = 0; i < M; i++)
    acc += a[i];

  return (acc == 357389824) && (result == M);
}

int main()
{
  int result;

  result = go();

  if (test(result))
  {
    printf("PASSED\n");
    return 0;
  }
  else
  {
    printf("FAILED\n");
    return 1;
  }
}
