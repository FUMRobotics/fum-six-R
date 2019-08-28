#include "msixrlistener.h"
#include "SixRGrammerParser.h"
#include "variable.h"
#include <QThread>
#include <map>

using namespace std;
using namespace antlr4;

bool icompare_pred(unsigned char a, unsigned char b)
{
    return std::tolower(a) == std::tolower(b);
}

bool stringCompare(std::string const& a, std::string const& b)
{
    if (a.length()==b.length()) {
        return std::equal(b.begin(), b.end(),
                          a.begin(), icompare_pred);
    }
    else {
        return false;
    }
}

MsixRlistener::MsixRlistener()
{
    controller = Controller::getInstance();
    global.setSubRoutineName("global");
    main.setSubRoutineName("main");
}

void MsixRlistener::enterModuleRoutines(SixRGrammerParser::ModuleRoutinesContext *ctx)
{
    for(int i=0;i<ctx->children.size();i++)
    {
        if(dynamic_cast<SixRGrammerParser::MainRoutineContext *>(ctx->children.at(i))!=nullptr)
        {
            _enterMainRoutine((SixRGrammerParser::MainRoutineContext *)(ctx->children.at(i)));
        }
        else if(dynamic_cast<SixRGrammerParser::SubRoutineContext *>(ctx->children.at(i))!=nullptr)
        {
            _enterSubroutineDeclartion((SixRGrammerParser::SubRoutineContext *)(ctx->children.at(i)));
        }
        else if(dynamic_cast<SixRGrammerParser::VariableDeclarationContext *>(ctx->children.at(i))!=nullptr)
        {
            _enterVariableDeclaration((SixRGrammerParser::VariableDeclarationContext *)(ctx->children.at(i)),&global);
        }
    }
}

void MsixRlistener::exitModuleRoutines(SixRGrammerParser::ModuleRoutinesContext *ctx)
{

}

void MsixRlistener::enterStart(SixRGrammerParser::StartContext * ctx)
{

}

// need test
void MsixRlistener::_enterVariableDeclaration(SixRGrammerParser::VariableDeclarationContext *ctx, Subroutine *nameSpace)
{
    string aaaa;
    Variable variable;

    variable.name = (ctx->variableName()->IDENTIFIER()->getText());
    variable.type = (ctx->type() != nullptr ? ctx->type()->getText() : "int");
    variable.setSize(1);
    if (!_isPrimitiveType(variable.type) && !_isSixRPrimitiveType(variable.type))
        throw "Undefined variable type: " + variable.type;
    if (ctx->variableInitialisation() != nullptr)
        aaaa= ctx->variableInitialisation()->getText();
    if( ctx->variableInitialisation()->getText()=="")
        throw "Empty Variable Init: " + variable.name;;
    variable.evaluated=true;
    if (_isPrimitiveType(variable.type) && ctx->variableInitialisation()->expression() != nullptr) {   //Primitive Type
        int idxSuffix = _getIndexFromVariableSuffix(ctx->variableName()->arrayVariableSuffix(), nameSpace);      // Only support array in primitive types
        if (idxSuffix != -1) {
            variable.setSize(idxSuffix);
        }
        variable.setData(_enterExpression(ctx->variableInitialisation()->expression(), nameSpace).data);
    }
    else if (_isSixRPrimitiveType(variable.type) && ctx->variableInitialisation()->sixRJPR() != nullptr) {    // SixRPrimitive Type    // Need to handle errors in types and names "int b2 = [b1[a1]]"-->OK
        //Need handle for initial value without cama. (X:10 Y:20) is wrong
        _setSixRJPR(ctx->variableInitialisation()->sixRJPR(), &variable, false, nameSpace);
    }
    if (!_checkVariableName(variable.name, nameSpace))
        nameSpace->addVariableToCtx(variable);
    else {
        throw "Duplicate variable name: " + variable.name;
    }
}
bool MsixRlistener::_checkVariableName(string varName, Subroutine *nameSpace){
    Variable temp;
    return nameSpace->getVariableByName(varName, temp);
    //    for(int i=0; i<variables[nameSpace].size(); i++){
    //        if(variables[nameSpace].at(i)->name==varName)
    //            return false;
    //    }
    //    return true;
}

int MsixRlistener::_getIndexFromVariableSuffix(SixRGrammerParser::ArrayVariableSuffixContext *ctx, Subroutine *nameSpace)
{
    if(ctx == nullptr)
        return -1;
    vector<int> arrayDims;
    for(int i=0; i<ctx->expression().size(); i++){
        Variable arraySize = _enterExpression(ctx->expression()[i], nameSpace);
        arrayDims.push_back(arraySize.getDataAt(0));
    }
    //Only support for 1D Array
    return arrayDims[0];
}

void MsixRlistener::_report(Subroutine *nameSpace, string msg)
{
    cout<<"Report: "<< msg<<endl;
    cout<<nameSpace->ToString()<<"***"<<endl;
}

bool MsixRlistener::_isPrimitiveType(string type)
{
    bool result = false;
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::INT]);
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::BOOL]);
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::CHAR]);
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::FLOAT]);
    return result;
}

