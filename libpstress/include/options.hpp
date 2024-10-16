
#pragma once

#include <CLI/CLI.hpp>
#include <memory>
#include <vector>

namespace mysql {
enum class alter_algorithm { default_, inplace, copy, instant };
enum class alter_lock { default_, none, shared, exclusive };
} // namespace mysql

struct options_holder {
  std::string flavor = "mysql";

  bool pquery_mode = false;
  int initial_seed = 0;
  bool just_load_ddl = false;

  bool no_ddl_workload = false;
  int seconds = 0;

  struct metadata_opts {
    unsigned max_indexes = 7;
    unsigned max_columns = 10;
    unsigned initial_tables = 10;
    unsigned max_index_columns = 10;

    bool encryption = true;
    bool virtual_columns = true;
    bool blob_columns = true;
    bool autoinc = true;
    bool desc_index = true;
    bool partitioned_tables = true;
    bool temporary_tables = true;
    bool normal_tables = true;
    unsigned normal_to_temporary_ratio = 10;
    unsigned initial_records = 1000;
    bool exact_initial_records = false;

    struct mysql_mopts {
      unsigned number_of_general_tablespaces = 1;
      unsigned number_of_undo_tablespaces = 3;
      std::string engine = "innodb";

      bool table_compression = true;
      bool column_compression = true;

      bool tablespaces = true;

      bool operator==(const mysql_mopts &) const = default;
    } mysql;

    bool operator==(const metadata_opts &) const = default;

  } metadata;

  struct probability_opts {

    unsigned alter_table_encrypt = 10;

    bool operator==(const probability_opts &) const = default;
  } probabilities;

  struct mysql_opts {

    bool undo_sql = true;

    std::vector<mysql::alter_algorithm> alter_algorithms = {
        mysql::alter_algorithm::default_, mysql::alter_algorithm::inplace,
        mysql::alter_algorithm::copy, mysql::alter_algorithm::instant};

    std::vector<mysql::alter_lock> alter_locks = {
        mysql::alter_lock::default_, mysql::alter_lock::none,
        mysql::alter_lock::shared, mysql::alter_lock::exclusive};

    bool operator==(const mysql_opts &) const = default;
  } mysql;

  bool operator==(const options_holder &) const = default;
};

std::unique_ptr<CLI::App> setup_options(options_holder &data);