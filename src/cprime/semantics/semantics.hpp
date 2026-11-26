#ifndef CPRIME_SEMANTICS_SEMANTICS_H_
#define CPRIME_SEMANTICS_SEMANTICS_H_

#include <cprime/ast/ast_context.hpp>
#include <cprime/diagnostics/diagnostic_sink.hpp>

namespace cprime::semantics {

[[nodiscard]]
auto check_cprime_semantics(
    ast::AstContext& ast_context,
    diagnostics::IDiagnosticSink& diagnostic_sink)
    -> bool;

} // namespace cprime::semantics

#endif // CPRIME_SEMANTICS_SEMANTICS_H_
