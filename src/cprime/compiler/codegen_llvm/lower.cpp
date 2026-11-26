#include "lower.hpp"

#include <unordered_map>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <cprime/ast/ast.hpp>
#include <cprime/ast/walker.hpp>
#include <cprime/support/contract.hpp>

namespace cprime::codegen_llvm {

namespace {

template<typename Callback>
class VariableDeclFinder final : private ast::Walker<VariableDeclFinder<Callback>>
{
    friend class ast::Walker<VariableDeclFinder<Callback>>;

public:
    explicit VariableDeclFinder(Callback callback)
        : callback_{std::move(callback)}
    {
    }

    auto run(ast::FunctionDecl& function_decl) -> void
    {
        this->walk(function_decl);
    }

private:
    CPRIME_AST_WALK_EDGES(FunctionBody, BlockStmts, VariableDecls)

    auto post_visit(ast::VariableDecl& decl) -> void
    {
        callback_(decl);
    }

    Callback callback_;
};

class LowerToLlvmModule final
    : private ast::StmtVisitor<>
    , private ast::ExprVisitor<llvm::Value*>
{
    friend class ast::StmtVisitor<>;
    friend class ast::ExprVisitor<llvm::Value*>;

public:
    explicit LowerToLlvmModule(
        llvm::LLVMContext& llvm_context,
        llvm::Module& module,
        ast::AstContext& ast_context)
        : llvm_context_{&llvm_context}
        , module_{&module}
        , builder_{llvm_context}
        , ast_context_{&ast_context}
    {
    }

    auto lower() -> void
    {
        visit(ast_context_->translation_unit());
    }

private:
    auto visit(ast::TranslationUnit& translation_unit) -> void
    {
        for (ast::FunctionDecl& function_decl : translation_unit.functions()) {
            if (function_decl.name() == "main") {
                visit(function_decl);
                return;
            }
        }
        CPRIME_UNREACHABLE("main() function not found.");
    }

    auto visit(ast::FunctionDecl& function_decl) -> void
    {
        llvm::FunctionType* ir_func_signature = llvm::FunctionType::get(
            builder_.getInt32Ty(),
            /*isVarArg=*/false);
        llvm::Function* ir_func = llvm::Function::Create(
            ir_func_signature,
            llvm::Function::ExternalLinkage,
            function_decl.name(),
            module_);

        builder_.SetInsertPoint(
            llvm::BasicBlock::Create(*llvm_context_, "entry", ir_func));
        emit_local_variable_allocas(function_decl);
        visit(function_decl.body());

        if (!builder_.GetInsertBlock()->hasTerminator()) {
            builder_.CreateUnreachable();
        }
    }

    auto emit_local_variable_allocas(ast::FunctionDecl& function_decl) -> void
    {
        local_variable_slots_.clear();

        // Keep allocas in the entry block so LLVM can promote them to registers.
        VariableDeclFinder{[this](ast::VariableDecl& decl) {
            llvm::AllocaInst* slot = builder_.CreateAlloca(
                builder_.getInt32Ty(),
                nullptr,
                llvm::Twine{"var.", decl.name()});
            local_variable_slots_.emplace(&decl, slot);
        }}.run(function_decl);
    }

    auto visit(ast::Block& block) -> void
    {
        for (ast::Stmt& stmt : block.stmts()) {
            if (builder_.GetInsertBlock()->hasTerminator()) {
                break;
            }
            if (stmt.kind() == ast::NodeKind::VariableDeclStmt) {
                visit(static_cast<ast::VariableDeclStmt&>(stmt));
            } else {
                dispatch_stmt(stmt);
            }
        }
    }

    auto visit(ast::EmptyStmt&) -> void
    {
    }

    auto visit(ast::VariableDeclStmt& stmt) -> void
    {
        for (ast::VariableDecl& decl : stmt.decls()) {
            if (auto initializer = decl.initializer()) {
                llvm::Value* value = dispatch_expr(*initializer);
                builder_.CreateStore(value, get_local_variable_slot(decl));
            }
        }
    }

    auto visit(ast::ReturnStmt& return_stmt) -> void
    {
        ast::Expr& return_value = return_stmt.value();
        CPRIME_ASSERT(
            return_value.kind() == ast::NodeKind::IntegerExpr,
            "Currently only integers are allowed in return statement.");
        i64 return_value_i64 =
            static_cast<ast::IntegerExpr&>(return_value).value();
        i32 return_value_i32 = static_cast<i32>(return_value_i64);

        builder_.CreateRet(builder_.getInt32(return_value_i32));
    }

    auto visit(ast::ExprStmt& expr_stmt) -> void
    {
        std::ignore = dispatch_expr(expr_stmt.expr());
    }

    auto visit(ast::StringExpr& string_expr) -> llvm::Value*
    {
        CPRIME_WIP();
    }

    auto visit(ast::IntegerExpr& integer_expr) -> llvm::Value*
    {
        // Целочисленный литерал в C' имеет тип I32.
        return builder_.getInt32(static_cast<u32>(integer_expr.value()));
    }

    auto visit(ast::NameExpr& name_expr) -> llvm::Value*
    {
        ast::BindingDecl& binding = name_expr.resolved_binding();
        return builder_.CreateLoad(
            builder_.getInt32Ty(),
            get_local_variable_slot(binding),
            llvm::Twine{"load.", binding.name()});
    }

    auto visit(ast::CallExpr& call_expr) -> llvm::Value*
    {
        switch (call_expr.callee_kind()) {
        case ast::CalleeKind::BuiltinPrint:
            return emit_print_call(call_expr.args());
        case ast::CalleeKind::BuiltinPrintln:
            return emit_println_call(call_expr.args());
        case ast::CalleeKind::Function:
            CPRIME_WIP();
        }
        CPRIME_UNREACHABLE("Unknown callee kind.");
    }

    auto emit_println_call(ast::ExprList& args) -> llvm::Value*
    {
        emit_print_call(args);
        builder_.CreateCall(
            get_puts_ir_callee(),
            {
                builder_.CreateGlobalString(""),
            });

        return nullptr;
    }

    // TODO: refactor this method later
    auto emit_print_call(ast::ExprList& args) -> llvm::Value*
    {
        llvm::FunctionCallee printf_ir_callee = get_printf_ir_callee();
        for (ast::Expr& expr : args.exprs()) {
            switch (expr.kind()) {
            case ast::NodeKind::StringExpr: {
                auto& string_expr = static_cast<ast::StringExpr&>(expr);
                builder_.CreateCall(
                    printf_ir_callee,
                    {
                        builder_.CreateGlobalString("%s"),
                        builder_.CreateGlobalString(
                            std::string_view{string_expr.value()}),
                    });
                break;
            }

            default: {
                llvm::Value* value = dispatch_expr(expr);
                builder_.CreateCall(
                    printf_ir_callee,
                    {
                        builder_.CreateGlobalString("%d"),
                        value,
                    });
                break;
            }
            }
        }

        return nullptr;
    }

    auto get_printf_ir_callee() -> llvm::FunctionCallee
    {
        llvm::FunctionType* signature = llvm::FunctionType::get(
            builder_.getInt32Ty(),
            {builder_.getPtrTy()},
            /*isVarArg=*/true);
        return module_->getOrInsertFunction("printf", signature);
    }

    auto get_puts_ir_callee() -> llvm::FunctionCallee
    {
        llvm::FunctionType* signature = llvm::FunctionType::get(
            builder_.getInt32Ty(),
            {builder_.getPtrTy()},
            /*isVarArg=*/false);
        return module_->getOrInsertFunction("puts", signature);
    }

    auto visit(ast::UnaryExpr& unary_expr) -> llvm::Value*
    {
        llvm::Value* operand = dispatch_expr(unary_expr.operand());
        switch (unary_expr.op()) {
        case ast::UnaryOp::Minus:
            // Про флаг NSW смотрите комментарий в
            // LowerToLlvmModule::visit(ast::BinaryExpr&).
            return builder_.CreateNSWNeg(operand, "neg_tmp");
        case ast::UnaryOp::Increment:
            CPRIME_WIP();
        case ast::UnaryOp::Decrement:
            CPRIME_WIP();
        }
        CPRIME_UNREACHABLE("Unknown unary operator.");
    }

    auto visit(ast::BinaryExpr& binary_expr) -> llvm::Value*
    {
        // This is implementation of signed arithmetic operations.
        // TODO: move this implementation to separate emit_X function.

        llvm::Value* left = dispatch_expr(binary_expr.left());
        llvm::Value* right = dispatch_expr(binary_expr.right());

        // Согласно спецификации C' знаковое переполнение --- это неопределённое
        // поведение. Для передачи информации в LLVM о том, что знаковое
        // переполнение невозможно, используем флаг NSW для инструкций add, sub
        // и mul. Знаковое переполнение в sdiv и srem --- это и так
        // неопределённое поведение.
        switch (binary_expr.op()) {
        case ast::BinaryOp::Add:
            return builder_.CreateNSWAdd(left, right, "add_tmp");
        case ast::BinaryOp::Sub:
            return builder_.CreateNSWSub(left, right, "sub_tmp");
        case ast::BinaryOp::Mul:
            return builder_.CreateNSWMul(left, right, "mul_tmp");
        case ast::BinaryOp::Div:
            return builder_.CreateSDiv(left, right, "div_tmp");
        case ast::BinaryOp::Modulo:
            return builder_.CreateSRem(left, right, "mod_tmp");
        }
        CPRIME_UNREACHABLE("Unknown binary operator.");
    }

    auto get_local_variable_slot(ast::BindingDecl& binding) -> llvm::AllocaInst*
    {
        auto it = local_variable_slots_.find(&binding);
        CPRIME_ASSERT(it != local_variable_slots_.end(), "Local variable slot not found.");
        return it->second;
    }

private:
    llvm::LLVMContext* llvm_context_;
    llvm::Module* module_;
    llvm::IRBuilder<> builder_;

    ast::AstContext* ast_context_;
    std::unordered_map<ast::BindingDecl*, llvm::AllocaInst*> local_variable_slots_;
};

} // namespace

auto lower_to_llvm_ir(
    llvm::LLVMContext& llvm_context,
    ast::AstContext& ast_context)
    -> std::unique_ptr<llvm::Module>
{
    auto module = std::make_unique<llvm::Module>("main module", llvm_context);

    LowerToLlvmModule{llvm_context, *module, ast_context}.lower();

    // TODO: pass error output stream through params
    if (llvm::verifyModule(*module, &llvm::errs())) {
        CPRIME_UNREACHABLE("LLVM Module verification failed.");
    }

    return module;
}

} // namespace cprime::codegen_llvm
