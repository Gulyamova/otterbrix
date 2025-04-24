#include <catch2/catch.hpp>
#include <optimizer/rules/projection_pruning.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/expressions/scalar_expression.hpp>

using namespace components::logical_plan;
using namespace components::optimizer::rules;
using namespace components::expressions;

TEST_CASE("Projection pruning drops unused columns") {
    auto node = make_node_data(nullptr, {"db", "test"});
    auto expr1 = make_scalar_identifier("a");
    auto expr2 = make_scalar_identifier("b");
    node->append_expression(expr1);
    node->append_expression(expr2);

    ProjectionPruningRule rule;
    auto result = rule.apply(node);

    REQUIRE(result.has_value());
    REQUIRE((*result)->expressions().size() == 1);
}
