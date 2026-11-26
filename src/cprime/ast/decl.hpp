#ifndef CPRIME_AST_DECL_H_
#define CPRIME_AST_DECL_H_

#include "list.hpp"
#include "node.hpp"
#include "stmt.hpp"
#include <cprime/support/prelude.hpp>
#include <cprime/source/source_buffer.hpp>

namespace cprime::ast {

class FunctionDecl final
    : public Node
    , public ListHook
{
public:
    explicit FunctionDecl(
        source::SourceSpan span,
        std::string_view name,
        Block& body)
        : Node{NodeKind::FunctionDecl, span}
        , name_{name}
        , body_{&body}
    {
    }

    auto name() -> std::string_view
    {
        return name_;
    }

    auto body() -> Block&
    {
        return *body_;
    }

private:
    std::string_view name_;
    Block* body_;
};

} // namespace cprime::ast

#endif // CPRIME_AST_DECL_H_
