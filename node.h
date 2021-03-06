#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>
#include <string> 
#include <iostream> 
#include <sstream>
#include <stdio.h>









using namespace std; 

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class Node {
public:
	virtual ~Node() {}
	virtual llvm::Value* codeGen(CodeGenContext& context) { return NULL; }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
public:
	long long value;
	NInteger(long long value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NDouble : public NExpression {
public:
	double value;
	NDouble(double value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NString : public NExpression {
public:
	std::string value;
	NString(std::string value) : value(value) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NIdentifier : public NExpression{
public:
	std::string name;
	NIdentifier(const std::string& name) : name(name) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
public:
	const NIdentifier& id;
	ExpressionList arguments;
	NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
		id(id), arguments(arguments) { }
	NMethodCall(const NIdentifier& id) : id(id) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator : public NExpression {
public:
	int op;
	NExpression& lhs;
	NExpression& rhs;
	NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
		lhs(lhs), rhs(rhs), op(op) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NAssignment : public NExpression {
public:
	NIdentifier& lhs;
	NExpression& rhs;
	NAssignment(NIdentifier& lhs, NExpression& rhs) : 
		lhs(lhs), rhs(rhs) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NExpression {
public:
	StatementList statements;
	NBlock() { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExpressionStatement : public NStatement {
public:
	NExpression& expression;
	NExpressionStatement(NExpression& expression) : 
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NReturnStatement : public NStatement {
public:
	NExpression& expression;
	NReturnStatement(NExpression& expression) : 
		expression(expression) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration : public NStatement {
public:
	const NIdentifier& type;
	NIdentifier& id;
	NExpression *assignmentExpr;
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id) :
		type(type), id(id) { }
	NVariableDeclaration(const NIdentifier& type, NIdentifier& id, NExpression *assignmentExpr) :
		type(type), id(id), assignmentExpr(assignmentExpr) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NExternDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NExternDeclaration(const NIdentifier& type, const NIdentifier& id,
            const VariableList& arguments) :
        type(type), id(id), arguments(arguments) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NFunctionDeclaration : public NStatement {
public:
	const NIdentifier& type;
	const NIdentifier& id;
	VariableList arguments;
	NBlock& block;
	NFunctionDeclaration(const NIdentifier& type, const NIdentifier& id, 
			const VariableList& arguments, NBlock& block) :
		type(type), id(id), arguments(arguments), block(block) { }
	virtual llvm::Value* codeGen(CodeGenContext& context);
};

class BranchStatement: public NStatement {
public:
    NExpression* testExpression;
    NBlock blockTrue;
    NBlock blockFalse;
    bool hasFalseBranch;

    std::string getUniqueName(){
        char buffer[16];
        sprintf( buffer, "lab%d", instanceCount );
        instanceCount += 1;
        return buffer; 
    }
    BranchStatement( NExpression* test, NBlock& blockTrue, NBlock blockFalse ) :
        testExpression( test ), 
        blockTrue( blockTrue ), 
        blockFalse( blockFalse ), 
        hasFalseBranch(true) { }

    BranchStatement( NExpression* test, NBlock& blockTrue ) :
        testExpression( test ), 
        blockTrue( blockTrue ), 
        hasFalseBranch(false) { }

    
    
    virtual llvm::Value* codeGen(CodeGenContext& context);

private:
    static int instanceCount;
};

class ForStatement: public NStatement {
public:
	
    NExpression* Start;
    NExpression* End;
    NExpression* Step;
    NBlock Body;
   

    std::string getUniqueName(){
        char buffer[16];
        sprintf( buffer, "lab%d", instanceCount );
        instanceCount += 1;
        return buffer; 
    }
    ForStatement( NExpression* Start, NExpression* End, NExpression* Step, NBlock& Body ) :
        Start( Start ), 
        End( End ), 
        Step( Step ), 
        Body(Body) { }

    
    
    virtual llvm::Value* codeGen(CodeGenContext& context);

private:
    static int instanceCount;
};

class WhileStatement: public NStatement {
public:
	
    NExpression* Start;
    NBlock Body;
   

    std::string getUniqueName(){
        char buffer[16];
        sprintf( buffer, "lab%d", instanceCount );
        instanceCount += 1;
        return buffer; 
    }
    WhileStatement( NExpression* Start, NBlock& Body ) :
        Start( Start ), 
        Body(Body) { }

    
    
    virtual llvm::Value* codeGen(CodeGenContext& context);

private:
    static int instanceCount;
};