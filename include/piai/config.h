#pragma once
#include <string>
namespace piai { struct Config { std::string bind="127.0.0.1"; unsigned port=8080; std::string model_dir="/var/lib/piai/models"; }; Config load_config(const char* path); }
