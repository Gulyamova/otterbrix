#include <catch2/catch.hpp>
#include <components/logical_plan/node_data.hpp>
#include <components/expressions/scalar_expression.hpp>
#include <optimizer/optimizer.hpp>

using namespace components::logical_plan;
using namespace components::expressions;
using namespace optimizer;

TEST_CASE("Constant Folding: 2 + 2 -> 4") {
    auto node = make_node_data(nullptr, {"db", "table"});

    auto expr = make_scalar_add(
        make_scalar_literal(2),
        make_scalar_literal(2)
    );
    node->append_expression(expr);

    auto result = optimize(node);

    REQUIRE(result->expressions().size() == 1);
    auto lit = boost::dynamic_pointer_cast<scalar_literal_expression_t>(result->expressions()[0]);
    REQUIRE(lit != nullptr);
    REQUIRE(lit->to_string() == "4");
}
