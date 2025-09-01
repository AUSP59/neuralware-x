<!-- SPDX-License-Identifier: Apache-2.0 -->
# Admission Policies (Cluster)

- **Kyverno verifyImages** (`deploy/kyverno/verify-images.yaml`): requires **cosign keyless signatures** from GitHub Actions OIDC for images `ghcr.io/*/neuralware-x:*`. Adjust namespaces and subjects to your org.
- **Seccomp/AppArmor**: ship example profiles in `deploy/security/` to run as **nonroot** with **RuntimeDefault** or stricter policies.
- **Verification helper**: `tools/verify_image_signature.sh` uses `cosign verify` to check signatures prior to deploy.

> Note: Applying cluster policies needs cluster-admin. Run in a non-prod env first.
