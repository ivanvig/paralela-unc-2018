#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// #define LIST_SIZE 1610612736 //6 GB of ints
#define LIST_SIZE 100 //6 GB of ints
#define BLOCK_SIZE 1024
#define CUDA_CALL(x) {cudaError_t cuda_error__ = (x); if (cuda_error__) printf("CUDA error: " #x " returned \"%s\"\n", cudaGetErrorString(cuda_error__));}

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

  int start_print = 0;
  int n_prints = 20;
  printf("antes de gpu \n");
  for (int i=start_print; i< start_print+n_prints; i++){
    printf("%i ", random_numbers[i]);
  }
  odd_even_bubble_sort(random_numbers, LIST_SIZE);

  printf("\ndespues de gpu \n");
  for (int i=start_print; i< start_print+n_prints; i++){
    printf("%i ", random_numbers[i]);
  }

  return 0;
}

__global__
void odd_even_bubble_sort_kernel(int32_t * unsorted_list, int32_t list_size)
{
  int32_t block_id = blockDim.x * blockIdx.x;
  int32_t thread_id = threadIdx.x;

  if ((block_id+thread_id)*2<20){
    unsorted_list[(block_id+thread_id)*2]=10;
  }
  // if (unsorted_list[(block_id+thread_id)*2] > unsorted_list[(block_id+thread_id)*2+1])
  //   SWAP(&unsorted_list[(block_id+thread_id)*2], &unsorted_list[(block_id+thread_id)*2+1]);
}



__host__
void odd_even_bubble_sort(int32_t * list, int32_t list_size)
{
  int32_t * cuda_dev;
  dim3 dimGrid ((uint)(LIST_SIZE/(2*BLOCK_SIZE)), 1, 1); //TODO: Usar ceil
	dim3 dimBlock (BLOCK_SIZE, 1, 1);

  CUDA_CALL(cudaMalloc(&cuda_dev, sizeof(int32_t)*list_size));

  CUDA_CALL(cudaMemcpy(cuda_dev, list, sizeof(int32_t)*list_size, cudaMemcpyHostToDevice));

  odd_even_bubble_sort_kernel<<<dimGrid, dimBlock>>>(list, list_size);

  CUDA_CALL(cudaDeviceSynchronize());

  CUDA_CALL(cudaMemcpy(list, cuda_dev, sizeof(int32_t)*list_size, cudaMemcpyDeviceToHost));
}



int assert_sorted (int * list, int list_size)
{
  for (int i=0; i<list_size-1; i++){
    if (list[i] > list[i+1])
      return 1;
  }

  return 0;
}
