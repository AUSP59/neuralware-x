<!-- SPDX-License-Identifier: Apache-2.0 -->
# Security Hardening
- Min TLS 1.2, legacy SSL disabled
- Secure ciphers `HIGH:!aNULL:!MD5`
- JWT signature + **claims** (iss/aud/exp/nbf) with optional leeway
