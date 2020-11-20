#pragma once

#include "expr.h"

class UnaryExprAST final : public ExprAST
{
public:
	UnaryExprAST(char opcode, std::unique_ptr<ExprAST> operand);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

private:
	char opcode;
	std::unique_ptr<ExprAST> operand;
};