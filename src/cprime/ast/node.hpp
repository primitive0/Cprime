#ifndef CPRIME_AST_NODE_H_
#define CPRIME_AST_NODE_H_

#include <cprime/support/prelude.hpp>
#include <cprime/source/source_buffer.hpp>
#include <cprime/support/contract.hpp>

namespace cprime::ast {

// Метка для геттеров узла, которые дают дочерние узлы этого узла.
template<typename T>
using Own = T&;

// Метка для геттеров узла, которые дают ссылки на узлы в других местах дерева.
template<typename T>
using Ref = T&;

template<typename T>
class Opt;

template<typename T>
class Opt<T&> final
{
public:
    explicit(false) Opt()
        : value_{nullptr}
    {
    }

    explicit(false) Opt(T* value)
        : value_{value}
    {
    }

    auto operator*() const -> T&
    {
        return value();
    }

    auto operator->() const -> T*
    {
        return &value();
    }

    explicit operator bool() const
    {
        return has_value();
    }

private:
    auto value() const -> T&
    {
        CPRIME_ASSERT(has_value(), "Cannot access empty ast::Opt.");
        return *value_;
    }

    auto has_value() const -> bool
    {
        return value_ != nullptr;
    }

private:
    T* value_;
};

template<typename T>
class LateInit final
{
public:
    explicit LateInit()
        : value_{}
        , initialized_{false}
    {
    }

    auto operator*() const -> const T&
    {
        return value();
    }

    auto operator->() const -> const T*
    {
        return &value();
    }

    auto operator=(const T& value) -> LateInit&
    {
        CPRIME_ASSERT(!initialized_, "Value is already initialized.");
        value_ = value;
        initialized_ = true;
        return *this;
    }

private:
    auto value() const -> const T&
    {
        CPRIME_ASSERT(initialized_, "Value is not initialized yet.");
        return value_;
    }

private:
    T value_;
    bool initialized_;
};

enum class NodeKind {
    TranslationUnit,
    FunctionDecl,
    VariableDecl,
    Block,
    EmptyStmt,
    VariableDeclStmt,
    ReturnStmt,
    ExprStmt,
    ExprList,
    StringExpr,
    IntegerExpr,
    NameExpr,
    CallExpr,
    UnaryExpr,
    BinaryExpr,
};

class Node
{
public:
    // Подклассы Node создаются на арене, предоставленной AstContext.
    // Стандартные операторы new/delete лучше запретить, чтобы избежать ошибок
    // при работе с узлами AST.

    static auto operator new(std::size_t) -> void* = delete;
    static auto operator delete(void*) -> void = delete;

    static auto operator new[](std::size_t) -> void* = delete;
    static auto operator delete[](void*) -> void = delete;

public:
    Node(const Node&) = delete;
    auto operator=(const Node&) -> Node& = delete;

    Node(Node&&) = delete;
    auto operator=(Node&&) -> Node& = delete;

    auto kind() -> NodeKind
    {
        return kind_;
    }

    auto span() -> source::SourceSpan
    {
        return span_;
    }

protected:
    explicit Node(NodeKind kind, source::SourceSpan span)
        : kind_{kind}
        , span_{span}
    {
    }

    ~Node() = default;

private:
    NodeKind kind_;
    source::SourceSpan span_;
};

} // namespace cprime::ast

#endif // CPRIME_AST_NODE_H_
