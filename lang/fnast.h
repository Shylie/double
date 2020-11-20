#pragma once

#include "expr.h"

class PrototypeAST final
{
public:
	PrototypeAST(const std::string& name, std::vector<std::string> args, bool isOperator = false, unsigned int precedence = 0);

	llvm::Function* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues);
	const std::string& GetName() const;

	bool IsUnaryOperator() const;
	bool IsBinaryOperator() const;

	char GetOperatorName() const;

	unsigned int GetBinaryPrecedence() const;

private:
	std::string name;
	std::vector<std::string> args;
	bool isOperator;
	unsigned int precedence;
};

class FunctionAST final
{
public:
	FunctionAST(std::unique_ptr<PrototypeAST> proto, std::vector<std::unique_ptr<ExprAST>> body);

	llvm::Function* Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues, std::map<char, unsigned int>& binopPrec, llvm::legacy::FunctionPassManager& fpm);

private:
	std::unique_ptr<PrototypeAST> proto;
	std::vector<std::unique_ptr<ExprAST>> body;
};