#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LIST_SIZE 1610612736 //6 GB of ints
#define BLOCK_SIZE 1024

int * random_numbers_cuda = (int *) malloc(sizeof(int)*LIST_SIZE);

__device__
inline void SWAP(int32_t *_a,int32_t *_b){int32_t __aux; __aux = *_a; *_a = *_b; *_b = __aux;}

int assert_sorted(int * list, int list_size);
void odd_even_bubble_sort(int32_t * list, int32_t list_size);

int main (){
  srand(time(NULL));

  int * random_numbers = (int *) malloc(sizeof(int)*LIST_SIZE);

  for (int i = 0; i<LIST_SIZE; i++){
    random_numbers[i] = rand()%20;
  }

  printf("antes de gpu \n");
  for (int i=0; i< 20; i++){
    printf("%i ", random_numbers[i]);
  }
  odd_even_bubble_sort(random_numbers, LIST_SIZE);

  printf("despues de gpu \n");
  for (int i=0; i< 20; i++){
    printf("%i ", random_numbers_cuda[i]);
  }
  //assert_sorted()

  return 0;
}

__global__
void odd_even_bubble_sort_kernel(int32_t * unsorted_list, int32_t list_size)
{
  int32_t block_id = blockDim.x * blockIdx.x;
  int32_t thread_id = threadIdx.x;

  if (unsorted_list[(block_id+thread_id)*2] > unsorted_list[(block_id+thread_id)*2+1])
    SWAP(&unsorted_list[(block_id+thread_id)*2], &unsorted_list[(block_id+thread_id)*2+1]);

}



__host__
void odd_even_bubble_sort(int32_t * list, int32_t list_size)
{
  int32_t * cuda_dev;
  cudaError_t error;
  error = cudaMalloc(&cuda_dev, sizeof(int32_t)*list_size);

  cudaMemcpy(cuda_dev, list, sizeof(int32_t)*list_size, cudaMemcpyHostToDevice);

  /* odd_even_bubble_sort_kernel<<<(LIST_SIZE/(2*BLOCK_SIZE),1,1), (BLOCK_SIZE,1,1)>>>(list, list_size); */
  odd_even_bubble_sort_kernel<<<(LIST_SIZE/(2*BLOCK_SIZE)), BLOCK_SIZE>>>(list, list_size);

  cudaMemcpy(random_numbers_cuda, cuda_dev, sizeof(int32_t)*list_size, cudaMemcpyDeviceToHost);
}



int assert_sorted (int * list, int list_size)
{
  for (int i=0; i<list_size-1; i++){
    if (list[i] > list[i+1])
      return 1;
  }

  return 0;
}
