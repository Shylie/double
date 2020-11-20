#include "lexer.h"

#include "expr.h"
#include "binaryexpr.h"
#include "callexpr.h"
#include "ifexpr.h"
#include "forexpr.h"
#include "globalexpr.h"
#include "numberexpr.h"
#include "varexpr.h"
#include "variableexpr.h"
#include "unaryexpr.h"
#include "fnast.h"

#include <cstdio>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>

static Token current;
static Lexer lexer;
static Token GetNextToken()
{
	current = lexer.Next();
	return current;
}

static std::map<char, unsigned int> binopPrec;

static int GetTokenPrecedence()
{
	if (!isascii(current.type)) { return -1; }

	int tokPrec = binopPrec[current.type];
	if (tokPrec <= 0) { return -1; }
	return tokPrec;
}

static std::unique_ptr<ExprAST> ParseExpression(bool ifConsumeEnd = true);

static std::unique_ptr<ExprAST> ParseNumberExpr()
{
	auto result = std::make_unique<NumberExprAST>(current.number);
	GetNextToken();
	return std::move(result);
}

static std::unique_ptr<ExprAST> ParseParenExpr()
{
	GetNextToken();
	auto v = ParseExpression();
	if (!v) { return nullptr; }

	if (current.type != ')')
	{
		fprintf(stderr, "expected ')'\n");
		return nullptr;
	}
	GetNextToken();
	return v;
}

static std::unique_ptr<ExprAST> ParseIdentifierExpr()
{
	std::string name = current.identifier;

	GetNextToken();

	if (current.type != '(') { return std::make_unique<VariableExprAST>(name); }

	GetNextToken();

	std::vector<std::unique_ptr<ExprAST>> args;
	if (current.type != ')')
	{
		while (true)
		{
			if (auto arg = ParseExpression())
			{
				args.push_back(std::move(arg));
			}
			else
			{
				return nullptr;
			}

			if (current.type == ')') { break; }

			if (current.type != ',')
			{
				fprintf(stderr, "expected ')' or ',' in argument list\n");
				return nullptr;
			}
			GetNextToken();
		}
	}

	GetNextToken();

	return std::make_unique<CallExprAST>(name, std::move(args));
}

static std::unique_ptr<ExprAST> ParseIfExpr(bool consumeEnd)
{
	GetNextToken();

	std::unique_ptr<ExprAST> condition = ParseExpression();
	if (!condition) { return nullptr; }

	if (current.type != Token::Then)
	{
		fprintf(stderr, "expected 'then' after condition\n");
		return nullptr;
	}
	GetNextToken();

	std::vector<std::unique_ptr<ExprAST>> thenblock;
	while (current.type != Token::Else && current.type != Token::ElseIf && current.type != '\0')
	{
		thenblock.push_back(std::move(ParseExpression()));
		if (!thenblock.back()) { return nullptr; }

		if (current.type == ';') { GetNextToken(); }
	}

	std::vector<std::unique_ptr<ExprAST>> elseblock;
	if (current.type != Token::Else && current.type != Token::ElseIf)
	{
		fprintf(stderr, "expected 'else' or 'elseif' to close then block\n");
		return nullptr;
	}
	else
	{
		if (current.type == Token::ElseIf)
		{
			current = Token::If;
			elseblock.push_back(std::move(ParseExpression(false)));
			if (!elseblock.back()) { return nullptr; }
		}
		else
		{
			GetNextToken();
		}
	}

	while (current.type != Token::End && current.type != '\0')
	{
		elseblock.push_back(std::move(ParseExpression()));
		if (!elseblock.back()) { return nullptr; }

		if (current.type == ';') { GetNextToken(); }
	}

	if (current.type != Token::End)
	{
		fprintf(stderr, "expected 'end' to close else block\n");
		return nullptr;
	}
	else
	{
		if (consumeEnd) { GetNextToken(); }
	}

	return std::make_unique<IfExprAST>(std::move(condition), std::move(thenblock), std::move(elseblock));
}

