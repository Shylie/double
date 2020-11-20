#pragma once

#include "expr.h"

class VariableExprAST final : public ExprAST
{
public:
	VariableExprAST(const std::string& name);

	llvm::Value* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues) override;

	const std::string& GetName() const;

private:
	std::string name;
};