bool MsixRlistener::_isSixRPrimitiveType(string type)
{
    bool result = false;
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::POINTJ]);
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::POINTP]);
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::POS]);
    result |= stringCompare( type, PrimitiveTypeS[PrimitiveType::ORIENT]);
    return result;
}

void MsixRlistener::_setRPPart(vector<SixRGrammerParser::SixRRPPartContext *> ctx, Variable *variable, Subroutine *nameSpace)
{
    for(int i=0; i<ctx.size(); i++){
        if(ctx[i]->sixRPPart() != nullptr){
            if(ctx[i]->sixRPPart()->XX() != nullptr){
                variable->setX(_enterExpression(ctx[i]->sixRPPart()->expression(), nameSpace).getDataAt(0));
            }else if(ctx[i]->sixRPPart()->YY() != nullptr){
                variable->setY(_enterExpression(ctx[i]->sixRPPart()->expression(), nameSpace).getDataAt(0));
            }else if(ctx[i]->sixRPPart()->ZZ() != nullptr){
                variable->setZ(_enterExpression(ctx[i]->sixRPPart()->expression(), nameSpace).getDataAt(0));
            }
        }else if(ctx[i]->sixRRPart() != nullptr){
            if(ctx[i]->sixRRPart()->AA() != nullptr){
                variable->setA(_enterExpression(ctx[i]->sixRRPart()->expression(), nameSpace).getDataAt(0));
            }else if(ctx[i]->sixRRPart()->BB() != nullptr){
                variable->setB(_enterExpression(ctx[i]->sixRRPart()->expression(), nameSpace).getDataAt(0));
            }else if(ctx[i]->sixRRPart()->CC() != nullptr){
                variable->setC(_enterExpression(ctx[i]->sixRRPart()->expression(), nameSpace).getDataAt(0));
            }
        }
    }
}

