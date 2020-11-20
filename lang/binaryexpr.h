#pragma once

#include "expr.h"

class BinaryExprAST final : public ExprAST
{
public:
	BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	char op;
	std::unique_ptr<ExprAST> lhs, rhs;
};