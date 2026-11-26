#ifndef CPRIME_AST_WALKER_H_
#define CPRIME_AST_WALKER_H_

#include "ast.hpp"
#include <concepts>
#include <magic_enum/magic_enum.hpp>
#include <cprime/support/prelude.hpp>
#include <cprime/support/contract.hpp>

namespace cprime::ast {

enum class EdgeKind {
    FunctionDecls,
    FunctionBody,
    VariableDecls,
    VariableInitializer,
    BlockStmts,
    ReturnValue,
    StmtExpr,
    ExprListItems,
    CallArgs,
    UnaryOperand,
    BinaryLeft,
    BinaryRight,
};

enum class WalkAction {
    // Продолжить обход дерева.
    Continue,

    // Продолжить обход дерева, но пропустить дочерние узлы текущего узла.
    SkipChildren,

    // Остановить обход дерева (выход с вызовом post_visit()).
    Stop,
};

#define CPRIME_AST_WALK_EDGES(...)                                                         \
    static consteval auto should_walk(::cprime::ast::EdgeKind edge) -> bool                \
    {                                                                                      \
        using enum ::cprime::ast::EdgeKind;                                                \
        for (auto allowed : std::initializer_list<::cprime::ast::EdgeKind>{__VA_ARGS__}) { \
            if (edge == allowed) {                                                         \
                return true;                                                               \
            }                                                                              \
        }                                                                                  \
        return false;                                                                      \
    }

template<typename Derived>
class Walker
{
protected:
    static constexpr WalkAction aContinue = WalkAction::Continue;
    static constexpr WalkAction aSkipChildren = WalkAction::SkipChildren;
    static constexpr WalkAction aStop = WalkAction::Stop;

    explicit Walker() = default;

    auto walk(Node& node) -> bool
    {
        static_assert(
            requires(Derived& derived, EdgeKind edge) {
                { derived.should_walk(edge) } -> std::same_as<bool>;
            },
            "Walker requires Derived::should_walk(EdgeKind) to return bool.");

        switch (node.kind()) {
        case NodeKind::TranslationUnit:
            return walk_node(static_cast<TranslationUnit&>(node));
        case NodeKind::FunctionDecl:
            return walk_node(static_cast<FunctionDecl&>(node));
        case NodeKind::VariableDecl:
            return walk_node(static_cast<VariableDecl&>(node));
        case NodeKind::Block:
            return walk_node(static_cast<Block&>(node));
        case NodeKind::EmptyStmt:
            return walk_node(static_cast<EmptyStmt&>(node));
        case NodeKind::VariableDeclStmt:
            return walk_node(static_cast<VariableDeclStmt&>(node));
        case NodeKind::ReturnStmt:
            return walk_node(static_cast<ReturnStmt&>(node));
        case NodeKind::ExprStmt:
            return walk_node(static_cast<ExprStmt&>(node));
        case NodeKind::ExprList:
            return walk_node(static_cast<ExprList&>(node));
        case NodeKind::StringExpr:
            return walk_node(static_cast<StringExpr&>(node));
        case NodeKind::IntegerExpr:
            return walk_node(static_cast<IntegerExpr&>(node));
        case NodeKind::NameExpr:
            return walk_node(static_cast<NameExpr&>(node));
        case NodeKind::CallExpr:
            return walk_node(static_cast<CallExpr&>(node));
        case NodeKind::UnaryExpr:
            return walk_node(static_cast<UnaryExpr&>(node));
        case NodeKind::BinaryExpr:
            return walk_node(static_cast<BinaryExpr&>(node));
        }
        CPRIME_UNREACHABLE(
            "Unknown AST node kind NodeKind::{}.",
            magic_enum::enum_name(node.kind()));
    }

    auto walk_children(TranslationUnit& translation_unit) -> bool
    {
        if (derived().should_walk(EdgeKind::FunctionDecls)) {
            for (FunctionDecl& decl : translation_unit.functions()) {
                if (!walk(decl)) {
                    return false;
                }
            }
        }
        return true;
    }

