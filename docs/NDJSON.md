<!-- SPDX-License-Identifier: Apache-2.0 -->
# NDJSON Output

Request NDJSON by either:
- Header: `Accept: application/x-ndjson`
- Query: `/predict?format=ndjson`

Each line returns probabilities for a row as a compact JSON object.
