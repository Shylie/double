#include "callexpr.h"

CallExprAST::CallExprAST(const std::string& callee, std::vector<std::unique_ptr<ExprAST>> args) : callee(callee), args(std::move(args))
{
}

llvm::Value* CallExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	llvm::Function* calleeF = module.getFunction(callee);
	if (!calleeF)
	{
		fprintf(stderr, "unknown function %s\n", callee.c_str());
		return nullptr;
	}

	if (calleeF->arg_size() != args.size())
	{
		fprintf(stderr, "expected %d arguments, got %d\n", static_cast<int>(calleeF->arg_size()), static_cast<int>(args.size()));
		return nullptr;
	}

	std::vector<llvm::Value*> argsV;
	size_t size = args.size();
	for (size_t i = 0; i < size; i++)
	{
		argsV.push_back(args[i]->Codegen(context, builder, module, namedValues));
		if (!argsV.back()) { return nullptr; }
	}

	return builder.CreateCall(calleeF, argsV, "calltmp");
}