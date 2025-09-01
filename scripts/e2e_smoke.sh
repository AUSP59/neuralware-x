#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
set -euo pipefail
PORT=${1:-8081}
./out/bin/nwx_serve model.bundle $PORT --token SECRET &
PID=$!
trap "kill $PID" EXIT
sleep 1
curl -sf http://localhost:$PORT/healthz >/dev/null
curl -sf http://localhost:$PORT/ready >/dev/null || true
curl -sf -H 'Authorization: Bearer SECRET' -H 'Content-Type: application/json'   -d '{"instances":[[0,1],[1,0]]}' http://localhost:$PORT/predict >/dev/null
echo "E2E smoke OK"
