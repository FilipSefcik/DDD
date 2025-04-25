#include "ddd.hpp"
#include "../processing/module_manager.hpp"
#include "libteddy/inc/reliability.hpp"
#include <chrono>
#include <iostream>
#include <libteddy/inc/io.hpp>

void ddd::calculate_availability(int state, bool timer_on) {
    if (timer_on) {
        this->start_time_ = std::chrono::high_resolution_clock::now();
    }

    module_manager moduleManager;
    moduleManager.load(this->conf_path);
    std::vector<module_info*>* modules = moduleManager.get_modules();
    std::sort(modules->begin(), modules->end(),
              [](module_info* a, module_info* b) { return a->get_priority() < b->get_priority(); });

    for (size_t i = 0; i < modules->size(); i++) {
        // std::cout << i << std::endl;
        this->use_teddy(modules->at(i));
    }

    modules->at(modules->size() - 1)->print_reliability(state);

    if (timer_on) {
        this->end_time_ = std::chrono::high_resolution_clock::now();
    }
}

void ddd::use_teddy(module_info* mod) {
    std::vector<double> ps;
    std::string const& path = mod->get_pla_path();
    std::optional<teddy::pla_file_binary> file = teddy::load_binary_pla(path, nullptr);
    teddy::bss_manager bssManager(file->input_count_, mod->get_var_count() * 100);
    teddy::bdd_manager::diagram_t f =
        teddy::io::from_pla(bssManager, *file)[mod->get_function_column()];
    ps = bssManager.calculate_probabilities(*mod->get_sons_reliability(), f);
    mod->set_my_reliability(&ps);

    module_info* parent = mod->get_parent();
    if (parent) {
        parent->set_sons_reliability(mod->get_position(), &ps);
    }
};

void ddd::get_max_time() {
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(this->end_time_ - this->start_time_);
    double seconds = duration.count() / 1000.0;
    std::cout << "Time: " << seconds << std::endl;
}
