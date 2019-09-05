#include "interruptM.h"

Interrupt::Interrupt()
{

}

void Interrupt::setName(string name)
{
    this->_name = name;
}

void Interrupt::setPriority(int priority)
{
    this->_priority = priority;
}

void Interrupt::setExpr(SixRGrammerParser::ExpressionContext *ctx)
{
    this->_expression = ctx;
}

void Interrupt::setAssignExpr(SixRGrammerParser::AssignmentExpressionContext *ctx)
{
    this->_assignmentExpression = ctx;
}

string Interrupt::getName()
{
    return _name;
}

int Interrupt::getPriority()
{
    return this->_priority;
}

SixRGrammerParser::ExpressionContext* Interrupt::getExpr()
{
    return _expression;
}

SixRGrammerParser::AssignmentExpressionContext *Interrupt::getAssignExpr()
{
    return _assignmentExpression;
}

string Interrupt::ToString()
{
    string s="";
    s+= "Interrupt is running: "+_name+" in "+nameSpace+".\r\n";
    return s;
}

