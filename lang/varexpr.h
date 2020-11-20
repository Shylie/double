#pragma once

#include "expr.h"

class VarExprAST final : public ExprAST
{
public:
	VarExprAST(std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> varnames);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> varnames;
};