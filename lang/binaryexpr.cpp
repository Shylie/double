#include "binaryexpr.h"
#include "variableexpr.h"

BinaryExprAST::BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs) : op(op), lhs(std::move(lhs)), rhs(std::move(rhs))
{
}

llvm::Value* BinaryExprAST::Codegen(llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<std::string, llvm::AllocaInst*>& namedValues)
{
	if (op == '=')
	{
		VariableExprAST* lhse = dynamic_cast<VariableExprAST*>(lhs.get());
		if (!lhse)
		{
			fprintf(stderr, "destination of '=' must be a variable\n");
			return nullptr;
		}

		llvm::Value* v = rhs->Codegen(context, builder, module, namedValues);
		if (!v) { return nullptr; }

		llvm::Value* variable = namedValues[lhse->GetName()];
		if (!variable)
		{
			variable = module.getGlobalVariable(lhse->GetName());

			if (!variable)
			{
				fprintf(stderr, "unknown variable name %s\n", lhse->GetName().c_str());
				return nullptr;
			}
		}

		builder.CreateStore(v, variable);
		return v;
	}

	llvm::Value* l = lhs->Codegen(context, builder, module, namedValues);
	llvm::Value* r = rhs->Codegen(context, builder, module, namedValues);

	if (!l || !r) { return nullptr; }

	switch (op)
	{
	case '+':
		return builder.CreateFAdd(l, r, "addtmp");

	case '-':
		return builder.CreateFSub(l, r, "subtmp");

	case '*':
		return builder.CreateFMul(l, r, "multmp");

	case '/':
		return builder.CreateFDiv(l, r, "divtmp");

	case '%':
		return builder.CreateFRem(l, r, "remtmp");
	
	case '<':
		l = builder.CreateFCmpULT(l, r, "cmptmp");
		return builder.CreateUIToFP(l, llvm::Type::getDoubleTy(context), "booltmp");

	case '>':
		l = builder.CreateFCmpUGT(l, r, "cmptmp");
		return builder.CreateUIToFP(l, llvm::Type::getDoubleTy(context), "booltmp");

	default:
		break;
	}

	llvm::Function* fn = module.getFunction(std::string("binary") + op);
	if (!fn)
	{
		fprintf(stderr, "binary operator '%s' not found\n", (std::string() + op).c_str());
		return nullptr;
	}

	llvm::Value* ops[2] = { l, r };
	return builder.CreateCall(fn, ops, std::string("binop") + op);
}