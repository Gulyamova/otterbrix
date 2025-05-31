#include "create_plan_join.hpp"

#include <components/expressions/compare_expression.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/physical_plan/collection/operators/operator_join.hpp>
#include <components/physical_plan/collection/operators/operator_hash_join.hpp>
#include <components/physical_plan/collection/operators/operator_merge_join.hpp>
#include <components/physical_plan/collection/operators/operator_index_nested_loop_join.hpp>
#include <components/physical_plan_generator/create_plan.hpp>
#include <components/optimizer/cost/join_strategy.hpp>

namespace services::collection::planner::impl {

    operators::operator_ptr create_plan_join(const context_storage_t& context,
                                             const components::logical_plan::node_ptr& node,
                                             components::logical_plan::limit_t limit) {
        const auto* join_node = static_cast<const components::logical_plan::node_join_t*>(node.get());
        // assign left collection as actor for join
        auto expr = reinterpret_cast<const components::expressions::compare_expression_ptr*>(&node->expressions()[0]);
        auto collection_context = context.at(node->children().front()->collection_full_name());
        auto predicate = operators::predicates::create_predicate(collection_context, *expr);
       
        // CBO: выбираем join стратегию
        const auto& left_node = node->children().front();
        const auto& right_node = node->children().back();
        auto strategy = cost::choose_join_strategy(left_node, right_node);

        // Рекурсивно создаём подпланы
        operators::operator_ptr left;
        operators::operator_ptr right;
        if (left_node) {
            left = create_plan(context, left_node, limit);
        }
        if (right_node) {
            right = create_plan(context, right_node, limit);
        }

        // оператор на основе выбранной стратегии
        switch (strategy) {
    case cost::join_strategy_t::hash:
        return make_intrusive<operators::operator_hash_join_t>(std::move(predicate),
                                                               std::move(left), std::move(right));

    case cost::join_strategy_t::merge:
        return make_intrusive<operators::operator_merge_join_t>(context.at(node->collection_full_name()),
                                                                std::move(predicate));

    case cost::join_strategy_t::index_nested_loop:
        return make_intrusive<operators::operator_index_nested_loop_join_t>(
                    context.at(node->collection_full_name()), std::move(predicate));

    case cost::join_strategy_t::grace_hash:
        return make_intrusive<operators::operator_grace_hash_join_t>(
                    context.at(node->collection_full_name()), std::move(predicate), 100'000);
}
                                             }
} // namespace services::collection::planner::impl
