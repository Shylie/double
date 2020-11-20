#include "numberexpr.h"

NumberExprAST::NumberExprAST(double value) : value(value)
{
}

llvm::Value* NumberExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	return llvm::ConstantFP::get(context, llvm::APFloat(value));
}