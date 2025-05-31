#include "operator_index_nested_loop_join.hpp"
#include <components/expressions/compare_expression.hpp>

namespace services::collection::operators {
using namespace components;

void operator_index_nested_loop_join_t::on_execute_impl(pipeline::context_t* ctx)
{
    if (!left_ || !right_ || !left_->output() || !right_->output()) return;
    output_ = make_operator_data(left_->output()->resource());

    std::string key;
    if (!extract_key_(key)) {   // непонятный предикат
        return operator_join_t::on_execute_impl(ctx);
    }

    // 1. Строим multi-index по правой таблице
    std::unordered_multimap<int64_t,doc_ptr> index;
    for (auto d : right_->output()->documents()) {
        index.emplace(d->get(key)->as_int64(), d);
    }

    // 2. Идём по левой и ищем совпадения
    for (auto dl : left_->output()->documents()) {
        auto val = dl->get(key)->as_int64();
        auto [it, end] = index.equal_range(val);
        for (; it!=end; ++it) {
            output_->append(document_t::merge(dl, it->second, left_->output()->resource()));
        }
    }
}

bool operator_index_nested_loop_join_t::extract_key_(std::string& out)
{
    auto* cmp = dynamic_cast<expressions::compare_expression_t*>(predicate_.get());
    if (!cmp || cmp->op()!=expressions::compare_operator_type::EQUAL) return false;
    auto* l = dynamic_cast<expressions::scalar_identifier_t*>(cmp->left().get());
    auto* r = dynamic_cast<expressions::scalar_identifier_t*>(cmp->right().get());
    if (!l||!r || l->name()!=r->name()) return false;
    out = l->name();  return true;
}

} // namespace services::collection::operators
