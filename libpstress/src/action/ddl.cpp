
#include "action/ddl.hpp"

#include <boost/algorithm/string/join.hpp>
#include <fmt/format.h>
#include <rfl.hpp>

using namespace metadata;
using namespace action;

namespace {
ColumnType randomColumnType(ps_random &rand) {
  auto arr = rfl::get_enumerator_array<ColumnType>();
  return arr[rand.random_number(std::size_t(0), arr.size() - 1)].second;
}

std::size_t randomColumnLength(ps_random &rand, ColumnType type) {
  switch (type) {
  case ColumnType::BYTEA:
  case ColumnType::TEXT:
    return 0;
  case ColumnType::CHAR:
  case ColumnType::VARCHAR:
    return rand.random_number(1, 100);
  case ColumnType::BOOL:
  case ColumnType::INT:
  case ColumnType::REAL:
    return 0;
  }
}

Column randomColumn(ps_random &rand, bool forceSerial = false) {
  Column col;

  col.name = fmt::format("col{}", rand.random_number<std::size_t>());
  col.type = forceSerial ? ColumnType::INT : randomColumnType(rand);

  if (forceSerial) {
    col.primary_key = true;
    col.auto_increment = true;
  } else {
    col.length = randomColumnLength(rand, col.type);
  }

  return col;
}

std::string columnDefinition(Column const &col) {
  if (col.auto_increment) {
    // assert that this is an int type
    return fmt::format("{} {}", col.name, "SERIAL");
  } else {
    std::string def =
        fmt::format("{} {}", col.name, rfl::enum_to_string(col.type));
    if (col.length > 0) {
      def += fmt::format("({})", col.length);
    }
    return def;
  }
}

} // namespace

CreateTable::CreateTable(DdlConfig const &config, Table::Type type)
    : config(config), type(type) {}

void CreateTable::execute(Metadata &metaCtx, ps_random &rand,
                          sql_variant::LoggedSQL *connection) const {
  if (metaCtx.size() >= config.max_table_count) {
    // log error and skip
    return;
  }

  metaCtx.createTable([&](Metadata::Reservation &res) {
    // 1: build definition

    auto table = res.table();

    table->name = fmt::format("foo{}", rand.random_number(1, 1000000)); // name;

    const size_t column_count =
        rand.random_number<std::size_t>(2, config.max_column_count);

    for (size_t idx = 0; idx < column_count; ++idx) {
      table->columns.push_back(randomColumn(rand, idx == 0));
    }

    if (type == Table::Type::partitioned) {
      // TODO: setup partition information
    }

    // 2: build & execute SQL statement

    std::vector<std::string> defs;
    std::vector<std::string> pk_columns;

    for (auto const &col : table->columns) {
      if (col.primary_key) {
        pk_columns.push_back(col.name);
      }
      defs.push_back(columnDefinition(col));
    }

    if (!pk_columns.empty()) {
      defs.push_back(fmt::format("PRIMARY KEY ({})",
                                 boost::algorithm::join(pk_columns, ", ")));
    }

    connection
        ->executeQuery(fmt::format("CREATE TABLE {} ({});", table->name,
                                   boost::algorithm::join(defs, ",\n")))
        .maybeThrow();

    res.complete();
  });
}

DropTable::DropTable(DdlConfig const &config) : config(config) {}

void DropTable::execute(Metadata &metaCtx, ps_random &rand,
                        sql_variant::LoggedSQL *connection) const {
  if (metaCtx.size() <= config.min_table_count) {
    // log error and skip
    return;
  }

  auto idx = rand.random_number(std::size_t(0), metaCtx.size() - 1);

  // TODO: add cascade randomly? And handle possibly other dropped tables

  metaCtx.dropTable(idx, [&](Metadata::Reservation &res) {
    if (!res.open())
      return;
    connection->executeQuery(fmt::format("DROP TABLE {};", res.table()->name))
        .maybeThrow();

    res.complete();
  });
}

AlterTable::AlterTable(DdlConfig const &config,
                       BitFlags<AlterSubcommand> const &possibleCommands)
    : config(config), possibleCommands(possibleCommands) {}

void AlterTable::execute(Metadata &metaCtx, ps_random &rand,
                         sql_variant::LoggedSQL *connection) const {
  auto idx = rand.random_number(std::size_t(0), metaCtx.size() - 1);

  metaCtx.alterTable(idx, [&](Metadata::Reservation &res) {
    if (!res.open())
      return;

    auto table = res.table();

    const auto commands = possibleCommands.All();

    const auto howManySubcommands =
        rand.random_number(std::size_t(1), config.max_alter_clauses);

    std::vector<std::string> alterSubcommands;

    std::vector<Column> newColumns;

    for (std::size_t idx = 0; idx < howManySubcommands; ++idx) {
      const auto cmdIndex =
          rand.random_number(std::size_t(0), commands.size() - 1);

      switch (commands[cmdIndex]) {
      case AlterSubcommand::addColumn: {
        const auto column = randomColumn(rand);
        alterSubcommands.emplace_back(
            fmt::format("ADD COLUMN {}", columnDefinition(column)));
        // we can't accidentally modify / drop new columns in the same statement
        newColumns.push_back(column);
      }
      case AlterSubcommand::dropColumn: {
        if (table->columns.size() < 3)
          continue;
        const auto columnIndex =
            rand.random_number(std::size_t(1), table->columns.size() - 1);
        alterSubcommands.emplace_back(
            fmt::format("DROP COLUMN {}", table->columns[columnIndex].name));
        table->columns.erase(table->columns.begin() + columnIndex);
        break;
      }
      }
    }

    table->columns.insert(table->columns.end(), newColumns.begin(),
                          newColumns.end());

    connection
        ->executeQuery(
            fmt::format("ALTER TABLE {} \n {};", res.table()->name,
                        boost::algorithm::join(alterSubcommands, ",\n")))
        .maybeThrow();

    res.complete();
  });
}
