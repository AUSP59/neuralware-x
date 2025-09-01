// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stddef.h>
#ifdef _WIN32
  #ifdef NWX_CAPI_EXPORT
    #define NWX_API __declspec(dllexport)
  #else
    #define NWX_API __declspec(dllimport)
  #endif
#else
  #define NWX_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef void* nwx_handle;
NWX_API nwx_handle nwx_load_bundle(const char* path);
NWX_API int nwx_output_dim(nwx_handle h);
NWX_API int nwx_predict(nwx_handle h, const double* data, int rows, int cols, double* out_probs);
NWX_API void nwx_free(nwx_handle h);
#ifdef __cplusplus
}
#endif