static std::unique_ptr<ExprAST> ParseForExpr()
{
	GetNextToken();

	if (current.type != Token::Identifier)
	{
		fprintf(stderr, "expected identifier after for\n");
		return nullptr;
	}

	std::string name = current.identifier;
	GetNextToken();

	if (current.type != '=')
	{
		fprintf(stderr, "expected '=' after for\n");
		return nullptr;
	}
	GetNextToken();

	auto start = ParseExpression();
	if (!start) { return nullptr; }

	if (current.type != ',')
	{
		fprintf(stderr, "expected ',' after for start value\n");
		return nullptr;
	}
	GetNextToken();

	auto end = ParseExpression();
	if (!end) { return nullptr; }

	std::unique_ptr<ExprAST> step = nullptr;
	if (current.type == ',')
	{
		GetNextToken();
		step = std::move(ParseExpression());
		if (!step) { return nullptr; }
	}

	if (current.type != Token::Do)
	{
		fprintf(stderr, "expected 'do' after for\n");
		return nullptr;
	}
	GetNextToken();

	std::vector<std::unique_ptr<ExprAST>> body;
	while (current.type != Token::End && current.type != '\0')
	{
		body.push_back(std::move(ParseExpression()));
		if (!body.back()) { return nullptr; }

		if (current.type == ';') { GetNextToken(); }
	}

	if (current.type != Token::End)
	{
		fprintf(stderr, "expected 'end' to close for loop\n");
		return nullptr;
	}
	else
	{
		GetNextToken();
	}

	return std::make_unique<ForExprAST>(name, std::move(start), std::move(end), std::move(step), std::move(body));
}

static std::unique_ptr<ExprAST> ParseVarExpr()
{
	GetNextToken();

	std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> varnames;

	if (current.type != Token::Identifier)
	{
		fprintf(stderr, "expected identifier after var\n");
		return nullptr;
	}

	while (true)
	{
		std::string name = current.identifier;
		GetNextToken();

		std::unique_ptr<ExprAST> init;
		if (current.type == '=')
		{
			GetNextToken();

			init = ParseExpression();
			if (!init) { return nullptr; }
		}

		varnames.push_back(std::make_pair(name, std::move(init)));

		if (current.type != ',') { break; }
		GetNextToken();

		if (current.type != Token::Identifier)
		{
			fprintf(stderr, "expected identifier list after var\n");
			return nullptr;
		}
	}

	if (current.type != ';')
	{
		fprintf(stderr, "expected ';' keyword after var\n");
		return nullptr;
	}
	GetNextToken();

	return std::make_unique<VarExprAST>(std::move(varnames));
}

static std::unique_ptr<ExprAST> ParsePrimary(bool ifConsumeEnd = true)
{
	switch (current.type)
	{
	case Token::Identifier:
		return ParseIdentifierExpr();

	case Token::Number:
		return ParseNumberExpr();

	case Token::If:
		return ParseIfExpr(ifConsumeEnd);

	case Token::For:
		return ParseForExpr();

	case Token::Var:
		return ParseVarExpr();

	case '(':
		return ParseParenExpr();

	default:
		fprintf(stderr, "unknown token %d, expected expression\n", current.type);
		return nullptr;
	}
}

static std::unique_ptr<ExprAST> ParseUnary(bool ifConsumeEnd = true)
{
	if (!isascii(current.type) || current.type == '(' || current.type == ',') { return ParsePrimary(ifConsumeEnd); }

	int opc = current.type;
	GetNextToken();

	if (auto operand = ParseUnary(ifConsumeEnd))
	{
		return std::make_unique<UnaryExprAST>(opc, std::move(operand));
	}
	else
	{
		return nullptr;
	}
}

static std::unique_ptr<ExprAST> ParseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> lhs)
{
	while (true)
	{
		int tokenPrec = GetTokenPrecedence();

		if (tokenPrec < exprPrec) { return lhs; }

		int binOp = current.type;
		GetNextToken();

		auto rhs = ParseUnary();
		if (!rhs) { return nullptr; }

		int nextPrec = GetTokenPrecedence();
		if (tokenPrec < nextPrec)
		{
			rhs = ParseBinOpRHS(tokenPrec + 1, std::move(rhs));
			if (!rhs) { return nullptr; }
		}

		lhs = std::make_unique<BinaryExprAST>(binOp, std::move(lhs), std::move(rhs));
	}
}

