#include "fnast.h"

PrototypeAST::PrototypeAST(const std::string& name, std::vector<std::string> args, bool isOperator, unsigned int precedence) : name(name), args(std::move(args)), isOperator(isOperator), precedence(precedence)
{
}

llvm::Function* PrototypeAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	if (module.getFunction(name))
	{
		fprintf(stderr, "cannot redefine function %s\n", name.c_str());
		return nullptr;
	}

	std::vector<llvm::Type*> doubles(args.size(), llvm::Type::getDoubleTy(context));
	llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), doubles, false);

	llvm::Function* f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, name, module);

	size_t idx = 0;
	for (auto& arg : f->args())
	{
		arg.setName(args[idx++]);
	}

	return f;
}

const std::string& PrototypeAST::GetName() const
{
	return name;
}

bool PrototypeAST::IsUnaryOperator() const
{
	return isOperator && args.size() == 1;
}

bool PrototypeAST::IsBinaryOperator() const
{
	return isOperator && args.size() == 2;
}

char PrototypeAST::GetOperatorName() const
{
	return name[name.length() - 1];
}

unsigned int PrototypeAST::GetBinaryPrecedence() const
{
	return precedence;
}

FunctionAST::FunctionAST(std::unique_ptr<PrototypeAST> proto, std::vector<std::unique_ptr<ExprAST>> body) : proto(std::move(proto)), body(std::move(body))
{
}

llvm::Function* FunctionAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues, std::map<char, unsigned int>& binopPrec, llvm::legacy::FunctionPassManager& fpm)
{
	if (body.size() == 0) { return nullptr; }

	llvm::Function* fn = module.getFunction(proto->GetName());
	if (!fn)
	{
		fn = proto->Codegen(context, builder, module, namedValues);
		if (!fn)
		{
			return nullptr;
		}
	}

	if (proto->IsBinaryOperator())
	{
		binopPrec[proto->GetOperatorName()] = proto->GetBinaryPrecedence();
	}

	llvm::BasicBlock* bb = llvm::BasicBlock::Create(context, "entry", fn);
	builder.SetInsertPoint(bb);

	std::remove_reference_t<decltype(namedValues)> temp;
	for (auto& arg : fn->args())
	{
		llvm::AllocaInst* allocation = builder.CreateAlloca(llvm::Type::getDoubleTy(context), 0, arg.getName());

		builder.CreateStore(&arg, allocation);

		temp[std::string(arg.getName())] = allocation;
	}

	for (size_t i = 0; i < body.size() - 1; i++)
	{
		body[i]->Codegen(context, builder, module, temp);
	}

	if (llvm::Value* retval = body[body.size() - 1]->Codegen(context, builder, module, temp))
	{
		builder.CreateRet(retval);

		if (llvm::verifyFunction(*fn)) { return nullptr; }

		fpm.run(*fn);

		if (llvm::verifyFunction(*fn)) { return nullptr; }

		return fn;
	}

	fn->eraseFromParent();
	return nullptr;
}