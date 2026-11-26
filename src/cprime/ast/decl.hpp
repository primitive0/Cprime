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

    auto body() -> Own<Block>
    {
        return *body_;
    }

private:
    std::string_view name_;
    Block* body_;
};

class BindingDecl : public Node
{
public:
    auto name() -> std::string_view
    {
        return name_;
    }

protected:
    explicit BindingDecl(NodeKind kind, source::SourceSpan span, std::string_view name)
        : Node{kind, span}
        , name_{name}
    {
    }

private:
    std::string_view name_;
};

class VariableDecl final
    : public BindingDecl
    , public ListHook
{
public:
    explicit VariableDecl(
        source::SourceSpan span,
        std::string_view name,
        Opt<Expr&> initializer)
        : BindingDecl{NodeKind::VariableDecl, span, name}
        , initializer_{initializer}
    {
    }

    auto initializer() -> Opt<Own<Expr>>
    {
        return initializer_;
    }

private:
    Opt<Expr&> initializer_;
};

// class ParameterDecl : public BindingDecl
// {
// };

} // namespace cprime::ast

#endif // CPRIME_AST_DECL_H_
