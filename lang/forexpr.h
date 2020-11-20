#pragma once

#include "expr.h"

class ForExprAST final : public ExprAST
{
public:
	ForExprAST(const std::string& varname, std::unique_ptr<ExprAST> start, std::unique_ptr<ExprAST> end, std::unique_ptr<ExprAST> step, std::vector<std::unique_ptr<ExprAST>> body);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	std::string varname;
	std::unique_ptr<ExprAST> start, end, step;
	std::vector<std::unique_ptr<ExprAST>> body;
};