static std::unique_ptr<ExprAST> ParseExpression(bool ifConsumeEnd)
{
	auto lhs = ParseUnary(ifConsumeEnd);

	if (!lhs) { return nullptr; }

	return ParseBinOpRHS(0, std::move(lhs));
}

static std::unique_ptr<ExprAST> ParseGlobal()
{
	GetNextToken();

	std::vector<std::pair<std::string, double>> vars;
	while (current.type == Token::Identifier)
	{
		std::string varname = current.identifier;
		double varinit = 0;
		GetNextToken();

		if (current.type == '=')
		{
			GetNextToken();

			bool negate = false;
			if (current.type == '-')
			{
				negate = true;
				GetNextToken();
			}

			if (current.type != Token::Number)
			{
				fprintf(stderr, "expected number after '='\n");
				return nullptr;
			}
			
			varinit = (negate ? -current.number : current.number);

			GetNextToken();
		}

		vars.push_back(std::make_pair(varname, varinit));

		if (current.type == ',') { GetNextToken(); }
	}

	if (current.type != ';')
	{
		fprintf(stderr, "expected ';' after global variable declarations\n");
		return nullptr;
	}

	GetNextToken();

	return std::make_unique<GlobalExprAST>(std::move(vars));
}

static std::unique_ptr<PrototypeAST> ParsePrototype()
{
	std::string fnName;

	unsigned int kind = 0;
	unsigned int binaryPrec = 30;

	switch (current.type)
	{
	case Token::Identifier:
		fnName = current.identifier;
		kind = 0;
		GetNextToken();
		break;

	case Token::Unary:
		GetNextToken();
		if (!isascii(current.type))
		{
			fprintf(stderr, "expected unary operator\n");
			return nullptr;
		}
		fnName = "unary";
		fnName += static_cast<char>(current.type);
		kind = 1;
		GetNextToken();
		break;

	case Token::Binary:
		GetNextToken();
		if (!isascii(current.type))
		{
			fprintf(stderr, "expected binary operator\n");
			return nullptr;
		}
		fnName = "binary";
		fnName += static_cast<char>(current.type);
		kind = 2;
		GetNextToken();

		if (current.type == Token::Number)
		{
			if (current.number < 1 || current.number > 100)
			{
				fprintf(stderr, "precedence must be [1, 100], got %d\n", static_cast<int>(current.number));
				return nullptr;
			}
			binaryPrec = static_cast<unsigned int>(current.number);
			GetNextToken();
		}
		break;

	default:
		fprintf(stderr, "expected function name in prototype\n");
		break;
	}

	if (current.type != '(')
	{
		fprintf(stderr, "expected '(' in prototype\n");
		return nullptr;
	}

	std::vector<std::string> argnames;

	while (GetNextToken(), current.type == Token::Identifier || current.type == ',')
	{
		if (current.type == Token::Identifier)
		{
			argnames.push_back(current.identifier);
		}
	}

	if (current.type != ')')
	{
		fprintf(stderr, "expected ')' in prototype, %d\n", current.type);
		return nullptr;
	}

	GetNextToken();

	if (kind && argnames.size() != kind)
	{
		fprintf(stderr, "invalid number of operands for %s operator\n", kind == 1 ? "unary" : "binary");
		return nullptr;
	}

	return std::make_unique<PrototypeAST>(fnName, std::move(argnames), kind != 0, binaryPrec);
}

static std::unique_ptr<FunctionAST> ParseDefinition()
{
	GetNextToken();
	auto proto = ParsePrototype();
	if (!proto) { return nullptr; }

	std::vector<std::unique_ptr<ExprAST>> es;

	while (current.type != Token::End && current.type != '\0')
	{
		es.push_back(std::move(ParseExpression()));
		if (!es.back()) { return nullptr; }

		if (current.type == ';') { GetNextToken(); }
	}
	
	if (current.type != Token::End)
	{
		fprintf(stderr, "expected 'end' to close definition of function %s\n", proto->GetName().c_str());
		return nullptr;
	}
	else
	{
		GetNextToken();
	}

	return std::make_unique<FunctionAST>(std::move(proto), std::move(es));
}

