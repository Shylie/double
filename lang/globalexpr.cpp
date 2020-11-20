#include "globalexpr.h"

GlobalExprAST::GlobalExprAST(std::vector<std::pair<std::string, double>> vars) : vars(std::move(vars))
{
}

llvm::Value* GlobalExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	for (size_t i = 0; i < vars.size(); i++)
	{
		module.getOrInsertGlobal(vars[i].first, llvm::Type::getDoubleTy(context));
		llvm::GlobalVariable* gv = module.getGlobalVariable(vars[i].first);
		gv->setInitializer(llvm::ConstantFP::get(context, llvm::APFloat(vars[i].second)));
	}

	return nullptr;
}
