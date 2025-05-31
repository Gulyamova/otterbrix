#include "operator_grace_hash_join.hpp"

namespace services::collection::operators {

void operator_grace_hash_join_t::on_execute_impl(pipeline::context_t* ctx)
{
    if (!left_||!right_||!left_->output()||!right_->output()) return;
    output_ = make_operator_data(left_->output()->resource());

    auto left_vec  = left_->output()->documents();
    auto right_vec = right_->output()->documents();

    // «Спиллинг»: делим на батчи batch_size_
    for (size_t lo=0; lo<left_vec.size();  lo += batch_size_) {
        size_t le = std::min(lo+batch_size_, left_vec.size());
        build_hash_table_(left_vec.begin()+lo, left_vec.begin()+le);

        for (size_t ro=0; ro<right_vec.size(); ro += batch_size_) {
            size_t re = std::min(ro+batch_size_, right_vec.size());
            probe_hash_table_(right_vec.begin()+ro, right_vec.begin()+re);
        }
        table_.clear();
    }
}

} // namespace services::collection::operators
