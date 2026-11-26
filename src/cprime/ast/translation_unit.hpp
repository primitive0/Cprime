#ifndef CPRIME_AST_TRANSLATIONUNIT_H_
#define CPRIME_AST_TRANSLATIONUNIT_H_

#include "decl.hpp"
#include "list.hpp"
#include "node.hpp"
#include <cprime/support/prelude.hpp>
#include <cprime/source/source_buffer.hpp>

namespace cprime::ast {

class TranslationUnit final : public Node
{
public:
    explicit TranslationUnit(
        source::SourceSpan span,
        List<FunctionDecl> functions)
        : Node{NodeKind::TranslationUnit, span}
        , functions_{std::move(functions)}
    {
    }

    auto functions() -> List<FunctionDecl>&
    {
        return functions_;
    }

private:
    List<FunctionDecl> functions_;
};

} // namespace cprime::ast

#endif // CPRIME_AST_TRANSLATIONUNIT_H_
