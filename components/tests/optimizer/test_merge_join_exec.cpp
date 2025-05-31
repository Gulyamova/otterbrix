#include <catch2/catch.hpp>
#include <components/physical_plan/collection/operators/operator_merge_join.hpp>
#include <components/document/document.hpp>

using namespace services::collection::operators;
using components::document::document_t;
using components::pipeline::context_t;

TEST_CASE("Merge-Join returns intersection by key")
{
    auto* res = nullptr;   // используем std::pmr::null_memory_resource

    // --- таблица L : id = 1,2,3
    auto Ldata = make_operator_data(res);
    for (int i = 1; i <= 3; ++i) {
        auto d = document_t::make_document(res);
        d->set("id", i);
        d->set("valL", i*10);
        Ldata->append(d);
    }

    // --- таблица R : id = 2,3,4
    auto Rdata = make_operator_data(res);
    for (int i = 2; i <= 4; ++i) {
        auto d = document_t::make_document(res);
        d->set("id", i);
        d->set("valR", i*100);
        Rdata->append(d);
    }

    // Псевдо-context + два «leaf» операторa
    context_collection_t ctx {};
    auto left_leaf  = make_intrusive<read_only_operator_t>(&ctx, operator_type::data_source);
    left_leaf->set_output(Ldata);
    auto right_leaf = make_intrusive<read_only_operator_t>(&ctx, operator_type::data_source);
    right_leaf->set_output(Rdata);

    // Предикат id = id
    auto pred = expressions::make_compare_expression(
        expressions::compare_operator_type::EQUAL,
        expressions::make_scalar_identifier("id"),
        expressions::make_scalar_identifier("id"));

    // Merge-Join оператор
    auto merge = make_intrusive<operator_merge_join_t>(&ctx, std::move(pred));
    merge->set_left(left_leaf);
    merge->set_right(right_leaf);

    merge->on_execute(nullptr);

    REQUIRE(merge->output()->documents().size() == 2);  // id = 2,3
}
