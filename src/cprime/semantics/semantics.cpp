#include "semantics.hpp"

#include <format>
#include <unordered_map>
#include <cprime/ast/ast.hpp>
#include <cprime/ast/walker.hpp>

namespace cprime::semantics {

class ResolveNamesPass final : private ast::Walker<ResolveNamesPass>
{
    friend class ast::Walker<ResolveNamesPass>;

public:
    explicit ResolveNamesPass(
        ast::AstContext& ast_context,
        diagnostics::IDiagnosticSink& diagnostic_sink)
        : ast_context_{&ast_context}
        , diagnostic_sink_{&diagnostic_sink}
        , bindings_{}
        , errors_found_{false}
    {
    }

    auto run() -> bool
    {
        walk(ast_context_->translation_unit());
        return !errors_found_;
    }

private:
    CPRIME_AST_WALK_EDGES(
        FunctionDecls,
        FunctionBody,
        BlockStmts,
        VariableDecls,
        VariableInitializer,
        ReturnValue,
        StmtExpr,
        CallArgs,
        ExprListItems,
        UnaryOperand,
        BinaryLeft,
        BinaryRight)

    auto post_visit(ast::FunctionDecl& /*function_decl*/) -> void
    {
        bindings_.clear();
    }

    auto post_visit(ast::VariableDecl& variable_decl) -> void
    {
        auto [it, inserted] = bindings_.insert({variable_decl.name(), &variable_decl});
        if (!inserted) {
            errors_found_ = true;
            diagnostic_sink_->emit_error(
                std::format("redeclaration of '{}'", variable_decl.name()),
                variable_decl.span());
            // TODO: add 'note' diagnostic about other declaration
        }
    }

    auto post_visit(ast::NameExpr& name_expr) -> void
    {
        auto it = bindings_.find(name_expr.name());
        if (it == bindings_.cend()) {
            errors_found_ = true;
            diagnostic_sink_->emit_error(
                std::format("use of undeclared identifier '{}'", name_expr.name()),
                name_expr.span());
            return;
        }
        name_expr.set_resolved_binding(*it->second);
    }

private:
    ast::AstContext* ast_context_;
    diagnostics::IDiagnosticSink* diagnostic_sink_;

    std::unordered_map<std::string_view, ast::BindingDecl*> bindings_;

    bool errors_found_;
};

auto check_cprime_semantics(
    ast::AstContext& ast_context,
    diagnostics::IDiagnosticSink& diagnostic_sink)
    -> bool
{
    return ResolveNamesPass{ast_context, diagnostic_sink}.run();
}

} // namespace cprime::semantics
