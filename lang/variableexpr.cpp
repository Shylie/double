#include "variableexpr.h"

VariableExprAST::VariableExprAST(const std::string& name) : name(name)
{
}

llvm::Value* VariableExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	llvm::Value* v = namedValues[name];
	if (!v)
	{
		v = module.getGlobalVariable(name);

		if (!v)
		{
			fprintf(stderr, "unknown variable name %s\n", name.c_str());
			return nullptr;
		}
	}

	return builder.CreateLoad(v, name.c_str());
}

const std::string& VariableExprAST::GetName() const
{
	return name;
}