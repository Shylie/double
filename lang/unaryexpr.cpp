#include "unaryexpr.h"

UnaryExprAST::UnaryExprAST(char opcode, std::unique_ptr<ExprAST> operand) : opcode(opcode), operand(std::move(operand))
{
}

llvm::Value* UnaryExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	llvm::Value* operandV = operand->Codegen(context, builder, module, namedValues);
	if (!operandV) { return nullptr; }

	llvm::Function* f = module.getFunction(std::string("unary") + opcode);
	if (!f)
	{
		fprintf(stderr, "unknown unary operator '%s'\n", (std::string() + opcode).c_str());
		return nullptr;
	}

	return builder.CreateCall(f, operandV, std::string("unaryop") + opcode);
}
