
#include "action/dml.hpp"

#include <boost/algorithm/string/join.hpp>
#include <fmt/format.h>
#include <rfl.hpp>

using namespace metadata;
using namespace action;

namespace {

std::string generate_value(metadata::Column const &col, ps_random &rand) {
  switch (col.type) {
  case metadata::ColumnType::INT:
    return std::to_string(rand.random_number(1, 1000000));
  case metadata::ColumnType::REAL:
    return std::to_string(rand.random_number(1.0, 1000000.0));
  case metadata::ColumnType::VARCHAR:
    return std::string("'") + rand.random_string(0, col.length) + "'";
  case metadata::ColumnType::BYTEA:
  case metadata::ColumnType::TEXT:
    return std::string("'") + rand.random_string(50, 1000) + "'";
  case metadata::ColumnType::BOOL:
    return rand.random_number(0, 1) == 1 ? "true" : "false";
  case metadata::ColumnType::CHAR:
    return std::string("'") + rand.random_string(0, col.length) + "'";
  }
}
}; // namespace

InsertData::InsertData(DmlConfig const &config, std::size_t rows)
    : config(config), table(nullptr), rows(rows) {}

InsertData::InsertData(DmlConfig const &config,
                       metadata::table_cptr const &table, std::size_t rows)
    : config(config), table(table), rows(rows) {}

void InsertData::execute(Metadata &metaCtx, ps_random &rand,
                         sql_variant::LoggedSQL *connection) const {

  auto table = this->table;

  // TODO : if no pointer given, select a random table
  if (metaCtx.size() == 0)
    return; // TODO: log
  while (table == NULL) {
    // select a random table from metadata
    std::size_t idx = rand.random_number<std::size_t>(0, metaCtx.size());
    table = metaCtx[idx];
  }

  std::stringstream sql;
  sql << "INSERT INTO ";
  sql << table->name;
  sql << " (";

  bool first = true;
  for (auto const &f : table->columns) {
    if (!f.auto_increment) {
      if (!first)
        sql << ", ";
      sql << f.name;
      first = false;
    }
  }

  sql << " ) VALUES ";
  for (std::size_t idx = 0; idx < rows; ++idx) {
    if (idx != 0)
      sql << ", ";
    sql << "(";

    first = true;
    for (auto const &f : table->columns) {
      if (!f.auto_increment) {
        if (!first)
          sql << ", ";
        sql << generate_value(f, rand);
        first = false;
      }
    }

    sql << ")";
  }

  sql << ";";

  connection->executeQuery(sql.str()).maybeThrow();
}
