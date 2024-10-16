#include "node.hpp"
#include "common.hpp"
#include "random_test.hpp"
#include "sql_variant/sql_variant.hpp"
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <iostream>

Node::Node() {
  workers.clear();
  performed_queries_total = 0;
  failed_queries_total = 0;
}

void Node::end_node() {
  writeFinalReport();
  if (general_log)
    general_log.close();
  if (options->at(Option::PQUERY)->getBool() && querylist)
    delete querylist;
}

/* create the logdir if does not exist
@return false on error, true on success */

static bool create_log_dir() {
  std::string logdir_path(opt_string(LOGDIR));

  std::error_code ec;
  std::filesystem::create_directories(logdir_path, ec);

  return !static_cast<bool>(ec);
}

bool Node::createGeneralLog() {
  std::string logName;
  logName = myParams.logdir + "/" + myParams.myName + "_ddl_step_" +
            std::to_string(options->at(Option::STEP)->getInt()) + ".log";
  bool is_success = create_log_dir();
  if (!is_success) {
    std::cerr << "Could not create log dir: " << strerror(errno) << std::endl;
    return false;
  }
  general_log.open(logName, std::ios::out | std::ios::trunc);
  general_log << "- PStress v" << PQVERSION << "-" << PQREVISION
              << " compiled with MySQL: " << mysql_get_client_info()
              << std::endl;

  if (!general_log.is_open()) {
    std::cout << "Unable to open log file " << logName << ": "
              << std::strerror(errno) << std::endl;
    return false;
  }
  return true;
}

void Node::writeFinalReport() {
  if (general_log.is_open()) {
    std::ostringstream exitmsg;
    exitmsg.precision(2);
    exitmsg << std::fixed;
    exitmsg << "* NODE SUMMARY: " << failed_queries_total << "/"
            << performed_queries_total << " queries failed, ("
            << (performed_queries_total - failed_queries_total) * 100.0 /
                   performed_queries_total
            << "% were successful)";
    general_log << exitmsg.str() << std::endl;
  }
}

int Node::startWork() {

  if (!createGeneralLog()) {
    std::cerr << "Exiting..." << std::endl;
    return 2;
  }

  if (myParams.flavor != "") {
    options->at(Option::FLAVOR)->setString(myParams.flavor);
  }

  std::cout << "- Connecting to " << myParams.myName << " [" << myParams.address
            << "]..." << std::endl;
  general_log << "- Connecting to " << myParams.myName << " ["
              << myParams.address << "]..." << std::endl;
  tryConnect();

  if (options->at(Option::PQUERY)->getBool()) {
    std::ifstream sqlfile_in;
    sqlfile_in.open(myParams.infile);

    if (!sqlfile_in.is_open()) {
      std::cerr << "Unable to open SQL file " << myParams.infile << ": "
                << strerror(errno) << std::endl;
      general_log << "Unable to open SQL file " << myParams.infile << ": "
                  << strerror(errno) << std::endl;
      return EXIT_FAILURE;
    }
    querylist = new std::vector<std::string>;
    std::string line;

    while (getline(sqlfile_in, line)) {
      if (!line.empty()) {
        querylist->push_back(line);
      }
    }

    sqlfile_in.close();
    general_log << "- Read " << querylist->size() << " lines from "
                << myParams.infile << std::endl;

    /* log replaying */
    if (options->at(Option::NO_SHUFFLE)->getBool()) {
      myParams.threads = 1;
      myParams.queries_per_thread = querylist->size();
    }
  }
  /* END log replaying */
  workers.resize(myParams.threads);

  for (int i = 0; i < myParams.threads; i++) {
    workers[i] = std::thread(&Node::workerThread, this, i);
  }

  for (int i = 0; i < myParams.threads; i++) {
    workers[i].join();
  }
  return EXIT_SUCCESS;
}

void Node::tryConnect() {

  try {
    auto connection = sql_variant::connect(
        myParams.flavor, {myParams.database, myParams.address, myParams.socket,
                          myParams.username, myParams.password,
                          myParams.maxpacket, myParams.port});

    general_log << "- Connected to " << connection->hostInfo() << "..."
                << std::endl;

    std::string server_version = connection->serverInfoString();

    general_log << "- Connected server version: " << server_version
                << std::endl;

    // TODO: move this output to the correct location
    if (strcmp(PLATFORM_ID, "Darwin") == 0)
      general_log << "- Table compression is disabled as hole punching is not "
                     "supported on OSX"
                  << std::endl;

  } catch (sql_variant::SqlException &sqle) {
    std::cerr << "Error " << sqle.what() << std::endl;
    std::cerr << "* PSTRESS: Unable to continue [1], exiting" << std::endl;
    general_log << "Error " << sqle.what() << std::endl;
    general_log << "* PSTRESS: Unable to continue [1], exiting" << std::endl;

    exit(EXIT_FAILURE);
  }

  if (options->at(Option::TEST_CONNECTION)->getBool()) {
    exit(EXIT_SUCCESS);
  }
}
