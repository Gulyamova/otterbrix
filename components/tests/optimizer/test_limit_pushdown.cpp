#include <catch2/catch.hpp>

#include <optimizer/rules/limit_pushdown.hpp>
#include <components/logical_plan/node_limit.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_sort.hpp>
#include <components/logical_plan/node_aggregate.hpp>
#include <components/logical_plan/node_group.hpp>
#include <components/expressions/expression_builders.hpp>

using namespace components::optimizer::rules;
using namespace components::logical_plan;
using namespace components::expressions;

TEST_CASE("LimitPushdownRule tests") {
    LimitPushdownRule rule;

    SECTION("Limit over node_data") {
        auto scan = make_node_data(nullptr, {"db", "collection"});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "limit"));
        limit->append_child(scan);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit over node_sort") {
        auto sort = std::make_shared<node_sort>(nullptr, {"db", "sorted"}, {});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "10"));
        limit->append_child(sort);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit over node_aggregate") {
        auto agg = std::make_shared<node_aggregate>(nullptr, {"db", "agg"});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "5"));
        limit->append_child(agg);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit over unknown node should not apply") {
        auto group = std::make_shared<node_group>(nullptr, {"db", "g"});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "3"));
        limit->append_child(group);
        auto result = rule.apply(limit);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Limit with no children") {
        auto limit = std::make_shared<node_limit>(nullptr);
        auto result = rule.apply(limit);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Limit with multiple children") {
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_child(make_node_data(nullptr, {"db", "a"}));
        limit->append_child(make_node_data(nullptr, {"db", "b"}));
        auto result = rule.apply(limit);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Not a limit node") {
        auto scan = make_node_data(nullptr, {"db", "collection"});
        auto result = rule.apply(scan);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Limit with empty expressions") {
        auto scan = make_node_data(nullptr, {"db", "collection"});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_child(scan);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit over nested aggregate") {
        auto inner = std::make_shared<node_aggregate>(nullptr, {"db", "agg"});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "100"));
        limit->append_child(inner);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit value is expression") {
        auto scan = make_node_data(nullptr, {"db", "collection"});
        auto limit = std::make_shared<node_limit>(nullptr);
        auto expr = make_scalar_expression(nullptr, scalar_type::limit_value, "limit_expr");
        limit->append_expression(expr);
        limit->append_child(scan);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Deep tree with limit on aggregate") {
        auto agg = std::make_shared<node_aggregate>(nullptr, {"db", "agg"});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "20"));
        limit->append_child(agg);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit with non-integer expression") {
        auto scan = make_node_data(nullptr, {"db", "collection"});
        auto limit = std::make_shared<node_limit>(nullptr);
        auto expr = make_scalar_expression(nullptr, scalar_type::limit_value, "not_a_number");
        limit->append_expression(expr);
        limit->append_child(scan);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit over node_sort with no expressions") {
        auto sort = std::make_shared<node_sort>(nullptr, {"db", "sorted"}, {});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_child(sort);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit over sort, deeply nested") {
        auto sort = std::make_shared<node_sort>(nullptr, {"db", "nested"}, {});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "7"));
        limit->append_child(sort);
        auto result = rule.apply(limit);
        REQUIRE(result.has_value());
    }

    SECTION("Limit with child not in allowed list") {
        auto group = std::make_shared<node_group>(nullptr, {"db", "group"});
        auto limit = std::make_shared<node_limit>(nullptr);
        limit->append_expression(make_scalar_expression(nullptr, scalar_type::limit_value, "9"));
        limit->append_child(group);
        auto result = rule.apply(limit);
        REQUIRE_FALSE(result.has_value());
    }
}
