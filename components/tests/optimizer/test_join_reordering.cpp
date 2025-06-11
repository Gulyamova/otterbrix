#include <catch2/catch.hpp>
#include <optimizer/rules/join_reorder.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/logical_plan/node_data.hpp>
#include <optimizer/cost/estimator.hpp>

using namespace components::logical_plan;
using namespace components::optimizer::rules;
using namespace components::optimizer::cost;

TEST_CASE("JoinReorderRule: reorder three joins by cost") {
    std::pmr::monotonic_buffer_resource res;
    auto A = make_node_data(&res, {"db", "A"}); A->set_estimated_rows(1000);
    auto B = make_node_data(&res, {"db", "B"}); B->set_estimated_rows(100);
    auto C = make_node_data(&res, {"db", "C"}); C->set_estimated_rows(10);

    auto join = make_node_join(&res, {}, join_type::inner);
    join->append_child(A);
    join->append_child(B);
    join->append_child(C);

    JoinReorderRule rule;
    auto result = rule.apply(join);
    REQUIRE(result.has_value());
    auto reordered = std::static_pointer_cast<node_join_t>(*result);
    REQUIRE(reordered->children().size() == 2);
    REQUIRE(reordered->children()[0]->collection_full_name().collection == "C");
}

TEST_CASE("JoinReorderRule: nested join JOIN(JOIN(A,B),C)") {
    std::pmr::monotonic_buffer_resource res;
    auto A = make_node_data(&res, {"db", "A"}); A->set_estimated_rows(200);
    auto B = make_node_data(&res, {"db", "B"}); B->set_estimated_rows(100);
    auto C = make_node_data(&res, {"db", "C"}); C->set_estimated_rows(10);

    auto inner = make_node_join(&res, {}, join_type::inner);
    inner->append_child(A);
    inner->append_child(B);

    auto outer = make_node_join(&res, {}, join_type::inner);
    outer->append_child(inner);
    outer->append_child(C);

    JoinReorderRule rule;
    auto result = rule.apply(outer);
    REQUIRE(result.has_value());
    auto reordered = std::static_pointer_cast<node_join_t>(*result);
    REQUIRE(reordered->children().size() == 2);
}

TEST_CASE("JoinReorderRule: keep ON expressions") {
    std::pmr::monotonic_buffer_resource res;
    auto A = make_node_data(&res, {"db", "A"}); A->set_estimated_rows(500);
    auto B = make_node_data(&res, {"db", "B"}); B->set_estimated_rows(100);

    auto join = make_node_join(&res, {}, join_type::inner);
    join->append_child(A);
    join->append_child(B);

    // Добавим выражение-предикат (заглушка)
    join->append_expression(std::make_shared<components::expressions::expression_constant_t>(true));

    JoinReorderRule rule;
    auto result = rule.apply(join);
    REQUIRE(result.has_value());
    REQUIRE(!(*result)->expressions().empty());
}

TEST_CASE("JoinReorderRule: returns nullopt for non-join node") {
    std::pmr::monotonic_buffer_resource res;
    auto node = make_node_data(&res, {"db", "A"});

    JoinReorderRule rule;
    auto result = rule.apply(node);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("JoinReorderRule: returns nullopt for single child join") {
    std::pmr::monotonic_buffer_resource res;
    auto A = make_node_data(&res, {"db", "A"});
    auto join = make_node_join(&res, {}, join_type::inner);
    join->append_child(A);

    JoinReorderRule rule;
    auto result = rule.apply(join);
    REQUIRE_FALSE(result.has_value());
}

TEST_CASE("JoinReorderRule: reorder four inputs by cost") {
    std::pmr::monotonic_buffer_resource res;
    std::vector<node_ptr> inputs = {
        make_node_data(&res, {"db", "A"}),
        make_node_data(&res, {"db", "B"}),
        make_node_data(&res, {"db", "C"}),
        make_node_data(&res, {"db", "D"})
    };
    inputs[0]->set_estimated_rows(500);
    inputs[1]->set_estimated_rows(100);
    inputs[2]->set_estimated_rows(10);
    inputs[3]->set_estimated_rows(50);

    auto join = make_node_join(&res, {}, join_type::inner);
    for (auto& n : inputs) join->append_child(n);

    JoinReorderRule rule;
    auto result = rule.apply(join);
    REQUIRE(result.has_value());
    REQUIRE((*result)->children().size() == 2);
}

TEST_CASE("JoinReorderRule: reorder with equal cost") {
    std::pmr::monotonic_buffer_resource res;
    auto A = make_node_data(&res, {"db", "A"}); A->set_estimated_rows(100);
    auto B = make_node_data(&res, {"db", "B"}); B->set_estimated_rows(100);

    auto join = make_node_join(&res, {}, join_type::inner);
    join->append_child(A);
    join->append_child(B);

    JoinReorderRule rule;
    auto result = rule.apply(join);
    REQUIRE(result.has_value());
}