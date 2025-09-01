// SPDX-License-Identifier: Apache-2.0
#include "nwx/trainer.hpp"
#include "nwx/logging.hpp"
#include "nwx/tensor.hpp"
#include "nwx/optim.hpp"
#include "nwx/metrics.hpp"
#include "nwx/scaler.hpp"
#include "nwx/split.hpp"
#include "nwx/activation.hpp"
#include <cmath>
#include "nwx/stratified.hpp"
#include <random>
#include <limits>
#include <fstream>

namespace nwx {
static double cosine_lr(double base_lr, int t, int T, int warmup){
  if (warmup>0 && t < warmup) return base_lr * (t+1) / (double)warmup;
  double tt = std::max(0, t - warmup);
  double TT = std::max(1, T - warmup);
  return 0.5*base_lr*(1.0 + std::cos(3.141592653589793 * tt / TT));
}
static void clip_by_global_norm(std::vector<double>& v, double max_norm){
  if (max_norm<=0.0) return;
  double sum=0.0; for(double x: v) sum += x*x;
  double g = std::sqrt(sum);
  if (g>max_norm && g>0.0){ double s = max_norm / g; for(double& x: v) x *= s; }
}


static Tensor2D select_rows(const Tensor2D& X, const std::vector<int>& idx) {
  Tensor2D R((int)idx.size(), X.cols);
  for (int i = 0; i < (int)idx.size(); ++i) {
    int r = idx[(size_t)i];
    for (int j = 0; j < X.cols; ++j) R(i,j) = X(r,j);
  }
  return R;
}

static std::vector<int> select_labels(const std::vector<int>& y, const std::vector<int>& idx) {
  std::vector<int> r(idx.size());
  for (size_t i = 0; i < idx.size(); ++i) r[i] = y[(size_t)idx[i]];
  return r;
}

void train_xent(MLP& model, const Dataset& data, const TrainConfig& cfg) {
  Logger log;
  std::mt19937 rng(cfg.seed);
  Tensor2D X = data.X;
  std::vector<int> y = data.y;
  StandardScaler scaler;
  if (cfg.standardize) { scaler.fit(X); X = scaler.transform(X); }

  // Split indices
  std::vector<int> tr_idx, va_idx;
  if (cfg.val_split > 0.0) {
    auto tv = stratified_split(y, cfg.val_split, rng);
    tr_idx = std::move(tv.first);
    va_idx = std::move(tv.second);
  } else {
    tr_idx.resize(X.rows); for (int i=0;i<X.rows;++i) tr_idx[i]=i;
  }

  AdamState adam;
  double best_val = std::numeric_limits<double>::infinity();
  int bad_epochs = 0;

  // Snapshot of best weights (early stopping)
  MLP best = model;

  std::ofstream hist;
  if (!cfg.history_path.empty()) {
    hist.open(cfg.history_path, std::ios::out);
    hist << "epoch,train_loss,train_acc,val_loss,val_acc\n";
  }

  for (int epoch = 1; epoch <= cfg.epochs; ++epoch) {
    // Shuffle train indices
    std::shuffle(tr_idx.begin(), tr_idx.end(), rng);
    int B = (cfg.batch <= 0 ? (int)tr_idx.size() : cfg.batch);
    for (int off = 0; off < (int)tr_idx.size(); off += B) {
      int end = std::min(off + B, (int)tr_idx.size());
      std::vector<int> batch_idx(tr_idx.begin() + off, tr_idx.begin() + end);
      Tensor2D xb = select_rows(X, batch_idx);
      std::vector<int> yb = select_labels(y, batch_idx);

      // Forward
      Tensor2D logits = model.forward(xb);
      auto probs = softmax(logits);

      // Grad logits
      Tensor2D dlogits = probs;
      for (int i = 0; i < dlogits.rows; ++i) dlogits(i, yb[(size_t)i]) -= 1.0;
      for (double& v : dlogits.data) v /= (double)dlogits.rows;

      // Grad W2, b2
      Tensor2D a1T(model.a1.cols, model.a1.rows);
      for (int i = 0; i < model.a1.rows; ++i) for (int j = 0; j < model.a1.cols; ++j) a1T(j,i) = model.a1(i,j);
      Tensor2D gW2 = matmul(a1T, dlogits);
      std::vector<double> gb2(model.b2.size(), 0.0);
      for (int i = 0; i < dlogits.rows; ++i) for (int j = 0; j < dlogits.cols; ++j) gb2[(size_t)j] += dlogits(i,j);

      // Backprop to a1
      Tensor2D d_a1(model.a1.rows, model.a1.cols);
      for (int i = 0; i < model.a1.rows; ++i) {
        for (int j = 0; j < model.a1.cols; ++j) {
          double sum = 0.0;
          for (int k = 0; k < dlogits.cols; ++k) sum += dlogits(i,k) * model.W2(j,k);
          d_a1(i,j) = (model.x1(i,j) > 0.0) ? sum : 0.0;
        }
      }

      // Grad W1, b1
      Tensor2D XT(xb.cols, xb.rows);
      for (int i = 0; i < xb.rows; ++i) for (int j = 0; j < xb.cols; ++j) XT(j,i) = xb(i,j);
      Tensor2D gW1 = matmul(XT, d_a1);
      std::vector<double> gb1(model.b1.size(), 0.0);
      for (int i = 0; i < d_a1.rows; ++i) for (int j = 0; j < d_a1.cols; ++j) gb1[(size_t)j] += d_a1(i,j);

      // Update
      if (cfg.optimizer == "adam" || cfg.optimizer == "adamw") {
        adam_update(adam, model.W1, model.b1, model.W2, model.b2, gW1, gb1, gW2, gb2, cfg.lr, cfg.weight_decay);
      } else {
        sgd_update(model.W1, model.b1, model.W2, model.b2, gW1, gb1, gW2, gb2, cfg.lr, cfg.weight_decay);
      }
    }

    // Epoch metrics
    auto train_probs = softmax(model.forward(select_rows(X, tr_idx)));
    double train_loss = cross_entropy(train_probs, select_labels(y, tr_idx));
    double train_acc = accuracy(train_probs, select_labels(y, tr_idx));

    if (!va_idx.empty()) {
      auto val_probs = softmax(model.forward(select_rows(X, va_idx)));
      double val_loss = cross_entropy(val_probs, select_labels(y, va_idx));
      double val_acc = accuracy(val_probs, select_labels(y, va_idx));
      if (hist.is_open()) hist << epoch << "," << train_loss << "," << train_acc << "," << val_loss << "," << val_acc << "\n";
      log.info("epoch %d/%d  train_loss=%.6f acc=%.3f  val_loss=%.6f acc=%.3f",
               epoch, cfg.epochs, train_loss, train_acc, val_loss, val_acc);
      if (val_loss + 1e-12 < best_val) { best_val = val_loss; bad_epochs = 0; best = model; }
      else if (cfg.patience > 0 && ++bad_epochs >= cfg.patience) {
        log.warn("Early stopping at epoch %d (patience %d)", epoch, cfg.patience);
        model = best;
        break;
      }
    } else {
      if (hist.is_open()) hist << epoch << "," << train_loss << "," << train_acc << ",,\n";
      log.info("epoch %d/%d  loss=%.6f  acc=%.3f", epoch, cfg.epochs, train_loss, train_acc);
    }
  }
}

} // namespace nwx
