<!-- SPDX-License-Identifier: Apache-2.0 -->
# TLS (Optional)

Build with TLS:
```bash
cmake -S . -B build -DNWX_ENABLE_OPENSSL=ON
cmake --build build --target nwx_serve
./out/bin/nwx_serve model.bundle 8443 --tls_cert cert.pem --tls_key key.pem
```
Note: TLS is optional and only enabled when OpenSSL is available at build-time.
