#include "ifexpr.h"

IfExprAST::IfExprAST(std::unique_ptr<ExprAST> condition, std::vector<std::unique_ptr<ExprAST>> thenblock, std::vector<std::unique_ptr<ExprAST>> elseblock) : condition(std::move(condition)), thenblock(std::move(thenblock)), elseblock(std::move(elseblock))
{
}

llvm::Value* IfExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	if (thenblock.size() == 0 || elseblock.size() == 0) { return nullptr; }

	llvm::Value* conditionV = condition->Codegen(context, builder, module, namedValues);
	if (!conditionV) { return nullptr; }

	conditionV = builder.CreateFCmpONE(conditionV, llvm::ConstantFP::get(context, llvm::APFloat(0.0)), "ifcond");

	llvm::Function* fn = builder.GetInsertBlock()->getParent();

	llvm::BasicBlock* thenbb = llvm::BasicBlock::Create(context, "then", fn);
	llvm::BasicBlock* elsebb = llvm::BasicBlock::Create(context, "else");
	llvm::BasicBlock* mergebb = llvm::BasicBlock::Create(context, "ifcont");

	builder.CreateCondBr(conditionV, thenbb, elsebb);

	builder.SetInsertPoint(thenbb);

	std::remove_reference_t<decltype(namedValues)> thentmp;
	for (auto pair : namedValues)
	{
		thentmp[pair.first] = pair.second;
	}

	for (size_t i = 0; i < thenblock.size() - 1; i++)
	{
		thenblock[i]->Codegen(context, builder, module, thentmp);
	}

	llvm::Value* thenV = thenblock[thenblock.size() - 1]->Codegen(context, builder, module, thentmp);
	if (!thenV) { return nullptr; }

	builder.CreateBr(mergebb);

	thenbb = builder.GetInsertBlock();

	fn->getBasicBlockList().push_back(elsebb);

	builder.SetInsertPoint(elsebb);

	std::remove_reference_t<decltype(namedValues)> elsetmp;
	for (auto pair : namedValues)
	{
		elsetmp[pair.first] = pair.second;
	}

	for (size_t i = 0; i < elseblock.size() - 1; i++)
	{
		elseblock[i]->Codegen(context, builder, module, elsetmp);
	}

	llvm::Value* elseV = elseblock[elseblock.size() - 1]->Codegen(context, builder, module, elsetmp);
	if (!elseV) { return nullptr; }

	builder.CreateBr(mergebb);

	elsebb = builder.GetInsertBlock();

	fn->getBasicBlockList().push_back(mergebb);
	builder.SetInsertPoint(mergebb);

	llvm::PHINode* pn = builder.CreatePHI(llvm::Type::getDoubleTy(context), 2, "iftmp");

	pn->addIncoming(thenV, thenbb);
	pn->addIncoming(elseV, elsebb);
	return pn;
}