static std::unique_ptr<PrototypeAST> ParseExtern()
{
	GetNextToken();
	return ParsePrototype();
}

static llvm::LLVMContext gContext;
static llvm::IRBuilder<> gBuilder(gContext);
static std::unique_ptr<llvm::Module> gModule;
static std::map<std::string, llvm::AllocaInst*> gNamedValues;
static std::unique_ptr<llvm::legacy::FunctionPassManager> gPass;

static void HandleDefinition()
{
	if (auto fnast = ParseDefinition())
	{
		if (auto* fnir = fnast->Codegen(gContext, gBuilder, *gModule, gNamedValues, binopPrec, *gPass))
		{

		}
		else
		{
			GetNextToken();
		}
	}
}

static void HandleExtern()
{
	if (auto protoast = ParseExtern())
	{
		if (auto* fnir = protoast->Codegen(gContext, gBuilder, *gModule, gNamedValues))
		{
		}
		else
		{
			GetNextToken();
		}
	}
}

static void HandleGlobal()
{
	if (auto globalast = ParseGlobal())
	{
		globalast->Codegen(gContext, gBuilder, *gModule, gNamedValues);
	}
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: lang [file]\n");
		return 1;
	}

	binopPrec['='] = 2;
	binopPrec['<'] = 10;
	binopPrec['>'] = 10;
	binopPrec['+'] = 20;
	binopPrec['-'] = 20;
	binopPrec['%'] = 40;
	binopPrec['*'] = 40;
	binopPrec['/'] = 40;

	gModule = std::make_unique<llvm::Module>("lang", gContext);
	gPass = std::make_unique<llvm::legacy::FunctionPassManager>(gModule.get());

	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();

	auto targetTriple = llvm::sys::getDefaultTargetTriple();
	gModule->setTargetTriple(targetTriple);

	std::string err;
	auto target = llvm::TargetRegistry::lookupTarget(targetTriple, err);

	if (!target)
	{
		llvm::errs() << err;
		return 1;
	}

	auto cpu = "generic";
	auto features = "";

	llvm::TargetOptions opt;
	auto rm = llvm::Optional<llvm::Reloc::Model>();
	auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, rm);

	gModule->setDataLayout(targetMachine->createDataLayout());

	auto filename = std::string(argv[1]) + ".obj";

	std::error_code ec;
	llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

	if (ec)
	{
		llvm::errs() << "Could not open file: " << ec.message();
		return 1;
	}

	gPass->add(llvm::createPromoteMemoryToRegisterPass());
	gPass->add(llvm::createInstructionCombiningPass());
	gPass->add(llvm::createDeadCodeEliminationPass());
	gPass->add(llvm::createReassociatePass());
	gPass->add(llvm::createNewGVNPass());
	gPass->add(llvm::createCFGSimplificationPass());

	gPass->doInitialization();

	llvm::legacy::PassManager outpass;
	auto filetype = llvm::CGFT_ObjectFile;

	if (targetMachine->addPassesToEmitFile(outpass, dest, nullptr, filetype))
	{
		llvm::errs() << "target can't emit a file of this type\n";
		return 1;
	}
	
	std::ifstream ifs(argv[1]);
	if (ifs.good())
	{
		lexer = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

		GetNextToken();

		bool quit = false;
		while (!quit)
		{
			switch (current.type)
			{
			case ';':
				GetNextToken();
				break;

			case Token::Def:
				HandleDefinition();
				break;

			case Token::Extern:
				HandleExtern();
				break;

			case Token::Var:
				HandleGlobal();
				break;

			default:
				quit = true;
				break;
			}
		}
		
		outpass.run(*gModule);
		dest.flush();

		return 0;
	}
	else
	{
		fprintf(stderr, "Could not open file %s\n", argv[1]);
		return 1;
	}
}