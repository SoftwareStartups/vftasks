#include <stdio.h>

#define M 1024
#define N 1024

int a[M][N];

void go()
{
  int i, j;

  for (i = 0; i < M; i++)
  {
    for (j = 0; j < N; j++)
    {
      if (i > 0 && j + 1 < N)
      {
        a[i][j] = i * j + a[i - 1][j + 1];
      }
      else
      {
        a[i][j] = i * j;
      }
    }
  }
}

int test()
{
  int i, j;
  int acc = 0;

  for (i = 0; i < M; i++)
  {
    for (j = 0; j < N; j++)
    {
      acc += a[i][j];
    }
  }

  return (acc == 438488320);
}

int main()
{
  go();

  if (test())
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
