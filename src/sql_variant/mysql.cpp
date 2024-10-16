
#include "sql_variant/mysql.hpp"

#include <mutex>
#include <mysql.h>
#include <sstream>

#include "common.hpp"

namespace {
struct MySQLSpecificResult : sql_variant::QuerySpecificResult {
  MYSQL_RES *res;
  std::size_t num_fields;

  MySQLSpecificResult(MYSQL_RES *res)
      : res(res), num_fields(res == nullptr ? 0 : mysql_num_fields(res)) {}

  ~MySQLSpecificResult() override {
    if (res != nullptr)
      mysql_free_result(res);
  }

  std::size_t numFields() const override { return num_fields; }

  std::size_t numRows() const override {
    return res == nullptr ? 0 : mysql_num_rows(res);
  }

  sql_variant::RowView nextRow() const override {
    if (res == nullptr) {
      throw sql_variant::SqlException("Not a SELECT-like statement!");
    }

    sql_variant::RowView ret;
    const auto mdata = mysql_fetch_row(res);
    const auto lengths = mysql_fetch_lengths(res);

    if (mdata == nullptr) {
      throw sql_variant::SqlException("No more rows");
    }

    ret.rowData.resize(num_fields);

    for (std::size_t i = 0; i < num_fields; ++i) {
      if (mdata[i] != nullptr) {
        ret.rowData[i] = std::string_view(mdata[i], lengths[i]);
      }
    }

    return ret;
  }
};
} // namespace

namespace sql_variant {

QuerySpecificResult::~QuerySpecificResult() {}

MySQL::MySQL(ServerParams const &params) {
  {
    // mysql_connect is not thread safe, hold a mutex

    static std::mutex mysql_init_mutex;
    std::lock_guard<std::mutex> guard(mysql_init_mutex);

    connection = mysql_init(nullptr);
  }

  if (params.maxpacket != MAX_PACKET_DEFAULT) {
    mysql_options(connection, MYSQL_OPT_MAX_ALLOWED_PACKET, &params.maxpacket);
  }
  if (mysql_real_connect(connection, params.address.c_str(),
                         params.username.c_str(), params.password.c_str(),
                         params.database.c_str(), params.port,
                         params.socket.c_str(), 0) == NULL) {

    std::stringstream ss;
    logError(ss);

    mysql_close(connection);
    mysql_thread_end();
    throw SqlException(ss.str());
  }

  if (connection == nullptr) {
    throw SqlException("mysql_init failed");
  }
}

MySQL::~MySQL() {
  if (connection != nullptr) {
    mysql_close(connection);
    mysql_thread_end();
    connection = nullptr;
  }
}

std::uint64_t MySQL::getAffectedRows() const {
  if (mysql_affected_rows(connection) == ~(unsigned long long)0) {
    return 0LL;
  }
  return mysql_affected_rows(connection);
}

void MySQL::logError(std::ostream &ostream) const {
  ostream << mysql_errno(connection) << ": " << mysql_error(connection);
}

QueryResult MySQL::executeQuery(std::string const &query) const {
  QueryResult result;

  result.executedAt = std::chrono::high_resolution_clock::now();
  const auto qres = mysql_real_query(connection, query.c_str(), query.size());
  const auto end = std::chrono::high_resolution_clock::now();

  result.executionTime = end - result.executedAt;

  result.errorInfo.errorCode = mysql_errno(connection);
  result.errorInfo.errorMessage = mysql_error(connection);

  if (qres == 1) { // failure

    result.errorInfo.errorStatus =
        result.errorInfo.errorCode ==
                (CR_SERVER_GONE_ERROR ||
                 result.errorInfo.errorCode == CR_SERVER_LOST)
            ? SqlStatus::serverGone
            : SqlStatus::error;
  } else { // success
    result.errorInfo.errorStatus = SqlStatus::success;

    result.data =
        std::make_unique<MySQLSpecificResult>(mysql_store_result(connection));
    result.affectedRows = mysql_affected_rows(connection);
  }

  return result;
}

} // namespace sql_variant