<!-- SPDX-License-Identifier: Apache-2.0 -->
# Helm Hardening Notes

- Enable `securityContext` with `runAsNonRoot: true`, `readOnlyRootFilesystem: true`.
- Attach `seccompProfile: type: Localhost` and mount `seccomp/nwx-serve.json`.
- Set liveness/readiness probes to `/healthz` and `/ready`.
