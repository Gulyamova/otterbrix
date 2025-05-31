#include "operator_merge_join.hpp"
#include <components/expressions/compare_expression.hpp>

namespace services::collection::operators {

using namespace components;

bool operator_merge_join_t::extract_key_(std::string& out)
{
    // Compare(EQUAL, Field("k"), Field("k"))                        
    auto* cmp = dynamic_cast<expressions::compare_expression_t*>(predicate_.get());
    if (!cmp || cmp->op() != expressions::compare_operator_type::EQUAL)
        return false;

    auto* left_id  = dynamic_cast<expressions::scalar_identifier_t*>(cmp->left().get());
    auto* right_id = dynamic_cast<expressions::scalar_identifier_t*>(cmp->right().get());
    if (!left_id || !right_id) return false;
    if (left_id->name() != right_id->name()) return false;

    out = left_id->name();
    return true;
}

void operator_merge_join_t::fallback_nested_loop_(pipeline::context_t* ctx)
{
    for (auto dl : left_->output()->documents()) {
        for (auto dr : right_->output()->documents()) {
            if (predicate_->check(dl, dr, ctx ? &ctx->parameters : nullptr)) {
                output_->append(document_t::merge(dl, dr, left_->output()->resource()));
            }
        }
    }
}

} // namespace services::collection::operators