void MsixRlistener::_setPPart(vector<SixRGrammerParser::SixRPPartContext *> ctx, Variable *variable, Subroutine *nameSpace)
{
    for(int i=0; i<ctx.size(); i++){
        if(ctx[i]->XX() != nullptr){
            variable->setX(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->YY() != nullptr){
            variable->setY(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->ZZ() != nullptr){
            variable->setZ(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }
    }
}

void MsixRlistener::_setRPart(vector<SixRGrammerParser::SixRRPartContext *> ctx, Variable *variable, Subroutine *nameSpace)
{
    for(int i=0; i<ctx.size(); i++){
        if(ctx[i]->AA() != nullptr){
            variable->setA(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->BB() != nullptr){
            variable->setB(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->CC() != nullptr){
            variable->setC(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }
    }
}

void MsixRlistener::_setJPart(vector<SixRGrammerParser::SixRJPartContext *> ctx, Variable *variable, Subroutine *nameSpace)
{
    for(int i=0; i<ctx.size(); i++){
        if(ctx[i]->J1() != nullptr){
            variable->setJ1(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->J2() != nullptr){
            variable->setJ2(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->J3() != nullptr){
            variable->setJ3(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->J4() != nullptr){
            variable->setJ4(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->J5() != nullptr){
            variable->setJ5(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }else if(ctx[i]->J6() != nullptr){
            variable->setJ6(_enterExpression(ctx[i]->expression(), nameSpace).getDataAt(0));
        }
    }
}


void MsixRlistener::_enterMainRoutine(SixRGrammerParser::MainRoutineContext *ctx)
{
    _enterStatementList(ctx->routineBody()->statementList(), &main);
    _report(&main, "exit main");
}

void MsixRlistener::_enterSubroutineDeclartion(SixRGrammerParser::SubRoutineContext *ctx )
{
    Subroutine *srd = new Subroutine();
    string subroutineName = ctx->procedureName()->IDENTIFIER()->getText();
    if(!_checkSubroutineName(subroutineName)){
        srd->setSubRoutineReturnType(ctx->type()!=nullptr?ctx->type()->getText():"int");
        srd->setSubRoutineName(subroutineName);
        srd->setSubRoutineStatements(ctx->routineBody()->statementList());
        _addFormalParametersToSubroutine(srd, ctx->formalParameters());
        subroutines.push_back(srd);
    }else{
        throw "Duplicate subroutine name: "+subroutineName;
    }

}
bool MsixRlistener::_checkSubroutineName(string name){
    for(int i=0; i<subroutines.size(); i++){
        if(name == subroutines[i]->getSubRoutineName())
            return true;
    }
    return false;
}

void MsixRlistener::_getVariableByName(string name, Variable *var, Subroutine *nameSpace)
{
    //var = new Variable();
    //Variable temp = *var;
    //var.name = name;
    if(nameSpace->getVariableByName(name, *var))
        return;
    else if(global.getVariableByName(name, *var))
        return;
    // Literal case
    try {
        Variable value;
        var->type=("double");
        var->name=("online variable");
        var->setDataAt(0, stod(name));
        return ;
    } catch (const std::exception& e) {
    }
    throw "Error 404: variable is not defined: "+name;
}

void MsixRlistener::_addFormalParametersToSubroutine(Subroutine *sub, SixRGrammerParser::FormalParametersContext *ctx)
{
    for(int i=0; i<ctx->parameter().size(); i++){
        Variable argument;
        argument.type = ctx->parameter()[i]->type()->getText();
        argument.name = ctx->parameter()[i]->variableName()->IDENTIFIER()->getText();
        int size = _getIndexFromVariableSuffix(ctx->parameter()[i]->variableName()->arrayVariableSuffix(), sub);
        if(size != -1)
            argument.setSize(size);
        else
            argument.setSize(1);
        sub->addVariableToCtx(argument);
    }
}


void MsixRlistener::_enterStateReturn(SixRGrammerParser::STATRETURNContext *ctx, Subroutine *nameSpace)
{
    Variable returnValue;
    if(ctx->expression() != nullptr)
        returnValue=_enterExpression(ctx->expression(), nameSpace);
    nameSpace->getReturnVal()->name=returnValue.name;
    nameSpace->getReturnVal()->type = returnValue.type;
    nameSpace->getReturnVal()->setData(returnValue.data);
}

int MsixRlistener::_enterStatementList(SixRGrammerParser::StatementListContext *ctx, Subroutine *nameSpace, string currentScope)
{
    int returnVal=10;
    for(int i=0;i<ctx->children.size() && !nameSpace->isReturnValReady();i++)
    {
        SixRGrammerParser::StatementContext* stat=dynamic_cast<SixRGrammerParser::StatementContext  *>(ctx->children.at(i));

        if(dynamic_cast<SixRGrammerParser::STATCONTINUEContext  *>(stat)!=nullptr)   //CONTINIUE handle if needed
        {
            returnVal = 0; //Start loop from over
            break;
        }
        else if(dynamic_cast<SixRGrammerParser::STATFORContext  *>(stat)!=nullptr)   //OK
        {
            _enterStateFor(((SixRGrammerParser::STATFORContext *) (ctx->children.at(i))), nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATIFContext  *>(stat)!=nullptr)   // OK
        {
            _enterStateIf((SixRGrammerParser::STATIFContext *) (stat), nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATWAITFORContext  *>(stat)!=nullptr)//WAIT FOR
        {

        }
        else if(dynamic_cast<SixRGrammerParser::STATWAITSECContext  *>(stat)!=nullptr)
        {
            _enterStateWaitSecond((SixRGrammerParser::STATWAITSECContext *) (stat), nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATWHILEContext  *>(stat)!=nullptr)
        {
            _enterStateWhile((SixRGrammerParser::STATWHILEContext *) (stat), nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATRETURNContext *>(stat))//RETURN handle if needed
        {
            _enterStateReturn((SixRGrammerParser::STATRETURNContext *)stat, nameSpace);
            returnVal = -1;
            break;
        }
        else if(dynamic_cast<SixRGrammerParser::STATASINEPRContext *>(stat)!=nullptr)//Need Implementation
        {
            _enterStateAssignExpression((SixRGrammerParser::STATASINEPRContext *)(stat), nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATEXPContext *>(stat)!=nullptr) // OK , subroutine
        {
            _enterStateExpression((SixRGrammerParser::STATEXPContext *)(stat), nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATBRAKEContext *>(stat))//Break handle if needed
        {
            returnVal = -1; //Dont run loop anymore
            break;
        }
        else if(dynamic_cast<SixRGrammerParser::STATNEWLINEContext *>(stat))//NewLine
        {

        }
        else if(dynamic_cast<SixRGrammerParser::STATPTPContext *>(stat)!=nullptr)
        {
            _enterStatePTP((SixRGrammerParser::STATPTPContext *) (stat),  nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATLINContext *>(stat)!=nullptr)
        {
            _enterStateLinear((SixRGrammerParser::STATLINContext *) (stat),  nameSpace);
        }
        else if (dynamic_cast<SixRGrammerParser::STATCIRContext *>(stat)!=nullptr)
        {
            _enterStateCirc((SixRGrammerParser::STATCIRContext *) (stat),  nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATVARDECContext *>(stat)!=nullptr)
        {
            _enterVariableDeclaration(((SixRGrammerParser::STATVARDECContext *)(stat))->variableDeclaration(), nameSpace);
        }
        else if(dynamic_cast<SixRGrammerParser::STATSCFContext *>(stat)!=nullptr)   //Implementaion
        {
            _enterStateSetFrame((SixRGrammerParser::STATSCFContext *)(stat), nameSpace);
        }
    }

    _report(nameSpace, "Finish statementList ("+currentScope+")");
    return returnVal;
}
void MsixRlistener::_enterStateWhile(SixRGrammerParser::STATWHILEContext *ctx, Subroutine *nameSpace)
{
    while(_enterExpression(ctx->expression(), nameSpace).getDataAt(0)){
        int res = _enterStatementList(ctx->statementList(), nameSpace, "While "+ctx->expression()->getText()+" :"+to_string(_enterExpression(ctx->expression(), nameSpace).getDataAt(0)));
        if(res == -1)   // return from this namespace
            return;
    }
}

void MsixRlistener::_enterStateAssignExpression(SixRGrammerParser::STATASINEPRContext *ctx, Subroutine *nameSpace)
{
    Variable dest;
    _getVariableByName(ctx->assignmentExpression()->variableName()->IDENTIFIER()->getText(), &dest,nameSpace);
    if(ctx->assignmentExpression()->expression()!=nullptr){
        int idxSuffix = _getIndexFromVariableSuffix(ctx->assignmentExpression()->variableName()->arrayVariableSuffix(), nameSpace);
        if(idxSuffix == -1)
            dest.setData(_enterExpression(ctx->assignmentExpression()->expression(), nameSpace).data);
        else
            dest.setDataAt(_enterExpression(ctx->assignmentExpression()->expression(), nameSpace).getDataAt(0), idxSuffix);
    }else if(ctx->assignmentExpression()->sixRJPR()!=nullptr){
        _setSixRJPR(ctx->assignmentExpression()->sixRJPR(), &dest, false, nameSpace);
    }
    nameSpace->setVariableByName(dest);
}

void MsixRlistener::_enterStateExpression(SixRGrammerParser::STATEXPContext *ctx, Subroutine *nameSpace)
{
    _enterExpression(ctx->expression(), nameSpace);
}

void MsixRlistener::_setSixRJPR(SixRGrammerParser::SixRJPRContext *ctx, Variable *variable, bool changeType, Subroutine *nameSpace)
{
    variable->setSize(6);
    SixRGrammerParser::SixRJPRContext *jpr = ctx;
    bool canInit = false;
    //JXPoint
    if (jpr->sixRJXPoint() != nullptr && (changeType || stringCompare(variable->type, PrimitiveTypeS[PrimitiveType::POINTP]) || stringCompare(variable->type, PrimitiveTypeS[PrimitiveType::POINTJ]))) {
        canInit= _setSixRJXPoint(jpr->sixRJXPoint(), variable, changeType, nameSpace);
    }
    //RPosition --> OK
    else if (jpr->sixRPosition() != nullptr && (changeType || stringCompare(variable->type, PrimitiveTypeS[PrimitiveType::POS]))) {
        canInit = true;
        _setPPart(jpr->sixRPosition()->sixRPPart(), variable, nameSpace);
        if(changeType)
            variable->type = PrimitiveTypeS[PrimitiveType::POS];
    }
    //ROrientation--> OK
    else if (jpr->sixROrientation() != nullptr && (changeType || stringCompare(variable->type, PrimitiveTypeS[PrimitiveType::ORIENT]))) {
        canInit = true;
        _setRPart(jpr->sixROrientation()->sixRRPart(), variable, nameSpace);
        if(changeType)
            variable->type = PrimitiveTypeS[PrimitiveType::ORIENT];
    }
    if (canInit == false){
        string initType="";
        if(jpr->sixRJXPoint()->sixRPPoint() != nullptr)
            initType = "JXPoint()->PPoint()";
        else if(jpr->sixRJXPoint()->sixRJPoint() != nullptr)
            initType = "JXPoint()->JPoint()";
        else if(jpr->sixRPosition()!= nullptr)
            initType = "Position";
        else if(jpr->sixROrientation()!= nullptr)
            initType = "sixROrientation";
        throw "Can not init variable, initilize and type does not match!! Initiale value should be from " + variable->type + " type but is "+ initType+".";
    }
}

bool MsixRlistener::_setSixRJXPoint(SixRGrammerParser::SixRJXPointContext *ctx, Variable *variable, bool changeType, Subroutine *nameSpace)
{
    bool canInit=false;
    variable->setSize(6);
    //PPoint
    if (ctx->sixRPPoint() != nullptr && (changeType || stringCompare(variable->type, PrimitiveTypeS[PrimitiveType::POINTP]))) {
        //PPart --> OK
        canInit = true;
        if (ctx->sixRPPoint()->sixRRPPart().size() >0) {
            _setRPPart(ctx->sixRPPoint()->sixRRPPart(), variable, nameSpace);
        }
        //VariableName --> OK
        else if (ctx->sixRPPoint()->variableName().size()>0) {
            for (int i = 0; i<ctx->sixRPPoint()->variableName().size(); i++) {
                Variable temp;
                _getVariableByName(ctx->sixRPPoint()->variableName()[i]->getText(), &temp, nameSpace);
                variable->setDataAt(i, temp.getDataAt(0));
            }
        }
        if(changeType)
            variable->type = PrimitiveTypeS[PrimitiveType::POINTP];
    }
    //JPoint --> OK
    else if (ctx->sixRJPoint() != nullptr && (changeType || stringCompare(variable->type, PrimitiveTypeS[PrimitiveType::POINTJ]))) {
        canInit = true;
        _setJPart(ctx->sixRJPoint()->sixRJPart(), variable, nameSpace);
        if(changeType)
            variable->type = PrimitiveTypeS[PrimitiveType::POINTJ];
    }
    return canInit;
}

Variable MsixRlistener::_enterExpression(SixRGrammerParser::ExpressionContext *ctx, Subroutine *nameSpace)
{
    if(ctx->procedureName()!=nullptr){
        string procedureName = ctx->procedureName()->getText();
        Subroutine *srd;
        bool find=false;
        for(int i=0; i<subroutines.size(); i++){
            if(subroutines[i]->getSubRoutineName() == procedureName){
                find = true;
                srd = subroutines[i];
                break;
            }
        }
        if(find == false){
            throw "Can not find procedure named: "+procedureName;
            return Variable();
        }else
        {
            Subroutine subroutine1 = *srd;
            Subroutine subroutineCopy = subroutine1;
            //Fill procedure argument
            //vector<Variable *> formalParameters = subroutineCopy.getSubRoutineFormalParameters();
            for(int i=0; i<subroutineCopy.getSubRoutineVariables().size(); i++){
                Variable temp = _enterExpression(ctx->expression().at(i), nameSpace) ;
                for(int j=0; j<subroutineCopy.getSubRoutineVariables()[i]->getData().size(); j++)
                    subroutineCopy.getSubRoutineVariables()[i]->setDataAt(temp.getDataAt(j),j);
                //formalParameters[i]=&temp;
            }
            _enterStatementList(subroutineCopy.getSubRoutineStatements(), &subroutineCopy);
            return *subroutineCopy.getReturnVal();
        }
    }
    else if(ctx->conditionalOrExpression()[0] != nullptr){
        if(ctx->conditionalOrExpression().size()==1){
            Variable returnExp=_enterConditionalOrExpression(ctx->conditionalOrExpression()[0], nameSpace);
            return returnExp;
        }
        Variable boolResult;
        boolResult.type = "bool";
        //boolResult.setDataAt(0,0); no need!
        for(int i=0; i<ctx->conditionalOrExpression().size()-1; i++){
            string op = ctx->relationalOp().at(i)->getText();
            Variable firstOpearnd = _enterConditionalOrExpression(ctx->conditionalOrExpression().at(i), nameSpace);
            Variable nextOpearnd =_enterConditionalOrExpression(ctx->conditionalOrExpression().at(i+1), nameSpace);
            for(int j=0; j<firstOpearnd.data.size(); j++)
                if(op == "=="){
                    boolResult.setDataAt(firstOpearnd.getDataAt(j) == nextOpearnd.getDataAt(j),j);
                }else  if(op == "!="){
                    boolResult.setDataAt(firstOpearnd.getDataAt(j) != nextOpearnd.getDataAt(j),j);
                }else  if(op == "<="){
                    boolResult.setDataAt(firstOpearnd.getDataAt(j) <= nextOpearnd.getDataAt(j),j);
                }else  if(op == ">="){
                    boolResult.setDataAt(firstOpearnd.getDataAt(j) >= nextOpearnd.getDataAt(j),j);
                }else  if(op == "<"){
                    boolResult.setDataAt(firstOpearnd.getDataAt(j) < nextOpearnd.getDataAt(j),j);
                }else  if(op == ">"){
                    boolResult.setDataAt(firstOpearnd.getDataAt(j) > nextOpearnd.getDataAt(j),j);
                }
        }
        return boolResult;
    }
    string s1s = ctx->getText();
    throw "Exception in expression: "+ctx->getText();
}

Variable MsixRlistener::_enterConditionalOrExpression(SixRGrammerParser::ConditionalOrExpressionContext *ctx, Subroutine *nameSpace)
{
    Variable result=_enterExclusiveOrExpression(ctx->exclusiveOrExpression()[0], nameSpace);
    if(ctx->exclusiveOrExpression().size()>1 && result.type!="bool")
        result.type = "int";
    for(int i=1; i<ctx->exclusiveOrExpression().size(); i++)
    {
        Variable nextOpearnd = _enterExclusiveOrExpression(ctx->exclusiveOrExpression()[i], nameSpace);
        for(int j=0; j<result.data.size(); j++)
            result.setDataAt((int)result.getDataAt(j) | (int)_enterExclusiveOrExpression(ctx->exclusiveOrExpression()[i], nameSpace).getDataAt(j), j);
    }
    return result;
}

Variable MsixRlistener::_enterExclusiveOrExpression(SixRGrammerParser::ExclusiveOrExpressionContext *ctx, Subroutine *nameSpace)
{
    Variable result=_enterConditionalAndExpression(ctx->conditionalAndExpression()[0], nameSpace);
    if(ctx->conditionalAndExpression().size()>1 && result.type!="bool")
        result.type = "int";
    for(int i=1; i<ctx->conditionalAndExpression().size(); i++)
    {
        Variable nextOpearnd = _enterConditionalAndExpression(ctx->conditionalAndExpression()[i], nameSpace);
        for(int j=0; j<result.data.size(); j++)
            result.setDataAt((int)result.getDataAt(j) ^ (int)nextOpearnd.getDataAt(j),j);
    }
    return result;
}

Variable MsixRlistener::_enterConditionalAndExpression(SixRGrammerParser::ConditionalAndExpressionContext *ctx, Subroutine *nameSpace)
{
    Variable result=_enterAdditiveExpression(ctx->additiveExpression()[0], nameSpace);
    if(ctx->additiveExpression().size()>1 && result.type!="bool")
        result.type = "int";
    for(int i=1; i<ctx->additiveExpression().size(); i++){
        Variable nextOpearnd = _enterAdditiveExpression(ctx->additiveExpression()[i], nameSpace);
        for(int j=0; j<result.data.size(); j++)
            result.setDataAt((int)result.getDataAt(j) & (int)nextOpearnd.getDataAt(j), j);
    }
    return result;
}

Variable MsixRlistener::_enterAdditiveExpression(SixRGrammerParser::AdditiveExpressionContext *ctx, Subroutine *nameSpace)
{
    Variable result=_enterMultiplicativeExpression(ctx->multiplicativeExpression()[0], nameSpace);

    for(int i=1; i<ctx->multiplicativeExpression().size(); i++){
        string op = ctx->children.at(2*i-1)->getText();
        Variable nextOpearnd = _enterMultiplicativeExpression(ctx->multiplicativeExpression()[i], nameSpace);
        for(int j=0; j<result.data.size(); j++)
            if(op == "+")
                result.setDataAt(result.getDataAt(j) + nextOpearnd.getDataAt(j),j);
            else if(op == "-")
                result.setDataAt(result.getDataAt(j) - nextOpearnd.getDataAt(j),j);
    }
    return result;
}

Variable MsixRlistener::_enterMultiplicativeExpression(SixRGrammerParser::MultiplicativeExpressionContext *ctx, Subroutine *nameSpace)
{
    Variable result=_enterUnaryNotExpression(ctx->unaryNotExpression()[0], nameSpace);
    for(int i=1; i<ctx->unaryNotExpression().size(); i++){
        string op = ctx->children.at(2*i-1)->getText();
        Variable nextOpearnd = _enterUnaryNotExpression(ctx->unaryNotExpression()[i], nameSpace);
        for(int j=0; j<result.data.size(); j++)
            if(op == "*")
                result.setDataAt(result.getDataAt(j) * nextOpearnd.getDataAt(j), j);
            else if(op == "/")
                result.setDataAt(result.getDataAt(j) / nextOpearnd.getDataAt(j), j);
    }
    return result;
}

Variable MsixRlistener::_enterUnaryNotExpression(SixRGrammerParser::UnaryNotExpressionContext *ctx, Subroutine *nameSpace)
{
    if(ctx->unaryNotExpression()!= nullptr){
        Variable temp = _enterUnaryNotExpression(ctx->unaryNotExpression(), nameSpace);
        if(temp.type != "bool")
            temp.type = "int";
        for(int i=0; i<temp.data.size(); i++)
            temp.setDataAt(~(int)temp.getDataAt(i), i);
        return  temp;
    }else if(ctx->unaryPlusMinuxExpression() != nullptr){
        return _enterUnaryPlusMinuxExpression(ctx->unaryPlusMinuxExpression(), nameSpace);
    }
}

Variable MsixRlistener::_enterUnaryPlusMinuxExpression(SixRGrammerParser::UnaryPlusMinuxExpressionContext *ctx, Subroutine *nameSpace)
{
    if(ctx->unaryPlusMinuxExpression()!= nullptr){
        Variable temp = _enterUnaryPlusMinuxExpression(ctx->unaryPlusMinuxExpression(), nameSpace);
        if(ctx->children.at(0)->getText()=="-"){
            for(int i=0; i<temp.data.size(); i++)
                temp.setDataAt(-temp.getDataAt(i), i);
        }
        return temp;
    }else if(ctx->primary() != nullptr){
        return _enterPrimary(ctx->primary(), nameSpace);
    }
}

Variable MsixRlistener::_enterPrimary(SixRGrammerParser::PrimaryContext *ctx, Subroutine *nameSpace)
{
    if(ctx->parExpression()!= nullptr){
        return  _enterParExpression(ctx->parExpression(), nameSpace);
    }else if(ctx->variableName()!=nullptr){
        Variable result;
        _getVariableByName(ctx->variableName()->IDENTIFIER()->getText(),&result, nameSpace);
        Variable resultCopy = result;
        //result->CopyTo(&resultCopy);
        if(ctx->variableName()->arrayVariableSuffix() != nullptr){
            Variable idx = _enterExpression(ctx->variableName()->arrayVariableSuffix()->expression()[0], nameSpace);    // Only support 1D array
            resultCopy.removeOtherIndex(idx.getDataAt(0));  // Only get first suffix;
        }
        return resultCopy;
    }else if(ctx->literal()!=nullptr){
        return _enterLiteral(ctx->literal(), nameSpace);
    }
}

Variable MsixRlistener::_enterParExpression(SixRGrammerParser::ParExpressionContext *ctx, Subroutine *nameSpace)
{
    return  _enterExpression(ctx->expression(), nameSpace);
}

Variable MsixRlistener::_enterLiteral(SixRGrammerParser::LiteralContext *ctx, Subroutine *nameSpace)
{
    Variable literal;

    if(ctx->numberLITERAL()!=nullptr){
        literal.type="double";
        literal.setDataAt(stod(ctx->numberLITERAL()->getText()), 0);
    }else if(ctx->booleanLiteral() != nullptr){
        literal.type="bool";
        literal.setDataAt(stringCompare(ctx->booleanLiteral()->getText(), "TRUE" )? 1:0, 0);
    }else if(ctx->charLITERAL() != nullptr){  //NOT supported
        literal.type = "char";
        literal.setDataAt((int)ctx->charLITERAL()->FragCHARLITERAL()->getText().at(1),0);
    } else if(ctx->stringLITERAL() != nullptr)  {  //Not supported
        literal.type = "string";
        for(int i=1; i<ctx->stringLITERAL()->getText().size()-1; i++ )
            literal.setDataAt((int)ctx->stringLITERAL()->getText().at(i), i-1);
    }
    return literal;
}

void MsixRlistener::_checkRobotStat()
{
    if(controller->beckhoff->stop)
        return;
    controller->beckhoff->doNextLine = false;
    //    while(1);
    //            Thre
    //        }
}

void MsixRlistener::_sendCommandToRobot(int command, map<string, Variable>parameters)
{
    controller->beckhoff->CurrentLineSetValue();

    if(controller->beckhoff->IsEnableMovement)
    {

        vector<double> _positions = parameters["p1"].getData();


            QList<double> tmpValue = controller->robot->currentObjectFrame->mainPoints();


            double SelectedFrame[6] = {tmpValue.at(0),tmpValue.at(1),tmpValue.at(2),
                                       tmpValue.at(3),tmpValue.at(4),tmpValue.at(5)};
            double TargetPoint[6] = {_positions.at(0),
                                     _positions.at(1),
                                     _positions.at(2),
                                     _positions.at(3),
                                     _positions.at(4),
                                     _positions.at(5)};
            double OutPointInRef[6];
            controller->robot->PointInReference(TargetPoint,SelectedFrame,"object",OutPointInRef);
            for (int i=0; i< controller->beckhoff->NumberOfRobotMotors; ++i) {
                controller->beckhoff->setTargetPosition(OutPointInRef[i],i);
            }



        controller->beckhoff->setTargetPosition(parameters["FF"].getDataAt(0),6);  // FF
        controller->beckhoff->setTargetPosition(parameters["CON"].getDataAt(0),7);  // CON
        controller->beckhoff->setGUIManager(10);
        //controller->beckhoff->setGUIManager(command);
        //qDebug() << "before 300" << endl;
        QThread::msleep(300);
        int next;// = controller->beckhoff->getNextCommandSign();
        do{
            QThread::msleep(200);
            next = controller->beckhoff->getNextCommandSign();

        }while(next==1);

    }
}


void MsixRlistener::_enterStateFor(SixRGrammerParser::STATFORContext *ctx, Subroutine *nameSpace) // OK need test
{
    Variable itr;// = new Variable();
    _getVariableByName(ctx->IDENTIFIER()->getText(), &itr,nameSpace);
    if(ctx->expression().size()==2){
        Variable v1 = _enterExpression(ctx->expression().at(0), nameSpace);
        Variable v2 = _enterExpression(ctx->expression().at(1), nameSpace);
        int startForLoop=(int)v1.getDataAt(0);
        int endForLoop=(int)v2.getDataAt(0);
        double iterator = itr.getDataAt(0);
        for(iterator=startForLoop;iterator<endForLoop;iterator++)
        {
            itr.setDataAt(iterator, 0);
            nameSpace->setVariableByName(itr);
            int res =_enterStatementList(ctx->statementList(), nameSpace, "For "+itr.name + " from "+to_string( startForLoop)+" to "+to_string(endForLoop));
            if(res == -1) // return from namespace
                return;
        }
    }else{
        throw "Error in selected range.\r\nSyntax:\r\nFOR IDENTIFIER = expression TO expression";
    }
}
void MsixRlistener::_enterStateIf(SixRGrammerParser::STATIFContext *ctx, Subroutine *nameSpace)
{
    Variable ifCondition = _enterExpression(ctx->expression(), nameSpace);
    if(ifCondition.getDataAt(0)){
        _enterStatementList(ctx->statementList()[0], nameSpace,"If "+ifCondition.name+" = true");
    }else{
        if(ctx->statementList().size()==2)
            _enterStatementList(ctx->statementList()[1], nameSpace,"If "+ifCondition.name+" = false");
    }
}

void MsixRlistener::_enterStateWaitSecond(SixRGrammerParser::STATWAITSECContext *ctx, Subroutine *nameSpace)
{
    //waitSec=stod(ctx->children.at(2)->getText());
    //controller
    //*********** call waitSec
    //controller
}

void MsixRlistener::_enterStateWaitFor(SixRGrammerParser::STATWAITFORContext *ctx, Subroutine *nameSpace)
{
    while(_enterExpression(ctx->expression(), nameSpace).getDataAt(0)==0);
}
void MsixRlistener::_enterStatePTP(SixRGrammerParser::STATPTPContext *ctx, Subroutine *nameSpace)
{
    _checkRobotStat();

    map<string, Variable>params;
    Variable dest;
    if(ctx->targetPoint()!=nullptr){
        if(ctx->targetPoint()->variableName()!=nullptr)
            _getVariableByName(ctx->targetPoint()->variableName()->IDENTIFIER()->getText(),&dest,nameSpace);
        else if(ctx->targetPoint()->sixRJXPoint()!=nullptr){
            _setSixRJXPoint(ctx->targetPoint()->sixRJXPoint(), &dest, true, nameSpace);
        }
    }else
        throw "LangErr PTP: Can not find destination";

    params["p1"] = dest;

    if(ctx->ffExpr()!=nullptr){
        params["FF"] = _enterExpression(ctx->ffExpr()->expression(), nameSpace);
        params["FF"].name = "FF";
    }
    if(ctx->conExpr()!=nullptr){
        params["CON"] = _enterExpression(ctx->conExpr()->expression(), nameSpace);
        params["CON"].name = "CON";
    }
    if(ctx->expression()!=nullptr){
        Variable approx;
        approx.setDataAt(_enterExpression(ctx->expression(), nameSpace).getDataAt(0),0);
        params["Approx"] = approx;//_enterExpression(ctx->expression(), nameSpace);
        params["Approx"].name = "Approx";
    }

    _sendCommandToRobot(ControlManager::PTP, params);
}

void MsixRlistener::_enterStateLinear(SixRGrammerParser::STATLINContext *ctx, Subroutine *nameSpace)
{
    _checkRobotStat();

    map<string, Variable>params;
    Variable dest;

    if(ctx->targetPoint()!=nullptr){
        if(ctx->targetPoint()->variableName()!=nullptr)
            _getVariableByName(ctx->targetPoint()->variableName()->IDENTIFIER()->getText(),&dest,nameSpace);
        else if(ctx->targetPoint()->sixRJXPoint()!=nullptr){
            _setSixRJXPoint(ctx->targetPoint()->sixRJXPoint(), &dest, true, nameSpace);
        }
    }else
        throw "LangErr LIN: Can not find destination";

    params["p1"] = dest;

    if(ctx->ffExpr()!=nullptr){
        params["FF"] = _enterExpression(ctx->ffExpr()->expression(), nameSpace);
        params["FF"].name = "FF";
    }
    if(ctx->conExpr()!=nullptr){
        params["CON"] = _enterExpression(ctx->conExpr()->expression(), nameSpace);
        params["CON"].name = "CON";
    }
    if(ctx->expression()!=nullptr){
        Variable approx;
        approx.setDataAt(_enterExpression(ctx->expression(), nameSpace).getDataAt(0),0);
        params["Approx"] = approx;//_enterExpression(ctx->expression(), nameSpace);
        params["Approx"].name = "Approx";
    }

    _sendCommandToRobot(ControlManager::LIN, params);
}

void MsixRlistener::_enterStateCirc(SixRGrammerParser::STATCIRContext *ctx, Subroutine *nameSpace)
{
    map<string, Variable>params;
    Variable point[3];
    if(ctx->targetPoint().size()==3){
        for(int i=0; i<ctx->targetPoint().size(); i++){
            if(ctx->targetPoint()[i]->variableName()!=nullptr)
                _getVariableByName(ctx->targetPoint()[i]->variableName()->IDENTIFIER()->getText(), &point[i],nameSpace);
            else if(ctx->targetPoint()[i]->sixRJXPoint()!=nullptr){
                _setSixRJXPoint(ctx->targetPoint()[i]->sixRJXPoint(), &point[i], true, nameSpace);
            }
        }
    }else
        throw "LangErr CIRC: Can not find destination";

    params["p1"] = point[0];
    params["p2"] = point[1];
    params["p3"] = point[2];

    if(ctx->radiusExpr()!=nullptr){
        params["Radius"] = _enterExpression(ctx->radiusExpr()->expression(), nameSpace);
        params["Radius"].name = "Radius";
    }
    if(ctx->ffExpr()!=nullptr){
        params["FF"] = _enterExpression(ctx->ffExpr()->expression(), nameSpace);
        params["FF"].name = "FF";
    }
    if(ctx->conExpr()!=nullptr){
        params["CON"] = _enterExpression(ctx->conExpr()->expression(), nameSpace);
        params["CON"].name = "CON";
    }
    if(ctx->expression()!=nullptr){
        Variable approx;
        approx.setDataAt(_enterExpression(ctx->expression(), nameSpace).getDataAt(0),0);
        params["Approx"] = approx;//_enterExpression(ctx->expression(), nameSpace);
        params["Approx"].name = "Approx";
    }

    _sendCommandToRobot(ControlManager::CIR, params);
}

void MsixRlistener::_enterStateSetFrame(SixRGrammerParser::STATSCFContext *ctx, Subroutine *nameSpace)
{
    map<string, Variable>params;
    Variable type;
    type.type = ctx->FrameType()->getText();
    type.name = ctx->variableName()->getText();
    params["type"] = type;
    _sendCommandToRobot(ControlManager::SetFrame, params);
}
