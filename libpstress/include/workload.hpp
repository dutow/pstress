
#pragma once

#include <thread>

#include "action/action_registry.hpp"
#include "metadata.hpp"
#include "sql_variant/generic.hpp"

using logged_sql_ptr = std::unique_ptr<sql_variant::LoggedSQL>;

using metadata_ptr = std::shared_ptr<metadata::Metadata>;

struct WorkloadParams {
  std::size_t duration_in_seconds;
  std::size_t repeat_times;
  std::size_t number_of_workers;
};

class Worker {
public:
  Worker(std::string const &name, logged_sql_ptr sql_conn,
         action::AllConfig config, metadata_ptr metadata);

  Worker(Worker &&) = default;

  virtual ~Worker();

  void create_random_tables(std::size_t count);

  void generate_initial_data();

  sql_variant::LoggedSQL *sql_connection() const;

  void reconnect();

protected:
  std::string name;
  logged_sql_ptr sql_conn;
  action::AllConfig config;
  metadata_ptr metadata;
  ps_random rand;
  std::shared_ptr<spdlog::logger> logger;
};

class RandomWorker : public Worker {
public:
  RandomWorker(std::string const &name, logged_sql_ptr sql_conn,
               action::AllConfig const &config, metadata_ptr metadata,
               action::ActionRegistry const &actions);

  RandomWorker(RandomWorker &&) = default;

  ~RandomWorker() override;

  void run_thread(std::size_t duration_in_seconds);

  void join();

  action::ActionRegistry &possibleActions();

protected:
  action::ActionRegistry actions;
  std::thread thread;
  std::size_t successfulActions = 0;
  std::size_t failedActions = 0;
};

class SqlFactory {
public:
  using on_connect_t = std::function<void(sql_variant::LoggedSQL const &)>;

  SqlFactory(sql_variant::ServerParams const &sql_params,
             on_connect_t connection_callback);

  std::unique_ptr<sql_variant::LoggedSQL>
  connect(std::string const &connection_name) const;

  sql_variant::ServerParams const &params() const;

private:
  // postgres / mysql selector
  sql_variant::ServerParams sql_params;
  on_connect_t connection_callback;
};

class Workload {
public:
  Workload(WorkloadParams const &params, SqlFactory const &sql_factory,
           action::AllConfig const &default_config, metadata_ptr metadata,
           action::ActionRegistry const &actions);

  void run();

  void wait_completion();

  // indexes starting from 1, as that's expected from lua
  RandomWorker &worker(std::size_t idx);

  std::size_t worker_count() const;

  void reconnect_workers();

private:
  std::size_t duration_in_seconds;
  std::size_t repeat_times;
  std::vector<RandomWorker> workers;
  action::ActionRegistry actions;
};

class Node {
public:
  Node(SqlFactory const &sql_factory);

  std::shared_ptr<Workload> init_random_workload(WorkloadParams const &wp);

  std::unique_ptr<Worker> make_worker(std::string const &name);

  action::ActionRegistry &possibleActions();

  sql_variant::ServerParams const &sql_params() const;

private:
  SqlFactory sql_factory;

  action::AllConfig default_config;
  metadata_ptr metadata;
  action::ActionRegistry actions = action::default_registy();
};
