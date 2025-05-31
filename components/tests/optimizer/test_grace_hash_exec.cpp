#include <catch2/catch.hpp>
#include <components/physical_plan/collection/operators/operator_grace_hash_join.hpp>
#include <components/document/document.hpp>

static operator_data_ptr big_data(int from,int to)
{
    auto d = make_operator_data(nullptr);
    for(int i=from;i<=to;++i){auto doc=components::document::document_t::make_document(nullptr);
        doc->set("id",i); d->append(doc);}
    return d;
}

TEST_CASE("Grace-Hash join big inputs")
{
    context_collection_t ctx;
    auto L = make_intrusive<read_only_operator_t>(&ctx,operator_type::data_source);
    auto R = make_intrusive<read_only_operator_t>(&ctx,operator_type::data_source);
    L->set_output(big_data(1,200'000));
    R->set_output(big_data(150'000,350'000));

    auto pred = expressions::make_compare_expression(
        expressions::compare_operator_type::EQUAL,
        expressions::make_scalar_identifier("id"),
        expressions::make_scalar_identifier("id"));

    auto join = make_intrusive<operator_grace_hash_join_t>(&ctx,std::move(pred),100'000);
    join->set_left(L); join->set_right(R);
    join->on_execute(nullptr);

    // Пересечение 150k..200k  → 50 001 строк
    REQUIRE(join->output()->documents().size()==50'001);
}
