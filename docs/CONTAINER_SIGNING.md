
# Container Image Signing

- After GHCR push, sign images with **cosign keyless**:
  ```bash
  COSIGN_EXPERIMENTAL=1 cosign sign ghcr.io/OWNER/REPO:latest
  cosign verify ghcr.io/OWNER/REPO:latest
  ```
- Configure in `.github/workflows/container.yml` after the build-push step.
