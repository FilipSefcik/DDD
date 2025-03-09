#pragma once
#include "../processing/process.hpp"

/*
 * @brief bdd_distributor is used as an interface to control processing of config file
 *
 * You can set a path to config file,
 * start calculating availability where you define used divider, value that you want to be
 * calculated and usage of timer, recieve the time of how long the calculating took.
 */
class ddd {
  private:
    int my_rank, process_count;
    double start_time, end_time;
    process* process_;

  public:
    ddd();
    void set_conf_path(const std::string& pa_conf_path);
    void calculate_availability(int divider_flag, int state, bool timer_on);
    void get_max_time();
    ~ddd();
};