#include <iostream>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_function.hpp>
#include <components/optimizer/optimizer.hpp>
#include <components/statistics/attach_statistics.hpp>
#include <components/explain/explain.hpp>

using namespace components::logical_plan;

int main() {
    std::pmr::monotonic_buffer_resource mem;

    // 1. Создаём SCAN
    auto scan = make_node_data(&mem, {"db", "coll"});

    // 2. Оборачиваем в FILTER
    auto filter = make_node_function(&mem, {"db", "coll"});
    filter->append_child(scan);

    // 3. Задаём статистику
    statistics::attach_statistics_recursively(filter, /*rows=*/50000);

    // 4. Прогоняем оптимизатор
    auto optimized = optimizer::optimize(filter);

    // 5. Выводим результат
    std::cout << explain::to_string(optimized);

    return 0;
}
