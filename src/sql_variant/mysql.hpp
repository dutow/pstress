
#pragma once

#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct MYSQL;

namespace sql_variant {

struct ServerParams {
  std::string database;
  std::string address;
  std::string socket;
  std::string username;
  std::string password;

  std::uint64_t maxpacket;
  std::uint16_t port;
};

class SqlException : public std::exception {
public:
  SqlException(std::string const &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};

enum class SqlStatus { success, error, serverGone };

struct ErrorInfo {
  std::uint64_t errorCode;
  std::string errorMessage;
  SqlStatus errorStatus;

  bool success() const { return errorStatus == SqlStatus::success; }
  bool serverGone() const { return errorStatus == SqlStatus::serverGone; }
};

struct RowView {
  std::vector<std::optional<std::string_view>> rowData;
};

struct QuerySpecificResult {
  virtual ~QuerySpecificResult();

  virtual std::size_t numFields() const = 0;
  virtual std::size_t numRows() const = 0;

  virtual RowView nextRow() const = 0;
};

struct QueryResult {
  std::string query;
  std::chrono::high_resolution_clock::time_point executedAt;
  std::chrono::nanoseconds executionTime;
  ErrorInfo errorInfo;
  std::uint64_t affectedRows = 0;

  std::unique_ptr<QuerySpecificResult> data;

  explicit operator bool() const { return errorInfo.success(); }

  bool success() const { return errorInfo.success(); }
};

class MySQL {
public:
  MySQL(ServerParams const &params);
  ~MySQL();

  MySQL(MySQL const &) = delete;
  MySQL &operator=(MySQL const &) = delete;

  MySQL(MySQL &&) noexcept = default;
  MySQL &operator=(MySQL &&) noexcept = default;

  void logError(std::ostream &ostream) const;

  QueryResult executeQuery(std::string const &query) const;

  std::uint64_t getAffectedRows() const;

  std::string serverInfo() const;

  std::string hostInfo() const;

  std::optional<std::string_view>
  querySingleValue(const std::string &sql) const;

  static void library_end();

private:
  MYSQL *connection;
};
} // namespace sql_variant