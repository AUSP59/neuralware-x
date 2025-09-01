# SPDX-License-Identifier: Apache-2.0
import os, ctypes, numpy as np
def _load_lib():
    names = ["libnwx_capi.so", "nwx_capi.dll", "libnwx_capi.dylib"]
    env = os.getenv("NWX_CAPI_PATH")
    if env and os.path.exists(env): return ctypes.CDLL(env)
    for n in names:
        try: return ctypes.CDLL(n)
        except OSError: pass
    here = os.path.dirname(__file__)
    for n in names:
        p = os.path.join(here, n)
        if os.path.exists(p): return ctypes.CDLL(p)
    raise OSError("Could not load nwx_capi; set NWX_CAPI_PATH")
_lib = _load_lib()
_lib.nwx_load_bundle.restype = ctypes.c_void_p
_lib.nwx_load_bundle.argtypes = [ctypes.c_char_p]
_lib.nwx_output_dim.restype = ctypes.c_int
_lib.nwx_output_dim.argtypes = [ctypes.c_void_p]
_lib.nwx_predict.restype = ctypes.c_int
_lib.nwx_predict.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_double), ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_double)]
_lib.nwx_free.restype = None
_lib.nwx_free.argtypes = [ctypes.c_void_p]
class NeuralWareX:
    def __init__(self, bundle_path:str):
        self._h = _lib.nwx_load_bundle(bundle_path.encode('utf-8'))
        if not self._h: raise RuntimeError("Failed to load bundle")
        self._out = _lib.nwx_output_dim(self._h)
    def predict(self, X: np.ndarray) -> np.ndarray:
        X = np.asarray(X, dtype=np.float64, order='C')
        rows, cols = X.shape
        out = np.empty((rows, self._out), dtype=np.float64)
        rc = _lib.nwx_predict(self._h, X.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), rows, cols, out.ctypes.data_as(ctypes.POINTER(ctypes.c_double)))
        if rc != 0: raise RuntimeError("nwx_predict failed")
        return out
    def __del__(self):
        try:
            if getattr(self, "_h", None): _lib.nwx_free(self._h); self._h=None
        except Exception: pass
