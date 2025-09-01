export interface PredictRequest { instances: number[][] }
export interface PredictResponse { predictions?: number[][]; probs?: number[][] }
export class NeuralWareXClient {
  constructor(private baseURL: string, private token?: string) {}
  async predict(payload: PredictRequest): Promise<PredictResponse> {
    const res = await fetch(new URL('/predict', this.baseURL), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', ...(this.token ? {'Authorization': `Bearer ${this.token}`} : {}) },
      body: JSON.stringify(payload)
    });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    return res.json();
  }
  async buildinfo(): Promise<any> {
    const res = await fetch(new URL('/buildinfo', this.baseURL));
    return res.json();
  }
}
