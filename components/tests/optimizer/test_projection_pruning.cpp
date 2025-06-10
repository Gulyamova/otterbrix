#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <optimizer/rules/projection_pruning.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/logical_plan/node_group.hpp>
#include <components/logical_plan/node_sort.hpp>
#include <components/logical_plan/node_match.hpp>
#include <components/expressions/expression_builders.hpp>

using namespace components::optimizer::rules;
using namespace components::logical_plan;
using namespace components::expressions;

TEST_CASE("ProjectionPruningRule tests") {
    ProjectionPruningRule rule;

    auto make_expr = [](const std::string& name) {
        return make_scalar_expression(nullptr, scalar_type::get_field, name);
    };

    SECTION("Removes unused expression from data") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto expr1 = make_expr("a");
        auto expr2 = make_expr("b");
        scan->append_expression(expr1);
        scan->append_expression(expr2);

        auto match = make_node_match(nullptr, scan->collection_full_name(), expr1);
        match->append_child(scan);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(scan->expressions().size() == 1);
        REQUIRE(scan->expressions()[0] == expr1);
    }

    SECTION("Keeps all used expressions") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto expr1 = make_expr("x");
        auto expr2 = make_expr("y");
        scan->append_expression(expr1);
        scan->append_expression(expr2);

        auto group = std::make_shared<node_group>(nullptr, scan->collection_full_name());
        group->append_expression(expr1);
        group->append_expression(expr2);
        group->append_child(scan);

        auto result = rule.apply(group);
        REQUIRE(result.has_value());
        REQUIRE(scan->expressions().size() == 2);
    }

    SECTION("No expressions to prune") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto result = rule.apply(scan);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Expression unused in nested child") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto expr = make_expr("z");
        scan->append_expression(expr);

        auto match = make_node_match(nullptr, scan->collection_full_name(), make_expr("x"));
        match->append_child(scan);

        auto result = rule.apply(match);
        REQUIRE(scan->expressions().empty());
    }

    SECTION("Multiple unused expressions") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        for (auto name : {"a", "b", "c", "d"}) {
            scan->append_expression(make_expr(name));
        }

        auto match = make_node_match(nullptr, scan->collection_full_name(), make_expr("a"));
        match->append_child(scan);

        auto result = rule.apply(match);
        REQUIRE(scan->expressions().size() == 1);
    }

    SECTION("Works with nested match nodes") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        scan->append_expression(make_expr("f"));

        auto m1 = make_node_match(nullptr, scan->collection_full_name(), make_expr("f"));
        m1->append_child(scan);

        auto m2 = make_node_match(nullptr, scan->collection_full_name(), make_expr("f"));
        m2->append_child(m1);

        auto result = rule.apply(m2);
        REQUIRE(result.has_value());
        REQUIRE(scan->expressions().size() == 1);
    }

    SECTION("Does not modify if all expressions used") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto expr = make_expr("abc");
        scan->append_expression(expr);

        auto match = make_node_match(nullptr, scan->collection_full_name(), expr);
        match->append_child(scan);

        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value()); 
    }

    SECTION("Removes all expressions if none used") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        scan->append_expression(make_expr("junk"));

        auto group = std::make_shared<node_group>(nullptr, scan->collection_full_name());
        group->append_child(scan);

        auto result = rule.apply(group);
        REQUIRE(scan->expressions().empty());
    }

    SECTION("Handles deeply nested trees") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        scan->append_expression(make_expr("keep"));
        scan->append_expression(make_expr("drop"));

        auto match = make_node_match(nullptr, scan->collection_full_name(), make_expr("keep"));
        auto group = std::make_shared<node_group>(nullptr, scan->collection_full_name());
        match->append_child(scan);
        group->append_child(match);

        auto result = rule.apply(group);
        REQUIRE(scan->expressions().size() == 1);
    }

    SECTION("Handles multiple children") {
        auto scan1 = make_node_data(nullptr, {"db", "a"});
        scan1->append_expression(make_expr("x"));

        auto scan2 = make_node_data(nullptr, {"db", "b"});
        scan2->append_expression(make_expr("y"));

        auto match = make_node_match(nullptr, {"db", "a"}, make_expr("x"));
        match->append_child(scan1);
        match->append_child(scan2);

        auto result = rule.apply(match);
        REQUIRE(scan1->expressions().size() == 1);
        REQUIRE(scan2->expressions().empty());
    }

    SECTION("No changes on unrelated node types") {
        auto sort = std::make_shared<node_sort>(nullptr, {"db", "s"}, {});
        auto result = rule.apply(sort);
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Keeps expressions used in sort") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto expr = make_expr("sortme");
        scan->append_expression(expr);

        auto sort = std::make_shared<node_sort>(nullptr, scan->collection_full_name(), {make_sort_expression("sortme", sort_order::asc)});
        sort->append_child(scan);

        auto result = rule.apply(sort);
        REQUIRE(result.has_value());
        REQUIRE(scan->expressions().size() == 1);
    }

    SECTION("Leaves empty children alone") {
        auto scan = make_node_data(nullptr, {"db", "empty"});
        auto match = make_node_match(nullptr, scan->collection_full_name(), make_expr("x"));
        match->append_child(scan);
        auto result = rule.apply(match);
        REQUIRE(scan->expressions().empty());
    }

    SECTION("Does not modify node_data without expressions") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto match = make_node_match(nullptr, scan->collection_full_name(), make_expr("x"));
        match->append_child(scan);
        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value());
    }
}
