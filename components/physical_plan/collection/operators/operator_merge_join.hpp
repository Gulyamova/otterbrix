#pragma once

#include "operator_join.hpp"
#include <algorithm>

namespace services::collection::operators {

/** Лайт Merge-Join.
 *  Ограничения:
 *   • поддерживает predicate == 'Equal(fieldX, fieldX)';
 *   • работает только для INNER-join;
 *   • входы целиком сортируются в памяти.
 */
class operator_merge_join_t final : public operator_join_t {
public:
    operator_merge_join_t(context_collection_t* ctx,
                          predicates::predicate_ptr pred,
                          type join_type = type::inner)
        : operator_join_t(ctx, join_type, std::move(pred))
    {}

private:
    using doc_ptr = components::document::document_ptr;
    using vec_t   = std::vector<doc_ptr>;

    void on_execute_impl(components::pipeline::context_t* ctx) override
    {
        if (!left_ || !right_) return;
        if (!left_->output() || !right_->output()) return;

        // 1. Выделяем выход
        output_ = make_operator_data(left_->output()->resource());

        // 2. Копируем документ-указатели для сортировки
        vec_t left_docs  = left_->output()->documents();
        vec_t right_docs = right_->output()->documents();

        // 3. Вычисляем ключ-доступ из предиката
        std::string key_name;
        if (!extract_key_(key_name)) {
            /* неизвестный формат */
            return fallback_nested_loop_(ctx);
        }

        // 4. Сортировка
        auto cmp = [&](const doc_ptr& a, const doc_ptr& b) {
            return a->get(key_name)->as_int64() < b->get(key_name)->as_int64();
        };
        std::sort(left_docs.begin(),  left_docs.end(),  cmp);
        std::sort(right_docs.begin(), right_docs.end(), cmp);

        // 5. Слияние
        std::size_t i = 0, j = 0;
        while (i < left_docs.size() && j < right_docs.size()) {
            auto lv = left_docs[i]->get(key_name)->as_int64();
            auto rv = right_docs[j]->get(key_name)->as_int64();
            if (lv == rv) {
                output_->append(document_t::merge(left_docs[i], right_docs[j],
                                                  left_->output()->resource()));
                ++i; ++j;
            } else if (lv < rv) {
                ++i;
            } else {
                ++j;
            }
        }
    }

    bool extract_key_(std::string& out);
    void fallback_nested_loop_(components::pipeline::context_t* ctx);
};

} // namespace services::collection::operators
