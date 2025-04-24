#include <catch2/catch.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/expressions/compare_expression.hpp>
#include <components/expressions/scalar_expression.hpp>
#include <optimizer/optimizer.hpp>

using namespace components::logical_plan;
using namespace components::expressions;
using namespace optimizer;

TEST_CASE("Predicate Simplification: WHERE TRUE → removed") {
    auto node = make_node_data(nullptr, {"db", "table"});

    auto condition = make_compare_expression(
        compare_operator_type::EQUAL,
        make_scalar_literal(true),
        make_scalar_literal(true)
    );
    node->append_expression(condition);

    auto optimized = optimize(node);

    REQUIRE(optimized->expressions().empty());
}
