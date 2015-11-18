#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;
//int PrintfMethodCall::instanceCount = 0;
int BranchStatement::instanceCount = 0;

/* Compile the AST into a module */
void CodeGenContext::generateCode(NBlock& root)
{
	std::cout << "Generating code...\n";
	
	/* Create the top level interpreter function to call as entry */
	vector<Type*> argTypes;
	FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), makeArrayRef(argTypes), false);
	mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
	
	/* Push a new variable/block context */
	pushBlock(bblock);
	root.codeGen(*this); /* emit bytecode for the toplevel block */
	ReturnInst::Create(getGlobalContext(), bblock);
	popBlock();
	
	/* Print the bytecode in a human-readable format 
	   to see if our program compiled properly
	 */
	std::cout << "Code is generated.\n";
	PassManager pm;
	pm.add(createPrintModulePass(outs()));
	pm.run(*module);
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
	std::cout << "Running code...\n";
	ExecutionEngine *ee = EngineBuilder(module).create();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	std::cout << "Code was run.\n";
	return v;
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(const NIdentifier& type) 
{
	if (type.name.compare("entero") == 0) {
		return Type::getInt64Ty(getGlobalContext());
	}
	else if (type.name.compare("doble") == 0) {
		return Type::getDoubleTy(getGlobalContext());
	}
	else if (type.name.compare("cadena") == 0) {
		// Esto es para hacer las cadenas
		return  Type::getLabelTy(getGlobalContext());
	}
	return Type::getVoidTy(getGlobalContext());
}

/* -- Code Generation -- */
/* -- aqui es donde esta la magia*/

Value* NInteger::codeGen(CodeGenContext& context)
{
	std::cout << "Creating integer: " << value << endl;
	return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context)
{
	std::cout << "Creating double: " << value << endl;
	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* NString::codeGen(CodeGenContext& context)
{
	//En esta parte es para crear las cadenas, amigo estamos cerca de crear las
	std::cout << "Creating string: " << value << endl;
	return ConstantFP::get(Type::getLabelTy(getGlobalContext()), value);
}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
	std::cout << "Creating identifier reference: " << name << endl;
	if (context.locals().find(name) == context.locals().end()) {
		std::cerr << "undeclared variable " << name << endl;
		return NULL;
	}
	return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* NMethodCall::codeGen(CodeGenContext& context)
{
	Function *function = context.module->getFunction(id.name.c_str());
	if (function == NULL) {
		std::cerr << "no such function " << id.name << endl;
	}
	std::vector<Value*> args;
	ExpressionList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		args.push_back((**it).codeGen(context));
	}
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
	std::cout << "Creating method call: " << id.name << endl;
	return call;
}

Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
	std::cout << "Creating binary operation " << op << endl;
	Instruction::BinaryOps instr;
	switch (op) {
		case TPLUS:    return BinaryOperator::Create( Instruction::Add,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TMINUS:   return BinaryOperator::Create( Instruction::Sub,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TMUL:     return BinaryOperator::Create( Instruction::Mul,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TDIV:     return BinaryOperator::Create( Instruction::SDiv,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
		// Logical Operations
    	case TCEQ:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_EQ,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TCNE:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_NE,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TCLT:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SLT,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TCGT:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SGT,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TCLE:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SLE,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
    	case TCGE:  return  CmpInst::Create( Instruction::ICmp, CmpInst::ICMP_SGE,
    	        lhs.codeGen(context), rhs.codeGen(context), "", context.currentBlock());
	}

	return NULL;

}

Value* NAssignment::codeGen(CodeGenContext& context)
{
	std::cout << "Creating assignment for " << lhs.name << endl;
	if (context.locals().find(lhs.name) == context.locals().end()) {
		std::cerr << "undeclared variable " << lhs.name << endl;
		return NULL;
	}
	return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}

Value* NBlock::codeGen(CodeGenContext& context)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		std::cout << "Generating code for " << typeid(**it).name() << endl;
		last = (**it).codeGen(context);
	}
	std::cout << "Creating block" << endl;
	return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* NReturnStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Generating return code for " << typeid(expression).name() << endl;
	Value *returnValue = expression.codeGen(context);
	context.setCurrentReturnValue(returnValue);
	return returnValue;
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
	std::cout << "Creating variable declaration " << type.name << " " << id.name << endl;
	AllocaInst *alloc = new AllocaInst(typeOf(type), id.name.c_str(), context.currentBlock());
	context.locals()[id.name] = alloc;
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		assn.codeGen(context);
	}
	return alloc;
}

