#include <catch2/catch.hpp>

#include <optimizer/rules/filter_pushdown.hpp>
#include <components/logical_plan/node_match.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_join.hpp>
#include <components/logical_plan/node_sort.hpp>
#include <components/logical_plan/node_aggregate.hpp>
#include <components/logical_plan/node_function.hpp>
#include <components/logical_plan/node_group.hpp>
#include <components/expressions/expression_builders.hpp>

using namespace optimizer::rules;
using namespace components::logical_plan;
using namespace components::expressions;

TEST_CASE("FilterPushdownRule basic cases") {

    FilterPushdownRule rule;

    SECTION("FilterOverData") {
        auto data = make_node_data(nullptr, {"db", "collection"});
        auto expr = make_compare_expression(nullptr, compare_type::eq);
        auto match = make_node_match(nullptr, {"db", "collection"}, expr);
        match->append_child(data);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_match_t);
    }

    SECTION("FilterOverJoinPushedToBothSides") {
        auto data1 = make_node_data(nullptr, {"db", "collection1"});
        auto data2 = make_node_data(nullptr, {"db", "collection2"});
        auto join = std::make_shared<node_join>(nullptr);
        join->append_child(data1);
        join->append_child(data2);

        auto expr = make_compare_expression(nullptr, compare_type::eq);
        auto match = make_node_match(nullptr, {"db", "collection"}, expr);
        match->append_child(join);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_join_t);
        REQUIRE(result.value()->children().size() == 2);
        for (const auto& child : result.value()->children()) {
            REQUIRE(child->type() == node_type::node_match_t);
        }
    }

    SECTION("FilterOverSort") {
        auto sort = std::make_shared<node_sort>(nullptr, {"db", "sorted"}, std::vector<expression_ptr>{});
        auto expr = make_compare_expression(nullptr, compare_type::eq);
        auto match = make_node_match(nullptr, {"db", "sorted"}, expr);
        match->append_child(sort);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_match_t);
    }

    SECTION("FilterOverAggregate") {
        auto agg = std::make_shared<node_aggregate>(nullptr, {"db", "agg"});
        auto expr = make_compare_expression(nullptr, compare_type::neq);
        auto match = make_node_match(nullptr, {"db", "agg"}, expr);
        match->append_child(agg);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_match_t);
    }

    SECTION("FilterOverEmptyExpression") {
        auto data = make_node_data(nullptr, {"db", "collection"});
        auto match = make_node_match(nullptr, {"db", "collection"});
        match->append_child(data);

        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("FilterOverUnknownNode") {
        auto unknown = std::make_shared<node_group>(nullptr, {"db", "unknown"});
        auto expr = make_compare_expression(nullptr, compare_type::gt);
        auto match = make_node_match(nullptr, {"db", "unknown"}, expr);
        match->append_child(unknown);

        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("FilterOverJoinOneChild") {
        auto data1 = make_node_data(nullptr, {"db", "collection1"});
        auto join = std::make_shared<node_join>(nullptr);
        join->append_child(data1);

        auto expr = make_compare_expression(nullptr, compare_type::lt);
        auto match = make_node_match(nullptr, {"db", "collection"}, expr);
        match->append_child(join);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_join_t);
    }

    SECTION("FilterNotMatchNode") {
        auto data = make_node_data(nullptr, {"db", "collection"});
        auto result = rule.apply(data);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("FilterMultipleChildren") {
        auto match = make_node_match(nullptr, {"db", "collection"}, make_compare_expression(nullptr, compare_type::eq));
        match->append_child(make_node_data(nullptr, {"db", "a"}));
        match->append_child(make_node_data(nullptr, {"db", "b"}));

        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("FilterOverDataWithMultipleExprs") {
        auto data = make_node_data(nullptr, {"db", "collection"});
        auto match = make_node_match(nullptr, {"db", "collection"});
        match->append_expressions({
            make_compare_expression(nullptr, compare_type::eq),
            make_compare_expression(nullptr, compare_type::lt)
        });
        match->append_child(data);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_match_t);
    }

    SECTION("FilterOverDeepJoin") {
        auto data1 = make_node_data(nullptr, {"db", "a"});
        auto data2 = make_node_data(nullptr, {"db", "b"});
        auto join_inner = std::make_shared<node_join>(nullptr);
        join_inner->append_child(data1);
        join_inner->append_child(data2);

        auto join_outer = std::make_shared<node_join>(nullptr);
        join_outer->append_child(join_inner);
        join_outer->append_child(make_node_data(nullptr, {"db", "c"}));

        auto expr = make_compare_expression(nullptr, compare_type::eq);
        auto match = make_node_match(nullptr, {"db", "any"}, expr);
        match->append_child(join_outer);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_join_t);
    }

    SECTION("FilterOverSortWithEmptyExpr") {
        auto sort = std::make_shared<node_sort>(nullptr, {"db", "sorted"}, std::vector<expression_ptr>{});
        auto match = make_node_match(nullptr, {"db", "sorted"});
        match->append_child(sort);

        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("FilterOverAggregateWithExpr") {
        auto agg = std::make_shared<node_aggregate>(nullptr, {"db", "agg"});
        auto expr = make_compare_expression(nullptr, compare_type::is_null);
        auto match = make_node_match(nullptr, {"db", "agg"}, expr);
        match->append_child(agg);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(result.value()->type() == node_type::node_match_t);
    }

    SECTION("FilterOnFunctionShouldSkip") {
        auto fn = std::make_shared<node_function>(nullptr);
        auto expr = make_compare_expression(nullptr, compare_type::gt);
        auto match = make_node_match(nullptr, {"db", "fn"}, expr);
        match->append_child(fn);

        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("FilterOverDeepDataTree") {
        auto data = make_node_data(nullptr, {"db", "collection"});
        auto match1 = make_node_match(nullptr, {"db", "collection"}, make_compare_expression(nullptr, compare_type::eq));
        match1->append_child(data);

        auto match2 = make_node_match(nullptr, {"db", "collection"}, make_compare_expression(nullptr, compare_type::lt));
        match2->append_child(match1);

        auto result = rule.apply(match2);
        REQUIRE(result.has_value());
    }
}
