#pragma once

#include "expr.h"

class CallExprAST final : public ExprAST
{
public:
	CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	std::string callee;
	std::vector<std::unique_ptr<ExprAST>> args;
};