# SPDX-License-Identifier: Apache-2.0
.PHONY: all build test install lint cppcheck format docs sbom bench run-serve

all: build test

build:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build --parallel

test:
	ctest --test-dir build --output-on-failure || true

install:
	cmake --install build

lint:
	./tools/lint.sh

cppcheck:
	cppcheck --enable=warning,style,performance,portability --std=c++20 -I include src

format:
	clang-format -i $(shell git ls-files '*.cpp' '*.hpp' '*.h')

docs:
	mkdocs build

sbom:
	syft packages dir:. -o spdx-json=sbom.spdx.json

bench:
	./out/bin/nwx_bench ./examples/xor.csv

run-serve:
	./out/bin/nwx_serve model.bundle 8080 --token SECRET --max_concurrency 128
