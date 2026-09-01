#include "runtime.hpp"
#include <algorithm>
#include <sstream>
namespace piai::runtime {
bool Runtime::load(const std::string& path){auto m=std::make_shared<gguf::Model>();if(!m->open(path))return false;std::lock_guard<std::mutex>g(mu_);model_=std::move(m);model_path_=path;return true;}
void Runtime::unload(){std::lock_guard<std::mutex>g(mu_);model_.reset();model_path_.clear();}
bool Runtime::loaded()const{std::lock_guard<std::mutex>g(mu_);return !!model_;}
std::string Runtime::model_info()const{std::lock_guard<std::mutex>g(mu_);if(!model_)return "{\"loaded\":false}";std::ostringstream o;o<<"{\"loaded\":true,\"version\":"<<model_->version()<<",\"tensors\":"<<model_->tensors().size()<<"}";return o.str();}
std::string Runtime::generate(const std::string& prompt,size_t max_tokens){std::lock_guard<std::mutex>g(mu_);++stats_.requests;if(!model_)return "";++stats_.generations;std::string out=prompt.substr(0,std::min(prompt.size(),size_t(4096)));(void)max_tokens;return out;}
Stats Runtime::stats()const{std::lock_guard<std::mutex>g(mu_);return stats_;}
Runtime& instance(){static Runtime r;return r;}
}
