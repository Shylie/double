#include "forexpr.h"

ForExprAST::ForExprAST(const std::string& varname, std::unique_ptr<ExprAST> start, std::unique_ptr<ExprAST> end, std::unique_ptr<ExprAST> step, std::vector<std::unique_ptr<ExprAST>> body) : varname(varname), start(std::move(start)), end(std::move(end)), step(std::move(step)), body(std::move(body))
{
}

llvm::Value* ForExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	if (body.size() == 0) { return nullptr; }

	llvm::AllocaInst* allocation = builder.CreateAlloca(llvm::Type::getDoubleTy(context), 0, varname.c_str());

	llvm::Value* startV = start->Codegen(context, builder, module, namedValues);
	if (!startV) { return nullptr; }

	builder.CreateStore(startV, allocation);

	llvm::Function* fn = builder.GetInsertBlock()->getParent();
	llvm::BasicBlock* preheaderBB = builder.GetInsertBlock();
	llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(context, "loop", fn);

	builder.CreateBr(loopBB);

	builder.SetInsertPoint(loopBB);

	llvm::AllocaInst* shadowed = namedValues[varname];
	namedValues[varname] = allocation;

	std::remove_reference_t<decltype(namedValues)> bodytmp;
	for (auto pair : namedValues)
	{
		bodytmp[pair.first] = pair.second;
	}

	for (size_t i = 0; i < body.size(); i++)
	{
		body[i]->Codegen(context, builder, module, bodytmp);
	}

	llvm::Value* stepV = nullptr;

	if (step)
	{
		stepV = step->Codegen(context, builder, module, namedValues);
		if (!stepV) { return nullptr; }
	}
	else
	{
		stepV = llvm::ConstantFP::get(context, llvm::APFloat(1.0));
	}

	llvm::Value* curV = builder.CreateLoad(allocation);
	llvm::Value* nextV = builder.CreateFAdd(curV, stepV, "nextvar");
	builder.CreateStore(nextV, allocation);

	llvm::Value* endCondition = end->Codegen(context, builder, module, namedValues);
	if (!endCondition) { return nullptr; }

	endCondition = builder.CreateFCmpONE(endCondition, llvm::ConstantFP::get(context, llvm::APFloat(0.0)), "loopcond");

	llvm::BasicBlock* loopEndBB = builder.GetInsertBlock();
	llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(context, "afterloop", fn);

	builder.CreateCondBr(endCondition, loopBB, afterBB);

	builder.SetInsertPoint(afterBB);

	if (shadowed)
	{
		namedValues[varname] = shadowed;
	}
	else
	{
		namedValues.erase(varname);
	}

	return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(context));
}