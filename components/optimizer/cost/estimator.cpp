#include "estimator.hpp"
#include <statistics/statistics.hpp>
#include <components/logical_plan/node.hpp>

namespace components::optimizer::cost {

    // Структура для хранения затрат на ресурсы
    struct cost_t {
        double cpu_cost;  
        double ram_cost;  
        double rows_out; 

        double total() const {
            return cpu_cost + ram_cost * 0.01;  
        }
    };

    // SCAN: оценка затрат на чтение таблицы
    cost_t estimate_table_scan(const node_ptr& node) {
        if (node->statistics()) {
            if (node->statistics()->mode() == data_mode::table) {
                auto stats = std::static_pointer_cast<TableStatistics>(node->statistics());
                size_t rows = stats->row_count;

                double cpu_cost = rows * 0.5;
                double ram_cost = rows * 0.001;

                node->set_estimated_rows(rows);
                node->set_cpu_cost(cpu_cost);
                node->set_ram_cost(ram_cost);

                return {cpu_cost, ram_cost, rows};
            }
        }

        // если статистики нет
        size_t estimate = 1000;
        double cpu_cost = estimate * 0.5;
        double ram_cost = estimate * 0.001;

        node->set_estimated_rows(estimate);
        node->set_cpu_cost(cpu_cost);
        node->set_ram_cost(ram_cost);

        return {cpu_cost, ram_cost, estimate};
    }

    // FILTER: оценка затрат на фильтрацию
    cost_t estimate_filter(const node_ptr& node) {
        auto input = node->children()[0]; 
        auto input_stats = input->statistics();
        auto rows_in = input_stats->row_count;
        double selectivity = 0.1;  // Пример: 10% строк проходят фильтр
        double rows_out = rows_in * selectivity;

        double cpu_cost = rows_in * 0.2; 
        double ram_cost = rows_in * 0.0005;

        node->set_estimated_rows(rows_out);
        node->set_cpu_cost(cpu_cost);
        node->set_ram_cost(ram_cost);

        return {cpu_cost, ram_cost, rows_out};
    }

    // JOIN: оценка затрат на соединение
    cost_t estimate_join(const node_ptr& node) {
        auto left = node->children()[0];
        auto right = node->children()[1];

        auto left_cost = estimate_node_cost(left);
        auto right_cost = estimate_node_cost(right);

        double rows_out = left_cost.rows_out * right_cost.rows_out;

        // Для hash join учтём дополнительные затраты
        double cpu_cost = left_cost.cpu_cost + right_cost.cpu_cost + (rows_out * 0.5);  
        double ram_cost = left_cost.ram_cost + right_cost.ram_cost + (rows_out * 0.01); 

        node->set_estimated_rows(rows_out);
        node->set_cpu_cost(cpu_cost);
        node->set_ram_cost(ram_cost);

        return {cpu_cost, ram_cost, rows_out};
    }

    // Функция для оценки затрат на узел
    cost_t estimate_node_cost(const node_ptr& node) {
        switch (node->type()) {
            case node_type::data_t:
                return estimate_table_scan(node);  
            case node_type::function_t:
                return estimate_filter(node);  
            case node_type::join_t:
                return estimate_join(node);  
            default:
                return {0.0, 0.0, 0.0}; 
        }
    }

    void estimate_node_output_rows(const node_ptr& node) {
        if (node->statistics()) {
            auto stats = node->statistics();
            node->set_estimated_rows(stats->row_count);
        } else {
            size_t total_rows = 0;
            for (const auto& child : node->children()) {
                total_rows += child->estimated_rows();
            }
            node->set_estimated_rows(total_rows);
        }
    }

}  // namespace components::optimizer::cost
