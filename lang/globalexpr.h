#pragma once

#include "expr.h"

class GlobalExprAST final : public ExprAST
{
public:
	GlobalExprAST(std::vector<std::pair<std::string, double>> vars);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	std::vector<std::pair<std::string, double>> vars;
};