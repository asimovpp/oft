#include <stdlib.h>
#include <stdio.h>

double *op_malloc_(int *nbytes)
{
  double *ptr;

  if (*nbytes == 0) *nbytes = 1;

  ptr = (double *) malloc(*nbytes);

  if (ptr == NULL)
    {
      printf("Failed to allocate %i bytes\n",*nbytes);
      return NULL;
    }
  else
    printf("Allocated %i bytes at ptr %p\n",*nbytes,ptr); 

  return ptr;
}
