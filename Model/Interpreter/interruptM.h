#ifndef INTERRUPTM_H
#define INTERRUPTM_H
#include <vector>
#include "SixRGrammerParser.h"
using namespace::std;
class Interrupt{
public:
    Interrupt();
public:
    //bool isGlobal;
    string nameSpace;
    //Subroutine *nameSpace;
    void setName(string name);
    void setPriority(int priority);
    void setExpr(SixRGrammerParser::ExpressionContext* ctx);
    void setAssignExpr(SixRGrammerParser::AssignmentExpressionContext* ctx);
    string getName();
    int getPriority();
    SixRGrammerParser::ExpressionContext *getExpr();
    SixRGrammerParser::AssignmentExpressionContext *getAssignExpr();
    string ToString();

private:
    int _priority;
    string _name;
    SixRGrammerParser::ExpressionContext* _expression;
    SixRGrammerParser::AssignmentExpressionContext* _assignmentExpression;
};

#endif // INTERRUPTM_H
