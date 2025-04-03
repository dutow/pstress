
#pragma once

#include "process/process.hpp"
#include "sql_variant/generic.hpp"
#include <filesystem>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace process {

class Postgres {
public:
  using params_t = std::unordered_map<std::string, std::string>;

  Postgres(bool initDatadir, std::string const &logname,
           std::string const &installDir, std::string const &dataDir);

  Postgres(std::string const &logname, std::string const &installDir,
           std::string const &dataDir, sql_variant::ServerParams const &from,
           std::vector<std::string> additionalParams);

  void add_config(params_t additionalConfig);

  void add_hba(std::string const &host, std::string const &database,
               std::string const &user, std::string const &address,
               std::string const &method);

  bool start();

  bool restart(std::size_t graceful_wait_period);

  void stop(std::size_t graceful_wait_period);

  void kill9();

  bool createdb(std::string const &name);

  bool dropdb(std::string const &name);

  bool is_running();

  bool is_ready();

  bool wait_ready(std::size_t maxWaitTime);

private:
  std::filesystem::path installDir;
  std::filesystem::path dataDir;
  std::string port;

  std::shared_ptr<spdlog::logger> logger;

  BackgroundProcess::ptr_t postmaster = nullptr;
};

} // namespace process
