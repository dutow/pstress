
#include "sql_variant/generic.hpp"

namespace sql_variant {

QuerySpecificResult::~QuerySpecificResult() {}

GenericSQL::~GenericSQL() {}

std::optional<std::string_view>
GenericSQL::querySingleValue(const std::string &sql) const {

  // TODO: execute_sql should be here in a heavily refactored state so we can
  // use it (for logging)
  // const auto res = execute_sql(sql, thd);

  const auto res = executeQuery(sql);

  if (!res.success() || res.data == nullptr || res.data->numFields() < 1 ||
      res.data->numRows() < 1) {
    return std::nullopt;
  }

  const auto row = res.data->nextRow();

  return row.rowData[0];
}

ServerInfo GenericSQL::serverInfo() const { return serverInfo_; }

} // namespace sql_variant