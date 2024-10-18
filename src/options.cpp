

#include "options.hpp"

namespace {
class MyFormatter : public CLI::Formatter {
  std::string make_option_name(const CLI::Option *opt, bool) const override {
    // TODO: remove things between { ... }
    return opt->get_name(false, true);
    //
  }
};

} // namespace

std::unique_ptr<CLI::App> setup_options(options_holder &data) {
  auto app = std::make_unique<CLI::App>("App description");

  app->formatter(std::make_shared<MyFormatter>());

  ///////////////////////////////////////////////////////////////////////////////
  // Connection options

  app->add_flag("--mysql{mysql}", data.flavor,
                "Connect to a MySQL server (default)");
  app->add_flag("--postgres{postgres}", data.flavor,
                "Connect to a PostgreSQL server")
      ->excludes("--mysql");

  ///////////////////////////////////////////////////////////////////////////////
  // Execution control options

  app->add_flag(
         "--pquery", data.pquery_mode,
         "run pstress as pquery 2.0. sqls will be executed from --infine "
         "in some order based on shuffle. basically it will run in "
         "pquery mode you can also use -k")
      ->group("Execution");

  app->add_option("--seed", data.initial_seed,
                  "Initial seed used for the test, 0 = random")
      ->default_val(data.initial_seed)
      ->group("Execution");

  app->add_flag("--jlddl", data.just_load_ddl, "Load DDL and exit")
      ->group("Execution")
      ->configurable(false);

  ///////////////////////////////////////////////////////////////////////////////
  // Initial data setup options

  app->add_flag("--encryption,!--no-encryption", data.encryption,
                "Encryption testing")
      ->group("Data setup")
      ->default_val(data.encryption);

  app->add_flag("--virtual,!--no-virtual", data.virtual_columns,
                "Enable/disable virtual columns")
      ->group("Data setup")
      ->default_val(data.virtual_columns);

  app->add_flag("--blob,!--no-blob", data.blob_columns,
                "Enable/disable blob columns")
      ->group("Data setup")
      ->default_val(data.blob_columns);

  app->add_option("--tables", data.initial_tables, "Number of initial tables")
      ->default_val(data.initial_tables)
      ->group("Data setup");

  app->add_option("--indexes", data.max_indexes,
                  "Number of maximum indexes per table")
      ->default_val(data.max_indexes)
      ->group("Data setup");

  app->add_option("--columns", data.max_columns,
                  "Number of maximum columns  per table")
      ->default_val(data.max_columns)
      ->group("Data setup");

  app->add_option("--index-columns", data.max_index_columns,
                  "Number of maximum columns per index")
      ->default_val(data.max_index_columns)
      ->group("Data setup");

  app->add_flag("--autoinc,!--no-autoinc", data.use_autoinc,
                "Enable/disable auto increment/serial columns")
      ->group("Data setup")
      ->default_val(data.use_autoinc);

  app->add_flag("--desc-index,!--no-desc-index", data.use_desc_index,
                "Enable/disable DESC indexes")
      ->group("Data setup")
      ->default_val(data.use_desc_index);

  app->add_flag("--temporary-tables,!--no-temporary-tables",
                data.temporary_tables, "Enable/disable temporary tables")
      ->group("Data setup")
      ->default_val(data.temporary_tables);

  app->add_flag("--partition-tables,!--no-partition-tables",
                data.partitioned_tables, "Enable/disable partitioned tables")
      ->group("Data setup")
      ->default_val(data.partitioned_tables);

  app->add_flag("--normal-tables,!--no-normal-tables", data.normal_tables,
                "Enable/disable normal tables")
      ->group("Data setup")
      ->default_val(data.normal_tables);

  app->add_flag_function(
         "--only-partition-tables",
         [&](auto const &) {
           data.normal_tables = false;
           data.temporary_tables = false;
           data.partitioned_tables = true;
         },
         "Only use partitioned tables")
      ->group("Data setup")
      ->excludes("--temporary-tables")
      ->excludes("--partition-tables")
      ->excludes("--normal-tables")
      ->configurable(false);

  app->add_flag_function(
         "--only-temporary-tables",
         [&](auto const &) {
           data.normal_tables = false;
           data.temporary_tables = true;
           data.partitioned_tables = false;
         },
         "Only use partitioned tables")
      ->group("Data setup")
      ->excludes("--temporary-tables")
      ->excludes("--partition-tables")
      ->excludes("--normal-tables")
      ->excludes("--only-partition-tables")
      ->configurable(false);

  app->add_flag_function(
         "--only-normal-tables",
         [&](auto const &) {
           data.normal_tables = true;
           data.temporary_tables = false;
           data.partitioned_tables = false;
         },
         "Only use partitioned tables")
      ->group("Data setup")
      ->excludes("--temporary-tables")
      ->excludes("--partition-tables")
      ->excludes("--normal-tables")
      ->excludes("--only-partition-tables")
      ->excludes("--only-temporary-tables")
      ->configurable(false);

  app->add_option("--ratio-normal-temp", data.normal_to_temporary_ratio,
                  "ratio of normal to temporary tables. for e.g. if ratio to "
                  "normal table to temporary is 10 . --tables 40. them only 4 "
                  "temorary table will be created per session")
      ->default_val(data.normal_to_temporary_ratio)
      ->group("Data setup");

  app->add_option(
         "--records", data.initial_records,
         "Maximum number of initial records (N) in each table. The table will "
         "have random records in range of 0 to N. Also check "
         "--exact-initial-records")
      ->default_val(data.initial_records)
      ->group("Data setup");

  // TODO: add minimum-records

  app->add_flag("--exact-records", data.exact_initial_records,
                "Imsert exactly --records number of records in the tables.")
      ->group("Data setup")
      ->default_val(data.exact_initial_records);

  ///////////////////////////////////////////////////////////////////////////////
  // Workload SQL options

  app->add_flag("--no-ddl", data.no_ddl_workload, "Do not use DDL in workload")
      ->group("Workload");

  app->add_option("--seconds", data.seconds,
                  "Number of seconds to execute workload")
      ->default_val(data.seconds)
      ->group("Workload");

  ///////////////////////////////////////////////////////////////////////////////
  // Workload Probability options

  // TODO: mysql specific
  app->add_option("--alter-table-encrypt",
                  data.probabilities.alter_table_encrypt,
                  "ALTER TABLE SET ENCRYPTION")
      ->default_val(data.probabilities.alter_table_encrypt)
      ->group("Workload Probabilities");

  ///////////////////////////////////////////////////////////////////////////////
  // MySQL specific options

  app->add_option("--tbs-count", data.mysql.number_of_general_tablespaces,
                  "Random number of different general tablespaces")
      ->group("MySQL");

  app->add_option("--undo-tbs-count", data.mysql.number_of_undo_tablespaces,
                  "Number of default undo tablespaces")
      ->group("MySQL");

  app->add_option("--engine", data.mysql.engine, "Engine used")->group("MySQL");

  app->add_option("--no-table-compression", data.mysql.no_table_compression,
                  "Disable table compression")
      ->group("MySQL");

  app->add_option("--no-column-compression", data.mysql.no_column_compression,
                  "Disable column compression")
      ->group("MySQL");

  app->add_option("--undo-tbs-sql", data.mysql.undo_sql,
                  "Enable/disable testing create/alter/drop undo tablespace")
      ->group("MySQL")
      ->default_val(data.mysql.undo_sql);

  app->add_flag("--tablespaces,!--no-tablespaces", data.mysql.test_tablespaces,
                "Disable all type of tablespace including the general "
                "tablespace (not undo)")
      ->group("MySQL")
      ->default_val(data.mysql.test_tablespaces);

  std::vector<std::pair<std::string, mysql::alter_algorithm>>
      alter_algorithm_map{{"default", mysql::alter_algorithm::default_},
                          {"inplace", mysql::alter_algorithm::inplace},
                          {"copy", mysql::alter_algorithm::copy},
                          {"instant", mysql::alter_algorithm::instant}};

  app->add_option("--alter-algorithm", data.mysql.alter_algorithms,
                  "ALTER ALGORITHMs to use (default inplace copy instant)")
      ->group("MySQL")
      ->default_val(data.mysql.alter_algorithms)
      ->transform(
          CLI::CheckedTransformer(alter_algorithm_map, CLI::ignore_case));

  std::vector<std::pair<std::string, mysql::alter_lock>> alter_lock_map{
      {"default", mysql::alter_lock::default_},
      {"none", mysql::alter_lock::none},
      {"shared", mysql::alter_lock::shared},
      {"exclusive", mysql::alter_lock::exclusive}};

  app->add_option("--alter-lock", data.mysql.alter_locks,
                  "ALTER LOCKs to use (default none shared exclusive)")
      ->group("MySQL")
      ->default_val(data.mysql.alter_locks)
      ->transform(CLI::CheckedTransformer(alter_lock_map, CLI::ignore_case));

  return app;
}