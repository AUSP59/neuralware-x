# NEURALWARE-X OMEGA - Reproducible build environment
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN ./scripts/build.sh -DCMAKE_BUILD_TYPE=Release && ./scripts/test.sh
CMD ["bash"]
