#pragma once
#include "operator_hash_join.hpp"

namespace services::collection::operators {

/* Упрощённый Grace-Hash: просто обрабатывает вход пакетами,
   если строк > threshold. */
class operator_grace_hash_join_t final : public operator_hash_join_t {
public:
    operator_grace_hash_join_t(context_collection_t* c,
                               predicates::predicate_ptr p,
                               size_t batch = 100'000)
        : operator_hash_join_t(c,std::move(p)), batch_size_(batch) {}
private:
    size_t batch_size_;
    void on_execute_impl(components::pipeline::context_t* ctx) override;
};

} // namespace services::collection::operators
