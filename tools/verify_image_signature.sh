#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
IMG="${1:?IMAGE_REF required, e.g., ghcr.io/your-org/neuralware-x:main}"
ISSUER="${2:-https://token.actions.githubusercontent.com}"
SUBJECT="${3:-https://github.com/*/*/.github/workflows/*}"
cosign verify --certificate-oidc-issuer "$ISSUER" --certificate-identity "$SUBJECT" "$IMG"
