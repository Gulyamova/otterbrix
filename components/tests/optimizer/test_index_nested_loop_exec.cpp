#include <catch2/catch.hpp>
#include <components/physical_plan/collection/operators/operator_index_nested_loop_join.hpp>
#include <components/document/document.hpp>

using namespace services::collection::operators;
using components::document::document_t;

static operator_data_ptr make_data(std::initializer_list<int> keys)
{
    auto d = make_operator_data(nullptr);
    for (int k : keys) {
        auto doc = document_t::make_document(nullptr);
        doc->set("id", k); d->append(doc);
    }
    return d;
}

TEST_CASE("Index-NL join tiny right")
{
    context_collection_t ctx;
    auto left  = make_intrusive<read_only_operator_t>(&ctx, operator_type::data_source);
    auto right = make_intrusive<read_only_operator_t>(&ctx, operator_type::data_source);
    left ->set_output(make_data({1,2,3,4}));
    right->set_output(make_data({2,4}));

    auto pred = expressions::make_compare_expression(
        expressions::compare_operator_type::EQUAL,
        expressions::make_scalar_identifier("id"),
        expressions::make_scalar_identifier("id"));

    auto join = make_intrusive<operator_index_nested_loop_join_t>(&ctx,std::move(pred));
    join->set_left(left); join->set_right(right);
    join->on_execute(nullptr);

    REQUIRE(join->output()->documents().size()==2);
}
