#include <stdio.h>

// #define LIST_SIZE 1610612736 //6 GB of ints
//#define LIST_SIZE 209715200 //500 MB of ints
// #define LIST_SIZE 1048576 // 1MB of ints
#define LIST_SIZE 65536
// #define LIST_SIZE 4096
#define BLOCK_SIZE 1024
#define CUDA_CALL(x) {cudaError_t cuda_error__ = (x); if (cuda_error__) printf("CUDA error: " #x " returned \"%s\"\n", cudaGetErrorString(cuda_error__));}

__device__
inline void SWAP(int32_t *_a,int32_t *_b){int32_t __aux; __aux = *_a; *_a = *_b; *_b = __aux;}

void odd_even_bubble_sort_global(int32_t * list, int32_t list_size);
int assert_sorted (int * list, int list_size);

__global__
void koronel(int32_t * list, int32_t list_size)
{
  int64_t pos = 2*(blockDim.x * blockIdx.x + threadIdx.x);

  for (int64_t i = 0; i<list_size; i++){
    int64_t pos_oddeven = pos + (i&1);
    if (pos_oddeven < list_size-1)
      if(list[pos_oddeven]>list[pos_oddeven+1])
        SWAP(&list[pos_oddeven], &list[pos_oddeven+1]);
    __syncthreads();
  }
}

int main (){
  srand(time(NULL));

  int * random_numbers = (int *) malloc(sizeof(int)*LIST_SIZE);

  printf("Generando lista aleatoria de %i elementos\n", LIST_SIZE);
  for (int i = 0; i<LIST_SIZE; i++){
    random_numbers[i] = rand()%20;
    // random_numbers[i] = LIST_SIZE - i;
    // random_numbers[i] = 0;
  }
  // random_numbers[0] = 111;
  // random_numbers[LIST_SIZE-1] = -1;
  int start_print = 0;
  int n_prints = 2048;
  printf("Antes de gpu: Elementos desde %i hasta %i \n", start_print, start_print+n_prints);
  for (int i=start_print; i< start_print+n_prints; i++){
    printf("%i ", random_numbers[i]);
  }
  printf("\n");
  printf("Odd even bubble sort con memoria global \n");
  odd_even_bubble_sort_global(random_numbers, LIST_SIZE);

  printf("Chequeando si la lista esta ordenada... \n");
  int elem;
  if (elem = assert_sorted(random_numbers, LIST_SIZE)) {
    printf("LISTA MAL ORDENADA EN ELEM N %i \n", elem);
    for (int i=((elem-100) > 0)*(elem-100); i < (((elem+100) < LIST_SIZE)*(elem+100) + ((elem+100) >= LIST_SIZE)*LIST_SIZE); i++)
      printf("%i ", random_numbers[i]);
    printf("\n");
  } else
    printf("LISTA BIEN ORDENADA \n");

  printf("Despues de gpu: Elementos desde %i hasta %i\n", start_print, start_print+n_prints);
  for (int i=start_print; i< start_print+n_prints; i++){
    printf("%i ", random_numbers[i]);
  }
  printf("\n");

  return 0;
}

__host__
void odd_even_bubble_sort_global (int32_t * list, int32_t list_size)
{
  int32_t * device_list_ref;

  // dim3 dimGrid ((uint)(LIST_SIZE/(2*BLOCK_SIZE)), 1, 1); //TODO: Usar ceil
  dim3 dimGrid (1, 1, 1); //TODO: Usar ceil
	dim3 dimBlock (BLOCK_SIZE, 1, 1);

  CUDA_CALL(cudaMalloc((void **) &device_list_ref, list_size*sizeof(int32_t)));
  CUDA_CALL(cudaMemcpy(device_list_ref, list, list_size*sizeof(int32_t), cudaMemcpyHostToDevice));

  printf("Llamando al kernel... \n");
  for (int i = 0; i < LIST_SIZE; i++)
    for (int j = 0; j < LIST_SIZE/BLOCK_SIZE; j++)
      koronel<<<dimGrid, dimBlock>>>((device_list_ref + BLOCK_SIZE*j + (i&1)), BLOCK_SIZE - (BLOCK_SIZE - (LIST_SIZE - (i&1))%BLOCK_SIZE)*(((j+1)*BLOCK_SIZE + (i&1)) > LIST_SIZE));

  CUDA_CALL(cudaMemcpy(list, device_list_ref, list_size*sizeof(int32_t), cudaMemcpyDeviceToHost));
  CUDA_CALL(cudaFree(device_list_ref));
}

__host__
int assert_sorted (int * list, int list_size)
{
  for (int i=0; i<list_size-1; i++){
    if (list[i] > list[i+1])
      return i+1;
  }
  return 0;
}