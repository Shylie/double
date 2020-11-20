#pragma once

#include "expr.h"

class NumberExprAST final : public ExprAST
{
public:
	NumberExprAST(double value);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	double value;
};