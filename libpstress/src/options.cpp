

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

  static const constexpr auto gr_execution = "Execution";
  static const constexpr auto gr_metadata = "Data setup";
  static const constexpr auto gr_workload = "Workload configuration";
  static const constexpr auto gr_probabilities =
      "Workload probabilities (weights)";
  static const constexpr auto gr_mysql_metadata = "MySQL Data setup";
  static const constexpr auto gr_mysql_workload =
      "MySQL Workload configuration";
  // static const constexpr auto gr_mysql_probabilities =
  //"MySQL Workload probabilities (weights)";

  app->add_flag(
         "--pquery", data.pquery_mode,
         "run pstress as pquery 2.0. sqls will be executed from --infine "
         "in some order based on shuffle. basically it will run in "
         "pquery mode you can also use -k")
      ->group(gr_execution);

  app->add_option("--seed", data.initial_seed,
                  "Initial seed used for the test, 0 = random")
      ->default_val(data.initial_seed)
      ->group(gr_execution);

  app->add_flag("--jlddl", data.just_load_ddl, "Load DDL and exit")
      ->group(gr_execution)
      ->configurable(false);

  ///////////////////////////////////////////////////////////////////////////////
  // Initial data setup options

  app->add_flag("--encryption,!--no-encryption", data.metadata.encryption,
                "Encryption testing")
      ->group(gr_metadata)
      ->default_val(data.metadata.encryption);

  app->add_flag("--virtual,!--no-virtual", data.metadata.virtual_columns,
                "Enable/disable virtual columns")
      ->group(gr_metadata)
      ->default_val(data.metadata.virtual_columns);

  app->add_flag("--blob,!--no-blob", data.metadata.blob_columns,
                "Enable/disable blob columns")
      ->group(gr_metadata)
      ->default_val(data.metadata.blob_columns);

  app->add_option("--tables", data.metadata.initial_tables,
                  "Number of initial tables")
      ->default_val(data.metadata.initial_tables)
      ->group(gr_metadata);

  app->add_option("--indexes", data.metadata.max_indexes,
                  "Number of maximum indexes per table")
      ->default_val(data.metadata.max_indexes)
      ->group(gr_metadata);

  app->add_option("--columns", data.metadata.max_columns,
                  "Number of maximum columns  per table")
      ->default_val(data.metadata.max_columns)
      ->group(gr_metadata);

  app->add_option("--index-columns", data.metadata.max_index_columns,
                  "Number of maximum columns per index")
      ->default_val(data.metadata.max_index_columns)
      ->group(gr_metadata);

  app->add_flag("--autoinc,!--no-autoinc", data.metadata.autoinc,
                "Enable/disable auto increment/serial columns")
      ->group(gr_metadata)
      ->default_val(data.metadata.autoinc);

  app->add_flag("--desc-index,!--no-desc-index", data.metadata.desc_index,
                "Enable/disable DESC indexes")
      ->group(gr_metadata)
      ->default_val(data.metadata.desc_index);

  app->add_flag("--temporary-tables,!--no-temporary-tables",
                data.metadata.temporary_tables,
                "Enable/disable temporary tables")
      ->group(gr_metadata)
      ->default_val(data.metadata.temporary_tables);

  app->add_flag("--partition-tables,!--no-partition-tables",
                data.metadata.partitioned_tables,
                "Enable/disable partitioned tables")
      ->group(gr_metadata)
      ->default_val(data.metadata.partitioned_tables);

  app->add_flag("--normal-tables,!--no-normal-tables",
                data.metadata.normal_tables, "Enable/disable normal tables")
      ->group(gr_metadata)
      ->default_val(data.metadata.normal_tables);

  app->add_flag_function(
         "--only-partition-tables",
         [&](auto const &) {
           data.metadata.normal_tables = false;
           data.metadata.temporary_tables = false;
           data.metadata.partitioned_tables = true;
         },
         "Only use partitioned tables")
      ->group(gr_metadata)
      ->excludes("--temporary-tables")
      ->excludes("--partition-tables")
      ->excludes("--normal-tables")
      ->configurable(false);

  app->add_flag_function(
         "--only-temporary-tables",
         [&](auto const &) {
           data.metadata.normal_tables = false;
           data.metadata.temporary_tables = true;
           data.metadata.partitioned_tables = false;
         },
         "Only use partitioned tables")
      ->group(gr_metadata)
      ->excludes("--temporary-tables")
      ->excludes("--partition-tables")
      ->excludes("--normal-tables")
      ->excludes("--only-partition-tables")
      ->configurable(false);

  app->add_flag_function(
         "--only-normal-tables",
         [&](auto const &) {
           data.metadata.normal_tables = true;
           data.metadata.temporary_tables = false;
           data.metadata.partitioned_tables = false;
         },
         "Only use partitioned tables")
      ->group(gr_metadata)
      ->excludes("--temporary-tables")
      ->excludes("--partition-tables")
      ->excludes("--normal-tables")
      ->excludes("--only-partition-tables")
      ->excludes("--only-temporary-tables")
      ->configurable(false);

  app->add_option("--ratio-normal-temp",
                  data.metadata.normal_to_temporary_ratio,
                  "ratio of normal to temporary tables. for e.g. if ratio to "
                  "normal table to temporary is 10 . --tables 40. them only 4 "
                  "temorary table will be created per session")
      ->default_val(data.metadata.normal_to_temporary_ratio)
      ->group(gr_metadata);

  app->add_option(
         "--records", data.metadata.initial_records,
         "Maximum number of initial records (N) in each table. The table will "
         "have random records in range of 0 to N. Also check "
         "--exact-initial-records")
      ->default_val(data.metadata.initial_records)
      ->group(gr_metadata);

  // TODO: add minimum-records

  app->add_flag("--exact-records", data.metadata.exact_initial_records,
                "Imsert exactly --records number of records in the tables.")
      ->group(gr_metadata)
      ->default_val(data.metadata.exact_initial_records);

  ///////////////////////////////////////////////////////////////////////////////
  // Workload SQL options

  app->add_flag("--no-ddl", data.no_ddl_workload, "Do not use DDL in workload")
      ->group(gr_workload);

  app->add_option("--seconds", data.seconds,
                  "Number of seconds to execute workload")
      ->default_val(data.seconds)
      ->group(gr_workload);

  ///////////////////////////////////////////////////////////////////////////////
  // Workload Probability options

  // TODO: mysql specific
  app->add_option("--alter-table-encrypt",
                  data.probabilities.alter_table_encrypt,
                  "ALTER TABLE SET ENCRYPTION")
      ->default_val(data.probabilities.alter_table_encrypt)
      ->group(gr_probabilities);

  ///////////////////////////////////////////////////////////////////////////////
  // MySQL specific options

  app->add_option("--tbs-count",
                  data.metadata.mysql.number_of_general_tablespaces,
                  "Random number of different general tablespaces")
      ->group(gr_mysql_metadata)
      ->default_val(data.metadata.mysql.number_of_general_tablespaces);

  app->add_option("--undo-tbs-count",
                  data.metadata.mysql.number_of_undo_tablespaces,
                  "Number of default undo tablespaces")
      ->group(gr_mysql_metadata)
      ->default_val(data.metadata.mysql.number_of_undo_tablespaces);

  app->add_option("--engine", data.metadata.mysql.engine, "Engine used")
      ->group(gr_mysql_metadata)
      ->default_val(data.metadata.mysql.engine);

  app->add_flag("--table-compression,!--no-table-compression",
                data.metadata.mysql.table_compression,
                "Disable table compression")
      ->group(gr_mysql_metadata)
      ->default_val(data.metadata.mysql.table_compression);

  app->add_flag("--column-compression,!--no-column-compression",
                data.metadata.mysql.column_compression,
                "Disable column compression")
      ->group(gr_mysql_metadata)
      ->default_val(data.metadata.mysql.column_compression);

  app->add_option("--undo-tbs-sql", data.mysql.undo_sql,
                  "Enable/disable testing create/alter/drop undo tablespace")
      ->group(gr_mysql_workload)
      ->default_val(data.mysql.undo_sql);

  app->add_flag("--tablespaces,!--no-tablespaces",
                data.metadata.mysql.tablespaces,
                "Disable all type of tablespace including the general "
                "tablespace (but not undo)")
      ->group(gr_mysql_metadata)
      ->default_val(data.metadata.mysql.tablespaces)
      ->excludes("--tbs-count");

  std::vector<std::pair<std::string, mysql::alter_algorithm>>
      alter_algorithm_map{{"default", mysql::alter_algorithm::default_},
                          {"inplace", mysql::alter_algorithm::inplace},
                          {"copy", mysql::alter_algorithm::copy},
                          {"instant", mysql::alter_algorithm::instant}};

  app->add_option("--alter-algorithm", data.mysql.alter_algorithms,
                  "ALTER ALGORITHMs to use (default inplace copy instant)")
      ->group(gr_mysql_workload)
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
      ->group(gr_mysql_workload)
      ->default_val(data.mysql.alter_locks)
      ->transform(CLI::CheckedTransformer(alter_lock_map, CLI::ignore_case));

  return app;
}