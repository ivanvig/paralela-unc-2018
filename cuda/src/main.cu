#include <stdio.h>

// #define LIST_SIZE 1610612736 //6 GB of ints
//#define LIST_SIZE 209715200 //500 MB of ints
// #define LIST_SIZE 1048576 // 1MB of ints
// #define LIST_SIZE 65536
#define LIST_SIZE 16384
#define BLOCK_SIZE 1024
#define CUDA_CALL(x) {cudaError_t cuda_error__ = (x); if (cuda_error__) printf("CUDA error: " #x " returned \"%s\"\n", cudaGetErrorString(cuda_error__));}

__device__
inline void SWAP(int32_t *_a,int32_t *_b){int32_t __aux; __aux = *_a; *_a = *_b; *_b = __aux;}

void odd_even_bubble_sort_global(int32_t * list, int32_t list_size);
void odd_even_bubble_sort_shared(int32_t * list, int32_t list_size);
int assert_sorted (int * list, int list_size);

__global__
void shared_koronel(int32_t * list, int32_t list_size)
{
  __shared__ int32_t slist[2*BLOCK_SIZE];

  int64_t pos = 2*(blockDim.x * blockIdx.x + threadIdx.x);

  int win_size = 2*blockDim.x - (2*blockDim.x - list_size%(2*blockDim.x))*(((blockIdx.x+1)*2*blockDim.x) > list_size);

  slist[threadIdx.x] = list[pos];
  slist[threadIdx.x + 1] = list[pos+1];
  for (int64_t i = 0; i<win_size; i++){
    int64_t pos_oddeven = pos + (i&1);
    if (pos_oddeven < win_size-1)
      if(slist[pos_oddeven]>slist[pos_oddeven+1])
        SWAP(&slist[pos_oddeven], &slist[pos_oddeven+1]);
    __syncthreads();
  }
  list[pos] = slist[pos];
  list[pos+1] = slist[pos+1];
}

__global__
void global_koronel(int32_t * list, int32_t list_size)
{
  int64_t pos = 2*(blockDim.x * blockIdx.x + threadIdx.x);

  int win_size = 2*blockDim.x - (2*blockDim.x - list_size%(2*blockDim.x))*(((blockIdx.x+1)*2*blockDim.x) > list_size);
  for (int64_t i = 0; i<win_size; i++){
    int64_t pos_oddeven = pos + (i&1);
    if (pos_oddeven < win_size)
      if(list[pos_oddeven]>list[pos_oddeven+1])
        SWAP(&list[pos_oddeven], &list[pos_oddeven+1]);
    __syncthreads();
  }
}

int main (){
  srand(time(NULL));

  int * random_numbers_global = (int *) malloc(sizeof(int)*LIST_SIZE);
  int * random_numbers_shared = (int *) malloc(sizeof(int)*LIST_SIZE);

  printf("Generando lista aleatoria de %i elementos\n", LIST_SIZE);
  for (int i = 0; i<LIST_SIZE; i++){
    random_numbers_global[i] = rand()%20;
    // random_numbers_global[i] = LIST_SIZE - i;
  }

  memcpy(random_numbers_shared, random_numbers_global, LIST_SIZE);
  int start_print = 0;
  int n_prints = 256;
  int elem;

  printf("Lista antes de gpu: Elementos desde %i hasta %i \n", start_print, start_print+n_prints);
  for (int i=start_print; i< start_print+n_prints; i++){
    printf("%i ", random_numbers_global[i]);
  }
  printf("\n");

  //*************************************
  // ODD-EVEN BUBBLE SORT CON GLOBAL MEM
  //*************************************

  printf("Odd even bubble sort con memoria global \n");
  odd_even_bubble_sort_global(random_numbers_global, LIST_SIZE);

  printf("Despues de gpu (global): Elementos desde %i hasta %i\n", start_print, start_print+n_prints);
  for (int i=start_print; i< start_print+n_prints; i++){
    printf("%i ", random_numbers_global[i]);
  }
  printf("\n");

  printf("Chequeando si la lista con global mem esta ordenada... \n");
  if (elem = assert_sorted(random_numbers_global, LIST_SIZE)) {
    printf("LISTA MAL ORDENADA EN ELEM N %i \n", elem);
    for (int i=((elem-100) > 0)*(elem-100); i < (((elem+100) < LIST_SIZE)*(elem+100) + ((elem+100) >= LIST_SIZE)*LIST_SIZE); i++)
      printf("%i ", random_numbers_global[i]);
    printf("\n");
  } else
    printf("LISTA CON GLOBAL MEM BIEN ORDENADA \n");

  printf("Finalizado sort con memoria global \n");

  //*************************************
  // ODD-EVEN BUBBLE SORT CON SHARED MEM
  //*************************************

  printf("Odd even bubble sort con memoria shared \n");
  odd_even_bubble_sort_shared(random_numbers_shared, LIST_SIZE);

  printf("Despues de gpu (shared): Elementos desde %i hasta %i\n", start_print, start_print+n_prints);
  for (int i=start_print; i< start_print+n_prints; i++){
    printf("%i ", random_numbers_shared[i]);
  }
  printf("\n");

  printf("Chequeando si la lista con shared mem esta ordenada... \n");
  if (elem = assert_sorted(random_numbers_shared, LIST_SIZE)) {
    printf("LISTA MAL ORDENADA EN ELEM N %i \n", elem);
    for (int i=((elem-100) > 0)*(elem-100); i < (((elem+100) < LIST_SIZE)*(elem+100) + ((elem+100) >= LIST_SIZE)*LIST_SIZE); i++)
      printf("%i ", random_numbers_shared[i]);
    printf("\n");
  } else
    printf("LISTA CON SHARED MEM BIEN ORDENADA \n");

  return 0;
}

