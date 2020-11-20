#include "varexpr.h"

VarExprAST::VarExprAST(std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> varnames) : varnames(std::move(varnames))
{
}

llvm::Value* VarExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	size_t e = varnames.size();

	for (size_t i = 0; i < e; i++)
	{
		if (namedValues[varnames[i].first])
		{
			fprintf(stderr, "cannot redeclare variabe '%s'\n", varnames[i].first.c_str());
			return nullptr;
		}
	}

	for (size_t i = 0; i < e; i++)
	{
		const std::string& name = varnames[i].first;
		ExprAST* init = varnames[i].second.get();

		llvm::Value* initval;
		if (init)
		{
			initval = init->Codegen(context, builder, module, namedValues);
			if (!initval) { return nullptr; }
		}
		else
		{
			initval = llvm::ConstantFP::get(context, llvm::APFloat(0.0));
		}

		llvm::AllocaInst* allocation = builder.CreateAlloca(llvm::Type::getDoubleTy(context), 0, name);
		builder.CreateStore(initval, allocation);

		namedValues[name] = allocation;
	}

	return nullptr;
}
