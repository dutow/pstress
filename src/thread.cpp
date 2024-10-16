#include "common.hpp"
#include "node.hpp"
#include "random_test.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <random>
#include <sstream>
std::atomic_flag lock_metadata = ATOMIC_FLAG_INIT;
std::atomic<bool> metadata_loaded(false);

void Node::workerThread(int number) {

  std::ofstream thread_log;
  std::ofstream client_log;
  if (options->at(Option::LOG_CLIENT_OUTPUT)->getBool()) {
    std::ostringstream cl;
    cl << myParams.logdir << "/" << myParams.myName << "_step_"
       << std::to_string(options->at(Option::STEP)->getInt()) << "_thread-"
       << number << ".out";
    client_log.open(cl.str(), std::ios::out | std::ios::trunc);
    if (!client_log.is_open()) {
      general_log << "Unable to open logfile for client output " << cl.str()
                  << ": " << std::strerror(errno) << std::endl;
      return;
    }
  }

  std::ostringstream os;
  os << myParams.logdir << "/" << myParams.myName << "_step_"
     << std::to_string(options->at(Option::STEP)->getInt()) << "_thread-"
     << number << ".sql";
  thread_log.open(os.str(), std::ios::out | std::ios::trunc);
  if (!thread_log.is_open()) {
    general_log << "Unable to open thread logfile " << os.str() << ": "
                << std::strerror(errno) << std::endl;
    return;
  }

  if (options->at(Option::LOG_QUERY_DURATION)->getBool()) {
    thread_log.precision(3);
    thread_log << std::fixed;
    std::cerr.precision(3);
    std::cerr << std::fixed;
    std::cout.precision(3);
    std::cout << std::fixed;
  }

  try {
    sql_variant::MySQL connection({myParams.database, myParams.address,
                                   myParams.socket, myParams.username,
                                   myParams.password, myParams.maxpacket,
                                   myParams.port});

    Thd1 *thd =
        new Thd1(number, thread_log, general_log, client_log, connection,
                 performed_queries_total, failed_queries_total);

    /* run pstress in with dynamic generator or infile */
    if (options->at(Option::PQUERY)->getBool() == false) {
      static bool success = false;

      /* load metadata */
      if (!lock_metadata.test_and_set()) {
        success = thd->load_metadata();
        metadata_loaded = true;
      }

      /* wait untill metadata is finished */
      while (!metadata_loaded) {
        std::chrono::seconds dura(3);
        std::this_thread::sleep_for(dura);
        thread_log << "waiting for metadata load to finish" << std::endl;
      }

      if (!success)
        thread_log << " initial setup failed, check logs for details "
                   << std::endl;
      else {
        if (!thd->run_some_query()) {
          std::ostringstream errmsg;
          errmsg << "Thread " << thd->thread_id
                 << " failed, check logs for detail message ";
          std::cerr << errmsg.str() << std::endl;
          if (general_log.is_open()) {
            general_log << errmsg.str() << std::endl;
          }
        }
      }
    } else {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> dis(0, querylist->size() - 1);
      int max_con_failures = 250;
      for (unsigned long i = 0; i < myParams.queries_per_thread; i++) {
        unsigned long query_number;
        // selecting query #, depends on random or sequential execution
        if (options->at(Option::NO_SHUFFLE)->getBool()) {
          query_number = i;
        } else {
          query_number = dis(gen);
        }
        // perform the query and getting the result
        execute_sql((*querylist)[query_number].c_str(), thd);

        if (thd->max_con_fail_count >= max_con_failures) {
          std::ostringstream errmsg;
          errmsg
              << "* Last " << thd->max_con_fail_count
              << " consecutive queries all failed. Likely crash/assert, user "
                 "privileges drop, or similar. Ending run.";
          std::cerr << errmsg.str() << std::endl;
          if (thread_log.is_open()) {
            thread_log << errmsg.str() << std::endl;
          }
          break;
        }
      }
    }
    delete thd;

  } catch (sql_variant::SqlException const &ex) {
    thread_log << "Error " << ex.what() << std::endl;

    general_log << ": Thread #" << number << " is exiting abnormally"
                << std::endl;

    return;
  }
}