Value* NExternDeclaration::codeGen(CodeGenContext& context)
{
    vector<Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
    
    
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
    return function;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
	vector<Type*> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		argTypes.push_back(typeOf((**it).type));
	}
	FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);

	context.pushBlock(bblock);
	context.currentFunction = function;
	Function::arg_iterator argsValues = function->arg_begin();
    Value* argumentValue;

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).codeGen(context);
		
		argumentValue = argsValues++;
		argumentValue->setName((*it)->id.name.c_str());
		StoreInst *inst = new StoreInst(argumentValue, context.locals()[(*it)->id.name], false, bblock);
	}
	
	block.codeGen(context);
	ReturnInst::Create(getGlobalContext(), context.getCurrentReturnValue(), bblock);

	context.popBlock();
	std::cout << "Creating function: " << id.name << endl;
	context.currentFunction = context.mainFunction;
	return function;
}

/*======================================================================================================
|||||||||||||||||||||||||||||   	  |||||| 	      ||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||||||||||||||||||||| 	|||||||| 	||||||||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||||||||||||||||||||| 	|||||||| 	||||||||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||||||||||||||||||||| 	|||||||| 	     |||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||||||||||||||||||||| 	|||||||| 	||||||||||||||||||||||||||||||||||||||||||||||||||||||
||||||||||||||||||||||||||||||| 	|||||||| 	||||||||||||||||||||||||||||||||||||||||||||||||||||||
|||||||||||||||||||||||||||||   	  |||||| 	||||||||||||||||||||||||||||||||||||||||||||||||||||||
====================================================================================================*/


Value *BranchStatement::codeGen(CodeGenContext& context)
{
    cout<< "Generating code for if sen" << std::endl;

    IRBuilder<> builder(context.currentBlock());
    Value* test = testExpression->codeGen( context );
    
    Function *TheFunction = builder.GetInsertBlock()->getParent();

  
  
    BasicBlock *btrue = BasicBlock::Create(getGlobalContext(), getUniqueName(), TheFunction);
    BasicBlock *bfalse = NULL;
    
    if( hasFalseBranch ){
    	cout<<"------->Tiene else";
        bfalse = BasicBlock::Create(getGlobalContext(), getUniqueName(), TheFunction);
    }

    builder.CreateCondBr(test, btrue, bfalse);

    context.pushBlock(btrue);
    blockTrue.codeGen(context);
    context.popBlock();
 
    if( hasFalseBranch ){   
        context.pushBlock(bfalse);
        blockFalse.codeGen(context);
        context.popBlock();
    }
    return NULL;

    

//    Value *CondV = testExpression->codeGen(context);
//    IRBuilder<> Builder(context.currentBlock());
//
//    if (!CondV) return nullptr;
//    
//
//	Function *TheFunction = Builder.GetInsertBlock()->getParent();
//
//	BasicBlock *ThenBB =
//    BasicBlock::Create(getGlobalContext(), "then", TheFunction);
//	BasicBlock *ElseBB = BasicBlock::Create(getGlobalContext(), "else");
//	BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
//	
//	Builder.CreateCondBr(CondV, ThenBB, ElseBB);
//
//
//
//	// Emit then value.
//	Builder.SetInsertPoint(ThenBB);
//	
//	Value *ThenV = blockTrue.codeGen(context);
//	if (!ThenV)
//	  return nullptr;
//	
//	Builder.CreateBr(MergeBB);
//	// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
//	ThenBB = Builder.GetInsertBlock();
//	// Emit else block.
//	TheFunction->getBasicBlockList().push_back(ElseBB);
//	Builder.SetInsertPoint(ElseBB);
//	
//	Value *ElseV = blockFalse.codeGen(context);
//	if (!ElseV)
//	  return nullptr;
//	
//	Builder.CreateBr(MergeBB);
//	// codegen of 'Else' can change the current block, update ElseBB for the PHI.
//	ElseBB = Builder.GetInsertBlock();
//	// Emit merge block.
//	TheFunction->getBasicBlockList().push_back(MergeBB);
//	Builder.SetInsertPoint(MergeBB);
//	PHINode *PN =
//	   Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), 2, "iftmp");
//
// 	PN->addIncoming(ThenV, ThenBB);
//	PN->addIncoming(ElseV, ElseBB);
//
//	return PN;

}


/**/

