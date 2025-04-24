#pragma once

#include "expression.hpp"
#include "expression_constant.hpp"
#include <components/types/literal.hpp>
#include <optional>

namespace components::expressions {

enum class arithmetic_operator_t {
    add,
    subtract,
    multiply,
    divide
};

class expression_arithmetic_t : public expression_i {
public:
    expression_arithmetic_t(arithmetic_operator_t op,
                            expression_ptr left,
                            expression_ptr right);

    arithmetic_operator_t op() const;
    expression_ptr left() const;
    expression_ptr right() const;

    std::optional<literal_t> try_fold_constant() const;

private:
    arithmetic_operator_t op_;
    expression_ptr left_;
    expression_ptr right_;

    hash_t hash_impl() const override;
    std::string to_string_impl() const override;
    bool equal_impl(const expression_i* rhs) const override;
    void serialize_impl(serializer::base_serializer_t* serializer) const override;
};

} // namespace components::expressions