    auto walk_children(FunctionDecl& decl) -> bool
    {
        if (derived().should_walk(EdgeKind::FunctionBody)) {
            if (!walk(decl.body())) {
                return false;
            }
        }
        return true;
    }

    auto walk_children(Block& block) -> bool
    {
        if (derived().should_walk(EdgeKind::BlockStmts)) {
            for (Stmt& stmt : block.stmts()) {
                if (!walk(stmt)) {
                    return false;
                }
            }
        }
        return true;
    }

    auto walk_children(EmptyStmt&) -> bool
    {
        return true;
    }

    auto walk_children(VariableDeclStmt& stmt) -> bool
    {
        if (derived().should_walk(EdgeKind::VariableDecls)) {
            for (VariableDecl& decl : stmt.decls()) {
                if (!walk(decl)) {
                    return false;
                }
            }
        }
        return true;
    }

    auto walk_children(VariableDecl& decl) -> bool
    {
        if (derived().should_walk(EdgeKind::VariableInitializer)) {
            if (auto initializer = decl.initializer()) {
                if (!walk(*initializer)) {
                    return false;
                }
            }
        }
        return true;
    }

    auto walk_children(ReturnStmt& stmt) -> bool
    {
        if (derived().should_walk(EdgeKind::ReturnValue)) {
            if (!walk(stmt.value())) {
                return false;
            }
        }
        return true;
    }

    auto walk_children(ExprStmt& stmt) -> bool
    {
        if (derived().should_walk(EdgeKind::StmtExpr)) {
            if (!walk(stmt.expr())) {
                return false;
            }
        }
        return true;
    }

    auto walk_children(ExprList& list) -> bool
    {
        if (derived().should_walk(EdgeKind::ExprListItems)) {
            for (Expr& expr : list.exprs()) {
                if (!walk(expr)) {
                    return false;
                }
            }
        }
        return true;
    }

    auto walk_children(StringExpr&) -> bool
    {
        return true;
    }

    auto walk_children(IntegerExpr&) -> bool
    {
        return true;
    }

    auto walk_children(NameExpr&) -> bool
    {
        return true;
    }

    auto walk_children(CallExpr& expr) -> bool
    {
        if (derived().should_walk(EdgeKind::CallArgs)) {
            if (!walk(expr.args())) {
                return false;
            }
        }
        return true;
    }

    auto walk_children(UnaryExpr& expr) -> bool
    {
        if (derived().should_walk(EdgeKind::UnaryOperand)) {
            if (!walk(expr.operand())) {
                return false;
            }
        }
        return true;
    }

    auto walk_children(BinaryExpr& expr) -> bool
    {
        if (derived().should_walk(EdgeKind::BinaryLeft)) {
            if (!walk(expr.left())) {
                return false;
            }
        }
        if (derived().should_walk(EdgeKind::BinaryRight)) {
            if (!walk(expr.right())) {
                return false;
            }
        }
        return true;
    }

private:
    template<typename Node>
    static constexpr bool HasPreVisit = requires(Derived& derived, Node& node) {
        { derived.pre_visit(node) } -> std::same_as<WalkAction>;
    };

    template<typename Node>
    static constexpr bool HasPostVisit = requires(Derived& derived, Node& node) {
        derived.post_visit(node);
    };

    template<typename T>
    auto walk_node(T& node) -> bool
    {
        WalkAction action = aContinue;
        if constexpr (HasPreVisit<T>) {
            action = derived().pre_visit(node);
        }

        bool should_continue = false;
        switch (action) {
        case aContinue:
            should_continue = walk_children(node);
            break;
        case aSkipChildren:
            should_continue = true;
            break;
        case aStop:
            should_continue = false;
            break;
        }

        if constexpr (HasPostVisit<T>) {
            derived().post_visit(node);
        }

        return should_continue;
    }

    auto derived() -> Derived&
    {
        return static_cast<Derived&>(*this);
    }
};

} // namespace cprime::ast

#endif // CPRIME_AST_WALKER_H_