__host__
void odd_even_bubble_sort_global (int32_t * list, int32_t list_size)
{
  int32_t * device_list_ref;
  cudaEvent_t start, stop;
  CUDA_CALL(cudaEventCreate(&start));
  CUDA_CALL(cudaEventCreate(&stop));

  dim3 dimGrid ((uint)(LIST_SIZE/(2*BLOCK_SIZE)), 1, 1); //TODO: Usar ceil
  // dim3 dimGrid (1, 1, 1); //TODO: Usar ceil
	dim3 dimBlock (BLOCK_SIZE, 1, 1);

  CUDA_CALL(cudaMalloc((void **) &device_list_ref, list_size*sizeof(int32_t)));
  CUDA_CALL(cudaMemcpy(device_list_ref, list, list_size*sizeof(int32_t), cudaMemcpyHostToDevice));

  printf("Llamando al kernel con global memory... \n");
  CUDA_CALL(cudaEventRecord(start));
  for (int i = 0; i < LIST_SIZE; i++){
    if (i%(LIST_SIZE/10)==0)
      printf("%d/100...\n", 10*i/(LIST_SIZE/10));

    global_koronel<<<dimGrid, dimBlock>>>((device_list_ref + (i&1)), LIST_SIZE - (i&1));
  }
  CUDA_CALL(cudaEventRecord(stop));
  CUDA_CALL(cudaEventSynchronize(stop));
  float milliseconds = 0;
  CUDA_CALL(cudaEventElapsedTime(&milliseconds, start, stop));

  printf("Tiempo en kernel de global (ms): %f\n", milliseconds/1000);

  CUDA_CALL(cudaMemcpy(list, device_list_ref, list_size*sizeof(int32_t), cudaMemcpyDeviceToHost));
  CUDA_CALL(cudaFree(device_list_ref));
}

__host__
void odd_even_bubble_sort_shared (int32_t * list, int32_t list_size)
{
  int32_t * device_list_ref;
  cudaEvent_t start, stop;
  CUDA_CALL(cudaEventCreate(&start));
  CUDA_CALL(cudaEventCreate(&stop));

  dim3 dimGrid ((uint)(LIST_SIZE/(2*BLOCK_SIZE)), 1, 1); //TODO: Usar ceil
	dim3 dimBlock (BLOCK_SIZE, 1, 1);

  CUDA_CALL(cudaMalloc((void **) &device_list_ref, list_size*sizeof(int32_t)));
  CUDA_CALL(cudaMemcpy(device_list_ref, list, list_size*sizeof(int32_t), cudaMemcpyHostToDevice));

  printf("Llamando al kernel con shared memory... \n");
  CUDA_CALL(cudaEventRecord(start));
  for (int i = 0; i < LIST_SIZE; i++){
    if (i%(LIST_SIZE/10)==0)
      printf("%d/100...\n", 10*i/(LIST_SIZE/10));

    shared_koronel<<<dimGrid, dimBlock>>>((device_list_ref + (i&1)), LIST_SIZE - (i&1));
  }
  CUDA_CALL(cudaEventRecord(stop));
  CUDA_CALL(cudaEventSynchronize(stop));
  float milliseconds = 0;
  CUDA_CALL(cudaEventElapsedTime(&milliseconds, start, stop));

  printf("Tiempo en kernel de shared (ms): %f\n", milliseconds/1000);

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