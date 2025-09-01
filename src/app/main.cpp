// SPDX-License-Identifier: Apache-2.0
#include "nwx/version.hpp"
#include "nwx/logging.hpp"
#include "nwx/json.hpp"
#include "nwx/trainer.hpp"
#include "nwx/csv.hpp"
#include "nwx/serialize.hpp"
#include "nwx/bundle.hpp"
#include "nwx/tensor.hpp"
#include "nwx/metrics_ex.hpp"
#include "nwx/roc_auc.hpp"
#include "nwx/calibration.hpp"
#include "nwx/scaler.hpp"
#include <iostream>
#include <filesystem>
#include <random>
#include <fstream>

using namespace nwx;

static void usage() {
  std::cout << "nwx " << std::string(kVersion) << "\n";
  std::cout << "Usage: nwx [--help] [--config <file>] [--dataset <csv>] [--epochs N] [--lr F] [--hidden N] [--seed N]\n"
               "           [--save path] [--load path] [--eval]\n"
               "           [--optimizer sgd|adam] [--wd F] [--batch N]\n"
               "           [--val_split F] [--patience N] [--standardize]\n"
               "           [--predict <csv>] [--predict_out <path>] [--history <csv>] [--activation relu|tanh] [--lr_schedule none|cosine] [--warmup_steps N] [--clip_norm X]\n";
}

