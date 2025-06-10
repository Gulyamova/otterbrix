#include <catch2/catch.hpp>

#include <optimizer/rules/constant_folding.hpp>
#include <components/expressions/expression_builders.hpp>
#include <components/expressions/expression_constant.hpp>
#include <components/logical_plan/node_match.hpp>
#include <components/logical_plan/node_data.hpp>

using namespace components::optimizer::rules;
using namespace components::logical_plan;
using namespace components::expressions;

TEST_CASE("Constant folding rule") {
    ConstantFoldingRule rule;

    auto make_const_expr = [](int32_t value) {
        return std::make_shared<expression_constant_t>(value_t(value));
    };

    auto make_add_expr = [](expression_ptr left, expression_ptr right) {
        auto expr = make_scalar_expression(nullptr, scalar_type::add);
        expr->append_param(std::move(left));
        expr->append_param(std::move(right));
        return expr;
    };

    SECTION("Fold constant expression: 3 + 5 → 8") {
        auto expr = make_add_expr(make_const_expr(3), make_const_expr(5));
        auto match = make_node_match(nullptr, {"db", "t"}, expr);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(match->expressions()[0]->is_constant());

        auto constant_expr = std::dynamic_pointer_cast<expression_constant_t>(match->expressions()[0]);
        REQUIRE(constant_expr->value() == value_t(8));
    }

    SECTION("Do not fold non-constant expression: a + 5") {
        auto var = make_scalar_expression(nullptr, scalar_type::get_field, "a");
        auto expr = make_add_expr(var, make_const_expr(5));
        auto match = make_node_match(nullptr, {"db", "t"}, expr);

        auto result = rule.apply(match);
        REQUIRE_FALSE(result.has_value());
        REQUIRE_FALSE(match->expressions()[0]->is_constant());
    }

    SECTION("Fold nested constant: (1 + 2) + 3") {
        auto inner = make_add_expr(make_const_expr(1), make_const_expr(2)); 
        auto outer = make_add_expr(inner, make_const_expr(3)); 
        auto match = make_node_match(nullptr, {"db", "t"}, outer);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(match->expressions()[0]->is_constant());

        auto constant_expr = std::dynamic_pointer_cast<expression_constant_t>(match->expressions()[0]);
        REQUIRE(constant_expr->value() == value_t(6));
    }

    SECTION("Fold multiple expressions in node") {
        auto expr1 = make_add_expr(make_const_expr(10), make_const_expr(5)); 
        auto expr2 = make_add_expr(make_const_expr(4), make_const_expr(1));  
        auto match = make_node_match(nullptr, {"db", "t"}, expr1);
        match->append_expression(expr2);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());
        REQUIRE(match->expressions()[0]->is_constant());
        REQUIRE(match->expressions()[1]->is_constant());
    }

    SECTION("Nested constant inside node_data") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto expr = make_add_expr(make_const_expr(2), make_const_expr(2));
        scan->append_expression(expr);

        auto result = rule.apply(scan);
        REQUIRE(result.has_value());
        REQUIRE(scan->expressions()[0]->is_constant());
        auto value = std::dynamic_pointer_cast<expression_constant_t>(scan->expressions()[0])->value();
        REQUIRE(value == value_t(4));
    }

    SECTION("No folding on already constant") {
        auto scan = make_node_data(nullptr, {"db", "t"});
        auto expr = make_const_expr(42);
        scan->append_expression(expr);

        auto result = rule.apply(scan);
        REQUIRE_FALSE(result.has_value());
        REQUIRE(scan->expressions()[0]->is_constant());
    }

    SECTION("Folding in deep tree") {
        auto inner = make_add_expr(make_const_expr(1), make_const_expr(1)); // 2
        auto scan = make_node_data(nullptr, {"db", "t"});
        scan->append_expression(inner);

        auto match = make_node_match(nullptr, scan->collection_full_name(), make_const_expr(true));
        match->append_child(scan);

        auto result = rule.apply(match);
        REQUIRE(result.has_value());

        auto folded_expr = std::dynamic_pointer_cast<expression_constant_t>(scan->expressions()[0]);
        REQUIRE(folded_expr->value() == value_t(2));
    }
}
