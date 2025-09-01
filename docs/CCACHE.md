
# ccache

Enable compiler caching to speed up rebuilds:
```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchains/ccache.cmake
```
On CI, a dedicated workflow seeds and preserves the cache.