int main(int argc, char** argv) {
  Logger log(LogLevel::Info);
  std::string config_path;
  std::string dataset_path = "./examples/xor.csv";
  int epochs = 1000;
  double lr = 0.1;
  int hidden = 8;
  int seed = 42;
  std::string save_path, load_path;
  bool eval_only = false;
  // New features
  std::string optimizer = "sgd";
  double wd = 0.0;
  int batch = 0;
  double val_split = 0.0;
  int patience = 0;
  bool standardize = false;
  std::string predict_path, predict_out, history_path, activation = "relu"; bool calibrate_temp=false; std::string calib_metric="nll"; std::string lr_schedule="none"; int warmup_steps=0; double clip_norm=0.0; bool quantize_runtime=false;

  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    if (a == "--help" || a == "-h") { usage(); return 0; }
    else if (a == "--config" && i+1 < argc) { config_path = argv[++i]; }
    else if (a == "--dataset" && i+1 < argc) { dataset_path = argv[++i]; }
    else if (a == "--epochs" && i+1 < argc) { epochs = std::stoi(argv[++i]); }
    else if (a == "--lr" && i+1 < argc) { lr = std::stod(argv[++i]); }
    else if (a == "--hidden" && i+1 < argc) { hidden = std::stoi(argv[++i]); }
    else if (a == "--seed" && i+1 < argc) { seed = std::stoi(argv[++i]); }
    else if (a == "--save" && i+1 < argc) { save_path = argv[++i]; }
    else if (a == "--load" && i+1 < argc) { load_path = argv[++i]; }
    else if (a == "--eval") { eval_only = true; }

    else if (a == "--optimizer" && i+1 < argc) { optimizer = argv[++i]; }
    else if (a == "--wd" && i+1 < argc) { wd = std::stod(argv[++i]); }
    else if (a == "--batch" && i+1 < argc) { batch = std::stoi(argv[++i]); }
    else if (a == "--val_split" && i+1 < argc) { val_split = std::stod(argv[++i]); }
    else if (a == "--patience" && i+1 < argc) { patience = std::stoi(argv[++i]); }
    else if (a == "--standardize") { standardize = true; }
    else if (a == "--predict" && i+1 < argc) { predict_path = argv[++i]; }
    else if (a == "--predict_out" && i+1 < argc) { predict_out = argv[++i]; }
    else if (a == "--history" && i+1 < argc) { history_path = argv[++i]; }
    else if (a == "--calibrate_temp") { calibrate_temp = true; }
    else if (a == "--calibration_metric" && i+1 < argc) { calib_metric = argv[++i]; }
    else if (a == "--lr_schedule" && i+1 < argc) { lr_schedule = argv[++i]; }
    else if (a == "--warmup_steps" && i+1 < argc) { warmup_steps = std::stoi(argv[++i]); }
    else if (a == "--clip_norm" && i+1 < argc) { clip_norm = std::stod(argv[++i]); }
    else if (a == "--quantize_runtime") { quantize_runtime = true; }
    else if (a == "--activation" && i+1 < argc) { activation = argv[++i]; }
    else { std::cerr << "Unknown arg: " << a << "\n"; usage(); return 2; }
  }

  if (!config_path.empty()) {
    std::ifstream in(config_path);
    if (!in) { log.error("Failed to open config: %s", config_path.c_str()); return 3; }
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    JsonFlat jf; if (!jf.parse(s)) { log.error("Invalid JSON in %s", config_path.c_str()); return 3; }
    if (auto v = jf.get_string("dataset")) dataset_path = *v;
    if (auto v = jf.get_number("epochs")) epochs = (int)*v;
    if (auto v = jf.get_number("learning_rate")) lr = *v;
    if (auto v = jf.get_number("hidden")) hidden = (int)*v;
    if (auto v = jf.get_number("seed")) seed = (int)*v;
    if (auto v = jf.get_string("optimizer")) optimizer = *v;
    if (auto v = jf.get_number("weight_decay")) wd = *v;
    if (auto v = jf.get_number("batch")) batch = (int)*v;
    if (auto v = jf.get_number("val_split")) val_split = *v;
    if (auto v = jf.get_number("patience")) patience = (int)*v;
    if (auto v = jf.get_bool("standardize")) standardize = *v;
    if (auto v = jf.get_string("activation")) activation = *v;
    if (auto v = jf.get_string("lr_schedule")) lr_schedule = *v;
    if (auto v = jf.get_int("warmup_steps")) warmup_steps = *v;
    if (auto v = jf.get_double("clip_norm")) clip_norm = *v;
    if (auto v = jf.get_bool("calibrate_temp")) calibrate_temp = *v;
    if (auto v = jf.get_string("calibration_metric")) calib_metric = *v;
  }

  // Predict-only path requires --load
  if (!predict_path.empty() && load_path.empty()) {
    log.error("--predict requires --load <model>");
    return 2;
  }

  // Load dataset unless predict-only
  Tensor2D X;
  std::vector<int> y;
  int out_classes = 0;
  if (predict_path.empty()) {
    auto rows = read_csv_numeric(dataset_path, ',');
    if (rows.empty()) { log.error("Empty dataset: %s", dataset_path.c_str()); return 4; }
    int in_dim = (int)rows[0].size() - 1;
    out_classes = 0;
    for (int i = 0; i < (int)rows.size(); ++i) {
      for (int j = 0; j < in_dim; ++j) {
        if (i==0 && j==0) X = Tensor2D((int)rows.size(), in_dim);
        X(i,j) = rows[i][j];
      }
      int label = (int)rows[i][in_dim];
      if (label+1 > out_classes) out_classes = label+1;
      y.push_back(label);
    }
  }

  TrainConfig cfg;
  cfg.epochs = epochs;
  cfg.lr = lr;
  cfg.hidden = hidden;
  cfg.seed = seed;
  cfg.weight_decay = wd;
  cfg.batch = batch;
  cfg.val_split = val_split;
  cfg.patience = patience;
  cfg.standardize = standardize;
  cfg.optimizer = optimizer;
  cfg.history_path = history_path;
  cfg.activation = activation;
  cfg.lr_schedule = lr_schedule;
  cfg.warmup_steps = warmup_steps;
  cfg.clip_norm = clip_norm;

  std::mt19937 rng(cfg.seed);
  MLP model;
  StandardScaler fitted_scaler; bool have_scaler=false; if (standardize && predict_path.empty()) { fitted_scaler.fit(X); have_scaler=true; }
  if (!load_path.empty()) {
    ModelBundle b; if (!load_bundle(b, load_path)) { log.error("Failed to load bundle: %s", load_path.c_str()); return 5; } model = b.model; StandardScaler bundle_scaler = b.scaler; bool bundle_has_scaler = b.has_scaler;
  } else {
    if (!predict_path.empty()) { log.error("No model to use for prediction"); return 2; }
  }

  if (predict_path.empty()) {
    if (load_path.empty()) {
      model.init(X.cols, cfg.hidden, out_classes, rng);
    }
    train_xent(model, Dataset{X,y,out_classes}, cfg);
    if (!save_path.empty()) {
      ModelBundle b; b.model = model; b.has_scaler = have_scaler; if (have_scaler) b.scaler = fitted_scaler;
      if (!save_bundle(b, save_path)) { log.error("Failed to save bundle to %s", save_path.c_str()); return 6; }
      log.info("Bundle saved to %s", save_path.c_str());
    }
    auto probs = softmax(model.forward(X));
    double loss = cross_entropy(probs, y);
    double acc = accuracy(probs, y);
    std::cout << "Final loss=" << loss << " acc=" << acc << "\n";

    // Extra metrics
    auto probs_full = softmax(model.forward(standardize ? fitted_scaler.transform(X) : X));
    auto f1 = f1_macro_micro(probs_full, y, out_classes);
    std::cout << "F1_macro=" << std::get<0>(f1) << " F1_micro=" << std::get<1>(f1) << "\n";
    if (out_classes == 2) {
      std::cout << "ROC_AUC=" << roc_auc_binary(probs_full, y) << " PR_AUC=" << pr_auc_binary(probs_full, y) << "\n";
    } else {
      std::cout << "ROC_AUC_OvR=" << roc_auc_ovr_macro(probs_full, y, out_classes) << "\n";
    }
    std::cout << "ECE=" << expected_calibration_error(probs_full, y, 10) << "\n";
    if (quantize_runtime) { enable_runtime_quantization(model); std::cout << "Runtime INT8 quantization enabled.\n"; }

    // Temperature scaling calibration on validation split (if requested)
    if (calibrate_temp && val_split > 0.0) {
      // Recreate the same split deterministically
      std::mt19937 split_rng(seed);
      auto tv = stratified_split(y, val_split, split_rng);
      auto tr_idx = tv.first; auto va_idx = tv.second;
      auto Xstd = standardize ? fitted_scaler.transform(X) : X;
      auto Xv = Tensor2D((int)va_idx.size(), Xstd.cols);
      std::vector<int> yv(va_idx.size());
      for (int i=0;i<(int)va_idx.size();++i){ int r=va_idx[i]; yv[i]=y[(size_t)r]; for (int j=0;j<Xstd.cols;++j) Xv(i,j)=Xstd(r,j); }
      auto logits_v = model.forward(Xv);
      auto nll_for_T = [&](double T){
        Tensor2D L = logits_v;
        for (double &u : L.data) u /= T;
        auto pv = softmax(L);
        return cross_entropy(pv, yv);
      };
      double bestT = 1.0, bestNLL = nll_for_T(1.0);
      for (double T=0.5; T<=5.0 + 1e-9; T += 0.05) {
        double nll = nll_for_T(T);
        if (nll < bestNLL) { bestNLL = nll; bestT = T; }
      }
      model.temperature = bestT;
      std::cout << "Calibrated temperature T=" << bestT << " (val NLL=" << bestNLL << ")\n";
    }

  } else {
    // Predict
    auto pred_rows = read_csv_numeric(predict_path, ',');
    int in_dim_pred = (int)pred_rows[0].size();
    Tensor2D Xp((int)pred_rows.size(), in_dim_pred);
    for (int i=0;i<(int)pred_rows.size();++i)
      for (int j=0;j<in_dim_pred;++j) Xp(i,j)=pred_rows[i][j];
    if (bundle_has_scaler) { Xp = bundle_scaler.transform(Xp); }
    if (quantize_runtime) enable_runtime_quantization(model);
    auto logits = model.forward(Xp);
    for (double &u : logits.data) u /= model.temperature;
    auto probs = softmax(logits);
    std::vector<int> pred(probs.rows);
    for (int i=0;i<probs.rows;++i){int best=0; double bv=probs(i,0); for (int j=1;j<probs.cols;++j) if(probs(i,j)>bv){bv=probs(i,j); best=j;} pred[i]=best;}
    if (!predict_out.empty()) {
      std::ofstream out(predict_out);
      for (size_t i=0;i<pred.size();++i) out << pred[i] << "\n";
      std::cout << "Wrote predictions to " << predict_out << "\n";
    } else {
      for (size_t i=0;i<pred.size();++i) std::cout << pred[i] << "\n";
    }
  }
  return 0;
}
