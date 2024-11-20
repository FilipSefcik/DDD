#pragma once
#include "../modules/module.hpp"
#include <vector>

/*
 * @brief Divider assigns modules from parameter to processing nodes.
 *
 * Includes a `check` method for verifying input modules.
 */
class divider {
  public:
    virtual ~divider() = default;

    virtual void divide_modules(std::vector<module*>* modules, std::vector<module*> nodes[],
                                std::vector<int>* count) = 0;

  protected:
    void check(std::vector<module*>* modules); // Defined in the base class
};

/*
 * @brief Divides modules among nodes evenly based on no particular property.
 */
class node_divider : public divider {
  public:
    void divide_modules(std::vector<module*>* modules, std::vector<module*> nodes[],
                        std::vector<int>* count) override;
};

/*
 * @brief Divides modules among nodes evenly based on how many variables modules have.
 */
class var_count_divider : public divider {
  public:
    void divide_modules(std::vector<module*>* modules, std::vector<module*> nodes[],
                        std::vector<int>* count) override;
};
