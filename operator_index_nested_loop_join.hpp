#pragma once
#include "operator_join.hpp"
#include <unordered_multimap>

namespace services::collection::operators {

class operator_index_nested_loop_join_t final : public operator_join_t {
public:
    operator_index_nested_loop_join_t(context_collection_t* ctx,
                                      predicates::predicate_ptr p,
                                      type join_type = type::inner)
        : operator_join_t(ctx, join_type, std::move(p)) {}
private:
    using doc_ptr = components::document::document_ptr;
    void on_execute_impl(components::pipeline::context_t* ctx) override;
    bool extract_key_(std::string& out);        
};

} // namespace services::collection::operators
