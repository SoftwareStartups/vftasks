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
