#include <ostream>
#include <sstream>
#include <cprime/support/prelude.hpp>
#include <cprime/ast/ast.hpp>
#include <cprime/ast/dump.hpp>
#include <cprime/diagnostics/diagnostic.hpp>
#include <cprime/diagnostics/diagnostic_recorder.hpp>
#include <cprime/parse/parse.hpp>
#include <cprime/semantics/semantics.hpp>
#include <cprime/source/source_buffer.hpp>
#include <cprime/support/contract.hpp>
#include <cprime/testing/file_test.hpp>

namespace cprime::semantics {

using ast::AstContext;
using diagnostics::Diagnostic;
using diagnostics::DiagnosticRecorder;
using diagnostics::Severity;
using source::LineColumn;
using source::SourceBuffer;

class SemanticsFileTest final : public testing::FileTest
{
public:
    explicit SemanticsFileTest(std::string_view exe_name)
        : FileTest{"semantics", "src/cprime/semantics/testdata/", exe_name}
    {
    }

    auto process(std::string_view input) -> std::string override
    {
        auto source_buffer = SourceBuffer::from_text(input);

        AstContext ast_context{};
        DiagnosticRecorder diagnostics{};
        bool success = parse::parse_cprime_source(
            ast_context, diagnostics, *source_buffer);

        CPRIME_ASSERT(success, "Failed to parse file.");
        CPRIME_ASSERT(!diagnostics.has_any(), "File parsed with warnings.");

        success = semantics::check_cprime_semantics(
            ast_context, diagnostics);

        std::ostringstream oss{};
        if (success) {
            oss << "SUCCESS\n\n";
        } else {
            oss << "ERROR\n\n";
            dump_diagnostics(oss, diagnostics);
        }
        return oss.str();
    }

private:
    static auto dump_diagnostics(
        std::ostream& os,
        const DiagnosticRecorder& diagnostics) -> void
    {
        for (const Diagnostic& diagnostic : diagnostics) {
            switch (diagnostic.severity) {
            case Severity::Warning:
                os << "Warning ";
                break;
            case Severity::Error:
                os << "Error ";
                break;
            }
            LineColumn location = diagnostic.span.location();
            os << "<" << location.line << ":" << location.column << "> ";
            os << "\"" << diagnostic.message << "\"\n";
        }
    }
};

} // namespace cprime::semantics

auto main(int argc, char** argv) -> int
{
    // TODO: get exe name from args?
    return cprime::semantics::SemanticsFileTest{"cprime_semantics_file_test"}
        .run(argc, argv);
}
