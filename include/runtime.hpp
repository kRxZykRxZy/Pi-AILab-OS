#pragma once
#include "gguf.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <vector>
namespace piai::runtime {
struct Stats { uint64_t requests=0, generations=0, tokens=0; };
class Runtime {
 mutable std::mutex mu_; std::shared_ptr<gguf::Model> model_; std::string model_path_; Stats stats_{};
 public:
  bool load(const std::string& path);
  void unload();
  bool loaded() const;
  std::string model_info() const;
  std::string generate(const std::string& prompt, size_t max_tokens);
  Stats stats() const;
};
Runtime& instance();
}
