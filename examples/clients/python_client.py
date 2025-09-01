# SPDX-License-Identifier: Apache-2.0
import requests, json, sys
url = sys.argv[1] if len(sys.argv)>1 else "http://localhost:8080/predict"
data = {"instances":[[0,1],[1,0]]}
r = requests.post(url, headers={"Authorization":"Bearer SECRET"}, json=data, timeout=5)
print(r.status_code, r.text[:200])
