import http from 'k6/http';
import { sleep } from 'k6';
export const options = { vus: 20, duration: '30s' };
export default function () {
  const url = 'http://localhost:8080/predict';
  const payload = '0,1\n1,0';
  const params = { headers: { 'Content-Type': 'text/csv', 'Authorization': 'Bearer SECRET' } };
  http.post(url, payload, params);
  sleep(0.1);
}
