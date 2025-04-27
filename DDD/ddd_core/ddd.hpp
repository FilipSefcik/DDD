#pragma once
#include "../modules/module_info.hpp"
#include <chrono>
#include <string>

/*
 * @brief bdd_distributor is used as an interface to control processing of config file
 *
 * You can set a path to config file,
 * start calculating availability where you define used divider, value that you want to be
 * calculated and usage of timer, recieve the time of how long the calculating took.
 */
class ddd {
  private:
    // int my_rank_, process_count_;
    std::chrono::high_resolution_clock::time_point start_time_, end_time_;
    std::string conf_path;
    void use_teddy(module_info* mod);
    void get_overall_reliability(std::vector<module_info*>* modules, int state);

  public:
    void set_conf_path(const std::string& pa_conf_path) { this->conf_path = pa_conf_path; }
    void calculate_availability(int state, bool timer_on);
    void get_max_time();
};