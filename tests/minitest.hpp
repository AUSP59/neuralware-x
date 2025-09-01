// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace minitest {

struct TestRegistry {
  std::vector<std::pair<std::string, std::function<void()>>> tests;
  static TestRegistry& instance() { static TestRegistry r; return r; }
  void add(std::string name, std::function<void()> fn) { tests.emplace_back(std::move(name), std::move(fn)); }
};

struct Registrar {
  Registrar(const std::string& name, std::function<void()> fn) { TestRegistry::instance().add(name, fn); }
};

#define TEST(name)   static void name();   static ::minitest::Registrar registrar_##name(#name, name);   static void name()

#define EXPECT_TRUE(cond) do { if (!(cond)) { std::cerr << __FILE__ << ":" << __LINE__ << " EXPECT_TRUE failed: " #cond "\n"; std::exit(1); } } while(0)
#define EXPECT_LT(a,b) do { if (!((a)<(b))) { std::cerr << __FILE__ << ":" << __LINE__ << " EXPECT_LT failed: " #a " < " #b "\n"; std::exit(1); } } while(0)
#define EXPECT_GE(a,b) do { if (!((a)>=(b))) { std::cerr << __FILE__ << ":" << __LINE__ << " EXPECT_GE failed: " #a " >= " #b "\n"; std::exit(1); } } while(0)

inline int run_all() {
  int passed = 0;
  for (auto& [name, fn] : TestRegistry::instance().tests) {
    try {
      fn();
      ++passed;
    } catch (const std::exception& e) {
      std::cerr << "Test " << name << " threw exception: " << e.what() << "\n";
      return 1;
    } catch (...) {
      std::cerr << "Test " << name << " threw unknown exception\n";
      return 1;
    }
  }
  std::cout << "minitest: " << passed << " tests passed.\n";
  return 0;
}

} // namespace minitest

int main() { return ::minitest::run_all(); }
