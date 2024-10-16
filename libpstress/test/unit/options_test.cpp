
#include "options.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include <iostream>
#include <stdio.h>

TEST_CASE("Empty options", "[configuration]") {
  options_holder opts{};
  auto app = setup_options(opts);

  const char *dummy_argv[1] = {"executable-name"};
  app->parse(1, dummy_argv);

  options_holder default_opts{};

  // Equality is trivial as long as CLI11 works, but this test also ensures that
  // our configuration doesn't throw any exceptions
  REQUIRE(opts == default_opts);
}

TEST_CASE("Postgres sets the correct flavor", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[2] = {"executable-name", "--postgres"};
  app->parse(2, dummy_argv);

  // std::cout << app->config_to_str(true, true) << std::endl;
  // std::cout << "---------" << std::endl;
  // std::cout << app->help() << std::endl;

  REQUIRE(opts.flavor == "postgres");
}

TEST_CASE("Postgres excludes MySQL", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--postgres", "--mysql"};
  REQUIRE_THROWS_WITH(
      app->parse(3, dummy_argv),
      Catch::Matchers::ContainsSubstring("--mysql excludes --postgres"));
}

TEST_CASE("Multiple alter algorithm options can be specified",
          "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[5] = {"executable-name", "--mysql",
                               "--alter-algorithm", "inplace", "cOPy"};
  app->parse(5, dummy_argv);

  REQUIRE_THAT(
      opts.mysql.alter_algorithms,
      Catch::Matchers::UnorderedEquals(std::vector<mysql::alter_algorithm>{
          mysql::alter_algorithm::inplace, mysql::alter_algorithm::copy}));
}

TEST_CASE("Unknown alter algorithm options fails", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[5] = {"executable-name", "--mysql",
                               "--alter-algorithm", "inplace", "cOPpy"};
  REQUIRE_THROWS_WITH(
      app->parse(5, dummy_argv),
      Catch::Matchers::ContainsSubstring("Check cOPpy value in"));
}

TEST_CASE("Only-partition-tables disables other tables", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[5] = {"executable-name", "--only-partition-tables"};

  app->parse(2, dummy_argv);

  REQUIRE(!opts.metadata.normal_tables);
  REQUIRE(!opts.metadata.temporary_tables);
  REQUIRE(opts.metadata.partitioned_tables);
}

TEST_CASE("Only-partition-tables excludes normal-tables", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-partition-tables",
                               "--normal-tables"};
  REQUIRE_THROWS_WITH(app->parse(3, dummy_argv),
                      Catch::Matchers::ContainsSubstring(
                          "--normal-tables excludes --only-partition-tables"));
}

TEST_CASE("Only-partition-tables excludes temporary-tables",
          "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-partition-tables",
                               "--temporary-tables"};
  REQUIRE_THROWS_WITH(
      app->parse(3, dummy_argv),
      Catch::Matchers::ContainsSubstring(
          "--temporary-tables excludes --only-partition-tables"));
}

TEST_CASE("Only-partition-tables excludes only-temporary-tables",
          "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-partition-tables",
                               "--only-temporary-tables"};
  REQUIRE_THROWS_WITH(
      app->parse(3, dummy_argv),
      Catch::Matchers::ContainsSubstring(
          "--only-partition-tables excludes --only-temporary-tables"));
}

TEST_CASE("Only-temporary-tables disables other tables", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[5] = {"executable-name", "--only-temporary-tables"};

  app->parse(2, dummy_argv);

  REQUIRE(!opts.metadata.normal_tables);
  REQUIRE(opts.metadata.temporary_tables);
  REQUIRE(!opts.metadata.partitioned_tables);
}

TEST_CASE("Only-temporary-tables excludes normal-tables", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-temporary-tables",
                               "--normal-tables"};
  REQUIRE_THROWS_WITH(app->parse(3, dummy_argv),
                      Catch::Matchers::ContainsSubstring(
                          "--normal-tables excludes --only-temporary-tables"));
}

TEST_CASE("Only-temporary-tables excludes partitioned-tables",
          "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-temporary-tables",
                               "--partition-tables"};
  REQUIRE_THROWS_WITH(
      app->parse(3, dummy_argv),
      Catch::Matchers::ContainsSubstring(
          "--partition-tables excludes --only-temporary-tables"));
}

TEST_CASE("Only-partition-tables excludes only-normal-tables",
          "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-partition-tables",
                               "--only-normal-tables"};
  REQUIRE_THROWS_WITH(
      app->parse(3, dummy_argv),
      Catch::Matchers::ContainsSubstring(
          "--only-partition-tables excludes --only-normal-tables"));
}

TEST_CASE("Only-temporary-tables excludes only-normal-tables",
          "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-temporary-tables",
                               "--only-normal-tables"};
  REQUIRE_THROWS_WITH(
      app->parse(3, dummy_argv),
      Catch::Matchers::ContainsSubstring(
          "--only-temporary-tables excludes --only-normal-tables"));
}

TEST_CASE("Only-normal-tables disables other tables", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[5] = {"executable-name", "--only-normal-tables"};

  app->parse(2, dummy_argv);

  REQUIRE(opts.metadata.normal_tables);
  REQUIRE(!opts.metadata.temporary_tables);
  REQUIRE(!opts.metadata.partitioned_tables);
}

TEST_CASE("Only-normal-tables excludes partition-tables", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-normal-tables",
                               "--partition-tables"};
  REQUIRE_THROWS_WITH(app->parse(3, dummy_argv),
                      Catch::Matchers::ContainsSubstring(
                          "--partition-tables excludes --only-normal-tables"));
}

TEST_CASE("Only-normal-tables excludes temporary-tables", "[configuration]") {
  options_holder opts;
  auto app = setup_options(opts);

  const char *dummy_argv[3] = {"executable-name", "--only-normal-tables",
                               "--temporary-tables"};
  REQUIRE_THROWS_WITH(app->parse(3, dummy_argv),
                      Catch::Matchers::ContainsSubstring(
                          "--temporary-tables excludes --only-normal-tables"));
}
