#include "optimizer.hpp"
#include "cost/estimator.hpp"
#include "rules/rule_set.hpp"
#include "utils/memo.hpp"  

namespace optimizer {
using namespace components::logical_plan;

static utils::Memo memo;  

node_ptr optimize(const node_ptr& node) {
    if (auto cached = memo.get(node)) {
        return *cached;
    }

    for (auto& child : node->children()) {
        child = optimize(child);
    }

    cost::estimate_node_output_rows(node);

    auto rewritten = rules::apply_all(node);

    memo.put(node, rewritten);

    return rewritten;
}

} // namespace optimizer
