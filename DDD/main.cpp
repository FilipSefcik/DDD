#include "ddd_core/ddd.hpp"

/*
 * @brief this function is called when app is used. Processes information based on command line
 * input arguments.
 *
 * To start main you should use command:
 * mpiexec <0> -n <1> <2> <3> <4> <5> <6>
 * \Where:
 * \0 - usage of threads as process (optional, default are cores), if yes use --use-hwthread-cpus
 * \1 - number of processes
 * \2 - path to this main
 * \3 - path to config file
 * \4 - which divider should be used (0 for var_count_divider or 1 for node_divider)
 * \5 - availability which state (0 or 1) should be calculated
 * \6 - usage of timer (y or n)
 */
int main(int argc, char* argv[]) {
    std::string conf_path = argc > 1 ? argv[1] : "module_map.conf";
    int divider_flag = argc > 2 ? std::stoi(argv[2]) : 0;
    int state = argc > 3 ? std::stoi(argv[3]) : 1;
    bool timer = false;
    if (argc > 4) {
        std::string yes = "y";
        if (yes == argv[4]) {
            timer = true;
        }
    }

    ddd distributor;
    distributor.set_conf_path(conf_path);
    distributor.calculate_availability(divider_flag, state, timer);
    if (timer) {
        distributor.get_max_time();
    }
    return 0;
}