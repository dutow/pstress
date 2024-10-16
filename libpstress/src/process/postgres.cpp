
#include "process/postgres.hpp"

#include <boost/process.hpp>
#include <boost/process/v1/io.hpp>
#include <fstream>
#include <memory>
#include <spdlog/spdlog.h>

namespace {

// pg_ctl ends, but keeps the output open which causes all kinds of issues if we
// try to keep everything in a single thread because of blocking reads as a
// workaround, outsource the output logging into its own thread we also have to
// make sure that the variables survive as long as the thread, so this uses a
// static make_shared constructor and some additional hacks, as the shared_ptr
// is constructed after the constructor finishes (needs 2 methods), and
// sometimes the thread object would be joined by the thread itself (needs
// detach)
struct CommandRunner : public std::enable_shared_from_this<CommandRunner> {
  boost::process::ipstream outerr;
  boost::process::child child;
  std::string commandLine;
  std::jthread logger;

private:
  template <typename... Ts>
  CommandRunner(Ts... args)
      : outerr(),
        child(std::string(args)..., boost::process::std_in.close(),
              (boost::process::std_out & boost::process::std_err) > outerr),
        commandLine(
            boost::algorithm::join(std::vector<std::string>{args...}, " ")) {}

  int finishRun() {
    logger = std::jthread([&]() {
      auto self_ptr = shared_from_this();

      spdlog::info("Executing command {}", commandLine);
      std::string line;

      while (child.running() && std::getline(outerr, line) && !line.empty()) {
        spdlog::info(">> {}", line);
      }
    });

    logger.detach();

    try {
      // TODO: how to solve this properly? If a process is too quick, we get an
      // exception here
      child.wait();
    } catch (...) {
    }

    return child.exit_code();
  }

public:
  template <typename... Ts> static int execute(Ts... args) {
    std::shared_ptr<CommandRunner> ptr(new CommandRunner(args...));
    return ptr->finishRun();
  }
};

int runPgCtl(std::string const &installDir, std::string const &dataDir,
             std::string const &subCommand) {
  boost::process::ipstream outerr;
  return CommandRunner::execute(std::format("{}/bin/pg_ctl", installDir), "-D",
                                dataDir, subCommand);
}
} // namespace

namespace process {

Postgres::Postgres(bool initDatadir, std::string const &installDir,
                   std::string const &dataDir)

    : installDir(installDir), dataDir(dataDir) {
  if (!std::filesystem::is_directory(this->installDir)) {
    throw std::runtime_error(std::format(
        "Specified install directory '{}' is not a directory.", installDir));
  }
  if (initDatadir) {
    if (std::filesystem::exists(this->dataDir)) {
      throw std::runtime_error(std::format(
          "Data directory '{}' already exists, can't initialize.", dataDir));
    }

    boost::process::ipstream outerr;
    int result = boost::process::system(
        std::format("{}/bin/initdb", installDir), "-D", dataDir,
        (boost::process::std_out & boost::process::std_err) > outerr);

    for (std::string line; std::getline(outerr, line);) {
      spdlog::info("pg initdb: {}", line);
    }

    if (result != 0) {
      throw std::runtime_error(std::format(
          "Initdb failed with data directory '{}' and install dir {}.", dataDir,
          installDir));
    }
  } else {
    if (!std::filesystem::is_directory(dataDir)) {
      throw std::runtime_error(std::format(
          "Specified data directory '{}' is not a directory.", installDir));
    }
  }
}

bool Postgres::start() { return runPgCtl(installDir, dataDir, "start") == 0; }

bool Postgres::restart() {
  return runPgCtl(installDir, dataDir, "restart") == 0;
}

void Postgres::stop() {
  int result = runPgCtl(installDir, dataDir, "stop");
  if (result != 0) {
    throw std::runtime_error(std::format(
        "Pg_ctl stop failed with data directory '{}' and install dir {}.",
        dataDir.string(), installDir.string()));
  }
}

void Postgres::kill9() {
  // TODO
}

void Postgres::add_config(params_t additionalConfig) {
  std::ofstream stream(std::format("{}/postgresql.conf", dataDir.string()),
                       std::ios_base::app);
  for (auto const &conf : additionalConfig) {
    stream << std::format("{} = {}", conf.first, conf.second) << std::endl;
  }
}

bool Postgres::createdb(std::string const &name) {
  return CommandRunner::execute(
      std::format("{}/bin/createdb", installDir.string()), name);
}

bool Postgres::dropdb(std::string const &name) {
  return CommandRunner::execute(
      std::format("{}/bin/dropdb", installDir.string()), name);
}

} // namespace process
