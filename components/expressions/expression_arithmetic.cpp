#include "expression_arithmetic.hpp"
#include "expression_constant.hpp"
#include <sstream>
#include <stdexcept>

namespace components::expressions {

expression_arithmetic_t::expression_arithmetic_t(arithmetic_operator_t op,
                                                 expression_ptr left,
                                                 expression_ptr right)
    : expression_i(expression_group::arithmetic)
    , op_(op)
    , left_(std::move(left))
    , right_(std::move(right)) {}

arithmetic_operator_t expression_arithmetic_t::op() const { return op_; }
expression_ptr expression_arithmetic_t::left() const { return left_; }
expression_ptr expression_arithmetic_t::right() const { return right_; }

std::optional<literal_t> expression_arithmetic_t::try_fold_constant() const {
    auto l = std::dynamic_pointer_cast<expression_constant_t>(left_);
    auto r = std::dynamic_pointer_cast<expression_constant_t>(right_);

    if (!l || !r || l->value().type() != literal_type::integer || r->value().type() != literal_type::integer) {
        return std::nullopt;
    }

    int64_t lv = l->value().as_integer();
    int64_t rv = r->value().as_integer();

    switch (op_) {
        case arithmetic_operator_t::add: return literal_t{lv + rv};
        case arithmetic_operator_t::subtract: return literal_t{lv - rv};
        case arithmetic_operator_t::multiply: return literal_t{lv * rv};
        case arithmetic_operator_t::divide:
            if (rv == 0) return std::nullopt;
            return literal_t{lv / rv};
        default:
            return std::nullopt;
    }
}

hash_t expression_arithmetic_t::hash_impl() const {
    hash_t h = 0;
    boost::hash_combine(h, static_cast<int>(op_));
    boost::hash_combine(h, left_->hash());
    boost::hash_combine(h, right_->hash());
    return h;
}

std::string expression_arithmetic_t::to_string_impl() const {
    std::stringstream ss;
    ss << "(" << left_->to_string();
    switch (op_) {
        case arithmetic_operator_t::add: ss << " + "; break;
        case arithmetic_operator_t::subtract: ss << " - "; break;
        case arithmetic_operator_t::multiply: ss << " * "; break;
        case arithmetic_operator_t::divide: ss << " / "; break;
    }
    ss << right_->to_string() << ")";
    return ss.str();
}

bool expression_arithmetic_t::equal_impl(const expression_i* rhs) const {
    auto other = dynamic_cast<const expression_arithmetic_t*>(rhs);
    return other &&
           op_ == other->op_ &&
           *left_ == *other->left_ &&
           *right_ == *other->right_;
}

void expression_arithmetic_t::serialize_impl(serializer::base_serializer_t*) const {
    // TODO: Implement serialization
}

} // namespace components::expressions
