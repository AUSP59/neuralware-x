// SPDX-License-Identifier: Apache-2.0
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <stdexcept>
#include <cstring>

extern "C" {
// Row-major A[m,k] * B[k,n] => C[m,n]
// Returns 0 on success; negative on error.
int nwx_cublas_dgemm(int m, int n, int k, const double* A_host, const double* B_host, double* C_host) {
  cudaError_t ce;
  cublasStatus_t st;
  double *A=NULL, *B=NULL, *C=NULL;
  size_t sA = (size_t)m*k*sizeof(double);
  size_t sB = (size_t)k*n*sizeof(double);
  size_t sC = (size_t)m*n*sizeof(double);

  if ((ce=cudaMalloc((void**)&A, sA)) != cudaSuccess) return -1;
  if ((ce=cudaMalloc((void**)&B, sB)) != cudaSuccess) { cudaFree(A); return -2; }
  if ((ce=cudaMalloc((void**)&C, sC)) != cudaSuccess) { cudaFree(A); cudaFree(B); return -3; }

  if ((ce=cudaMemcpy(A, A_host, sA, cudaMemcpyHostToDevice)) != cudaSuccess) { cudaFree(A); cudaFree(B); cudaFree(C); return -4; }
  if ((ce=cudaMemcpy(B, B_host, sB, cudaMemcpyHostToDevice)) != cudaSuccess) { cudaFree(A); cudaFree(B); cudaFree(C); return -5; }

  cublasHandle_t h;
  if ((st=cublasCreate(&h)) != CUBLAS_STATUS_SUCCESS) { cudaFree(A); cudaFree(B); cudaFree(C); return -6; }

  // Use row-major by swapping operands and dims (C = A*B row-major => C^T = B^T * A^T col-major)
  const double alpha=1.0, beta=0.0;
  st = cublasDgemm(h, CUBLAS_OP_T, CUBLAS_OP_T,
                   n, m, k,
                   &alpha,
                   B, k,  // B^T: (n x k) uses B with leading dim k
                   A, m,  // A^T: (k x m) uses A with leading dim m
                   &beta,
                   C, n); // C^T: (n x m) laid out row-major target with ld=n
  if (st != CUBLAS_STATUS_SUCCESS) { cublasDestroy(h); cudaFree(A); cudaFree(B); cudaFree(C); return -7; }

  st = cublasDestroy(h);
  if ((ce=cudaMemcpy(C_host, C, sC, cudaMemcpyDeviceToHost)) != cudaSuccess) { cudaFree(A); cudaFree(B); cudaFree(C); return -8; }

  cudaFree(A); cudaFree(B); cudaFree(C);
  cudaDeviceSynchronize();
  return 0;
}
} // extern "C"
