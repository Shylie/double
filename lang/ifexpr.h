#pragma once

#include "expr.h"

class IfExprAST final : public ExprAST
{
public:
	IfExprAST(std::unique_ptr<ExprAST> condition, std::vector<std::unique_ptr<ExprAST>> thenblock, std::vector<std::unique_ptr<ExprAST>> elseblock);

	virtual llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	std::unique_ptr<ExprAST> condition;
	std::vector<std::unique_ptr<ExprAST>> thenblock, elseblock;
};