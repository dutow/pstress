
#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

namespace process {

class Postgres {
public:
  using params_t = std::unordered_map<std::string, std::string>;

  Postgres(bool initDatadir, std::string const &installDir,
           std::string const &dataDir);

  void add_config(params_t additionalConfig);

  bool start();

  bool restart();

  void stop();

  void kill9();

  bool createdb(std::string const &name);

  bool dropdb(std::string const &name);

private:
  std::filesystem::path installDir;
  std::filesystem::path dataDir;
};

} // namespace process
