<!-- SPDX-License-Identifier: Apache-2.0 -->
# JWT (HS256) and mTLS (optional)

- Enable JWT HS256 by building with OpenSSL and passing `--jwt_hs256 SECRET` or env `NWX_JWT_HS256`.
- Require client certificates by setting `NWX_REQUIRE_CLIENT_CERT=1` and providing `--tls_cert/--tls_key`.
