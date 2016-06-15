// Authors: Eual Smith, Brendan Murphey, !Jacob Causer
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <stack>
#include <sstream>
#include <time.h>
#include <algorithm>
#include <stdlib.h>
using namespace std;

enum storeType { INTEGER, BOOLEAN, PROG_NAME, UNKNOWN };
enum allocation { YES, NO };
enum modes { VARIABLE, CONSTANT };

struct entry{
    string internalName;
    string externalName;
    storeType dataType;
    modes mode;
    string value;
    allocation alloc;
    int units;
};

//Stage 0
void CreateListingHeader();
void Parser();
void CreateListingTrailer();
void PrintSymbolTable();
void Prog();
void ProgStmt();
void Consts();
void Vars();
void BeginEndStmt();
void ConstStmts();
void VarStmts();
string Ids();
void Insert(string,storeType, modes, string, allocation, int);
storeType WhichType(string);
string WhichValue(string);
string NextToken();
char NextChar();
void Error(string);
string genInternalName(storeType);
bool NonKeyID();
bool isSpecial(char);
bool isLetter(char);
bool isInteger();
bool isBoolean();
      
//stage 1
void EXEC_STMTS();
void EXEC_STMT();
void ASSIGN_STMT();
void READ_STMT();
void READ_LIST();
void WRITE_STMT();
void WRITE_LIST();
void EXPRESS();
void EXPRESSES();
void TERM();
void TERMS();
void FACTOR();
void FACTORS();
void PART();
bool REL_OP();
bool ADD_LEVEL_OP();
bool MULT_LEVEL_OP();
void Code(string operatr, string operand1 = "", string operand2 = "");
void pushOperator(string);
void pushOperand(string);
string popOperator();
string popOperand();
string GetTemp();
void FreeTemp();
string GetMyJump();
void EmitProgramCode();
void EmitEndCode();
void EmitReadCode(string);
void EmitWriteCode(string);
void EmitAssignCode(string, string);
void EmitAdditionCode(string, string);
void EmitSubtractionCode(string, string);
void EmitNegCode(string);
void EmitMultiplicationCode(string, string);
void EmitDivisionCode(string, string);
void EmitModuloCode(string, string);
void EmitAndCode(string, string);
void EmitOrCode(string, string);
void EmitNotCode(string);
void EmitEqualityCode(string, string);
void EmitNotEqualCode(string, string);
void EmitGreaterThanCode(string, string);
void EmitGreaterThanEqualCode(string, string);
void EmitLessThanCode(string, string);
void EmitLessThanEqualCode(string, string);
bool isEXPRESS();
  
//stage 2
void IF_STMT();
void ELSE_PT();
void WHILE_STMT();
void REPEAT_STMT();
void NULL_STMT();
void EmitThenCode(string);
void EmitElseCode(string);
void EmitPostIfCode(string);
void EmitWhileCode();
void EmitDoCode(string);
void EmitPostWhileCode(string, string);
void EmitRepeatCode();
void EmitUntilCode(string, string);
bool isEXEC();

//**********************************GLOBAL VARIABLES***********************************
const int MAX_SYMBOL_TABLE_SIZE = 256;
int keyCount = 19;
string keys[19] = {"program", "begin", "end", "var", "const",
					"integer", "boolean", "true", "false", "not", "read", "write",
					"if", "then", "else", "repeat", "while", "do", "until"};
						
vector<entry> symbolTable;
ifstream sourceFile;
ofstream listingFile, objectFile;
string token;
char charac;
bool errorFound = false;
int lstLineNum = 0;
char prevCharac = ' ';
int boolCount = 0;
int intCount = 0;
int overflow = 0;
bool progFound = false;
const char END_OF_FILE = '$';	// arbitrary choice
string regA = "";
int myLabelCounter = -1;
int currentTempNo = -1;
int maxTempNo = -1;
int beginCounter = 0;


stack <string> Operator;
stack <string> Operand;
       
       
//*****************************************STAGE 0 BEGIN********************************************

int main(int argc, char **argv){
	sourceFile.open(argv[1]); //accepts input from argv[1]
	listingFile.open(argv[2]); //generates a listing to argv[2]
	objectFile.open(argv[3]); //generates an object code to argv[3]
	
	CreateListingHeader(); //top
	Parser(); //main
	CreateListingTrailer(); //end
	//PrintSymbolTable(); //object file
	return 0; //complete
}

// Creates the header forthe listing file
void CreateListingHeader(){
	time_t now = time(NULL); //time (as given)
	listingFile << left << "STAGE0:  " << "EJ Smith, Brendan Murphey, Jacob Causer\t" << ctime(&now) << "\n"; //names
	listingFile << setw(22) << "LINE NO." << "SOURCE STATEMENT\n\n";	//line numbers and source statements should be aligned under the headings
}

// Begins the program
void Parser(){
	NextChar();// charac must be initialized to the first character of the source file
	if(NextToken().compare("program")) //ifthe next token
		Error("keyword \"program\" expected");	// process error: keyword "program" expected;
	Prog();// parser implements the grammar rules, calling first rule
}

// Creates the trailer forthe listing file
void CreateListingTrailer(){
	listingFile << "\nCOMPILATION TERMINATED\t" << setw(5) << ((errorFound)? '1' : '0') << " ERRORS" << " ENCOUNTERED\n";
}

// Prints the symbol table into the object file
void PrintSymbolTable(){
	vector<entry>::iterator it;
	time_t now = time(NULL);
	
	objectFile << left << "STAGE1:  " << "EJ Smith, Brendan Murphey, Jacob Causer\t" << ctime(&now) << "\n";
	objectFile << "Symbol Table\n\n";
	
	for(it = symbolTable.begin(); it < symbolTable.end(); ++it)
	{
		objectFile << left << setw(15) << it->externalName << "  ";
		objectFile << left << setw(4) << it->internalName << "  ";
		objectFile << right << setw(9) << ((it->dataType == 2)?"PROG_NAME":(it->dataType == 1)?"BOOLEAN":(it->dataType == 0)?"INTEGER":"") << "  ";
		objectFile << right << setw(8) << ((it->mode == 1)?"CONSTANT":(it->mode == 0)?"VARIABLE":"") << "  ";
		objectFile << right << setw(15) << ((it->value == "true")?"1":((it->value=="false")? "0" : it->value)) << "  ";
		objectFile << right << setw(3) << right << ((it->alloc)?"NO":"YES");
		objectFile << setw(3) << it->units << "\n";
	}
}

// Production 1
void Prog(){ //token should be "program"
	if(token != "program"){
		Error("keyword 'program' expected");
	}
	ProgStmt();
	
	if(token == "const"){ 
		Consts();
	}
	if(token == "var"){
		Vars();
	}
	if(token != "begin"){
		Error("keyword \"const\", \"var\", or \"begin\" expected"); // 11-18-15 EJ changed
	}
	BeginEndStmt();
	if(token[0] != END_OF_FILE){
		Error("no text may follow end");
	}
}

// Production 2
void ProgStmt(){ //token should be "program"
	string x;
	if(token != "program")
		Error("keyword 'program' expected");
	x = NextToken();
	// note: NonKeyID only checks TOKENS
	if(!NonKeyID())
		Error("program name expected");
	if(NextToken() != ";")
		Error("semicolon expected");
	NextToken();
	Insert(x,PROG_NAME,CONSTANT,x,NO,0);
}

// Production 3
void Consts(){ //token should be "const"
	if(token != "const")
		Error("keyword \"const\" expected");
	NextToken();
	if(!NonKeyID())
		Error("non-keyword identifier must follow \"const\"");
	ConstStmts();
}

// Production 4
void Vars(){ //token should be "var"
	if(token != "var")
		Error("keyword 'var' expected");
	NextToken();
	if(!NonKeyID())
		Error("non-keyword identifier  must follow 'var'");
	VarStmts();
}

// Production 5
void BeginEndStmt(){
	if (token != "begin")
		Error("keyword \"begin\" expected");
	else{
		if (beginCounter == 0)
			Code("program");
		beginCounter++;
	}NextToken();
	if(NonKeyID() || token == "read" || token == "write" || token == "if" || token == "while" || token == "repeat" || token == ";" || token == "begin"){
		EXEC_STMTS();
	}if(token != "end"){
		Error("keyword \"end\" expected");  // process error: keyword "end" expected
	}else{
		beginCounter--;
	}NextToken();
	
	if (beginCounter == 0 && token != ".")
		Error("\".\" expected");
	else if (beginCounter > 0 && token != ";")
		Error("\";\" expected");
	else if (beginCounter < 0 )
		Error("need a \"begin\" before this point"); 
	
	if (token == ".")
		Code("end");
	NextToken();
}

// Production 6
void ConstStmts(){ //token should be NON_KEY_ID
	string x,y;
	int spot = -1;
		
	if(!NonKeyID())
		Error("non-keyword identifier expected"); 
	
	x = token;
	
	if(NextToken() != "="){
		Error("\"=\" expected");
	}
	
	y = NextToken();
	
	if(y != "+" && y != "-" && y !="not" && !NonKeyID() && !isBoolean() && !isInteger())
		Error("token to right of \"=\" illegal");
			
	if(y == "+" || y == "-"){
	NextToken();
	if(!isInteger())
		Error("integer expected after sign");
	y = y + token;
	}
	if(y == "not"){
		NextToken();
		for(uint i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].externalName == token)
				spot = i;
		}
		
		if(spot != -1){
			if(symbolTable[spot].dataType != BOOLEAN)
				Error("boolean expected after not");
		}

		else if(!isBoolean())
			Error("boolean expected after not");
			
		// token was external name?
		if(spot != -1){
			if(symbolTable[spot].value == "true")
				y = "false";
			else if(symbolTable[spot].value == "false")
				y = "true";
			else
				Error("invalid value forBOOLEAN type");
		}
		else if(token == "true"){ 
			y = "false";
		}else{
				y = "true";
		}
	}
	if(NextToken() != ";")
		Error("\":\" expected");
			
	Insert(x,WhichType(y),CONSTANT,WhichValue(y),YES,1);
		
	NextToken();
		
	if(token != "begin" && token != "var" && !NonKeyID())
		Error("non-keyword identifier, \"begin\", or \"var\" expected");

	if(NonKeyID())
		ConstStmts();
}

// Production 7
void VarStmts(){	//token should now be NON_KEY_ID
	string x, y;
	if(!NonKeyID())
		Error("non-keyword identifier expected");
	x = Ids();
	if(token != ":")
		Error("\":\" expected");
	NextToken();
	if(token != "integer" && token != "boolean")
		Error("illegal type follows \":\"");
	y = token;
	if(NextToken() != ";")
		Error("semicolon expected");
	
	if(y == "integer")
		Insert(x, INTEGER, VARIABLE, "", YES, 1);
	else if(y == "boolean")
		Insert(x, BOOLEAN, VARIABLE, "", YES, 1);
	else if(y == "program")
		Insert(x, PROG_NAME, VARIABLE, "", NO, 0);
	else
		Error("not a valid storeType");
	
	NextToken();
	//ifthis doesn't work, put nexttoken before the if, and replace it with token
	if(token != "begin" && !NonKeyID())
		Error("non-keyword identifier or \"begin\" expected");
	if(NonKeyID())
		VarStmts();
}

// Production 8
string Ids(){ //token should be NON_KEY_ID
	string temp,tempString;
	
	if(!NonKeyID())
		Error("non-keyword identifier expected");
	tempString = token;
	temp = token;
	if(NextToken() == ",")
	{
		NextToken();
		if(!NonKeyID())
			Error("non-keyword identifier expected");
		tempString = temp + "," + Ids();
	}
	return tempString;
}

// Inserts a new entry into the symbol table
void Insert(string externalName, storeType inType, modes inMode, string inValue,
 allocation inAlloc, int inUnits)
{
    string name;
    string::iterator end = externalName.end();
    for(string::iterator a = externalName.begin(); a < externalName.end(); a++){
        name = ""; //initialize a new name
        while((*a != ',') && (a < end) ){ //fill in name appropriately
			name += *a;
			a++;
        }
		if(!name.empty()){
			if(name.length() > 15){ //ifthe name is too big (over 15), ignore characters past 15
				name = name.substr(0,15);
			}
			for(int i = 0; i < symbolTable.size(); i++){ //ifthe name is already there, error!
				if(symbolTable[i].externalName == name){
					Error("multiple name definition");
				}
			}   
			if(find(keys, keys + keyCount, name) != keys + keyCount){ //see ifname matches keys
				Error("illegal use of keyword"); //ifso, error
			}
			else{ //otherwise, set up the push_back
				entry my;
				if(name.length() > 15){
					my.externalName = name.substr(0,15);
				}else{
					my.externalName = name;
				}if(isupper(name[0])){
					my.internalName = name;
				}else{
					my.internalName = genInternalName(inType);
				}
				if(inValue.length() > 15){
				  my.value = inValue.substr(0,15);
				}else{
				  my.value = inValue;
				}
				my.dataType = inType;
				my.mode = inMode;
				my.alloc = inAlloc;
				my.units = inUnits;
				symbolTable.push_back(my);
			}
			++overflow;
			if(overflow > 256)
				Error("symbol table entries have exceeded the maximum allowed value");
		}
	}
}

// Determines which data type "name" has
storeType WhichType(string name){
	string::iterator it;
	vector<entry>::iterator vit;
	bool isInteger = true;
	
	if(name == "true" || name == "false")
		return BOOLEAN;
	if(isdigit(name[0]) || name[0] == '+' || name[0] == '-')
	{
		for(it = name.begin() + 1; it < name.end(); ++it)
		{
			if(!isdigit(*it))
				isInteger = false;
		}
		
		if(isInteger)
			return INTEGER;
	}
	
	for(vit = symbolTable.begin(); vit < symbolTable.end(); ++vit)
	{
		if(name == (*vit).externalName)
			return (*vit).dataType;
	}
	
	Error("reference to undefined constant1");
}


// Determines which value "name" has
string WhichValue(string name)
{
	string::iterator it;
	vector<entry>::iterator vit;
	bool isLiteral = true;

	if(name == "true" || name == "false")
		isLiteral = true;
	else if(isdigit(name[0]) || name[0] == '+' || name[0] == '-')
	{
		for(it = name.begin() + 1; it < name.end(); ++it)
		{
			if(!isdigit(*it))
				isLiteral = false;
		}
		
		if(isLiteral)
			return name;
	}
	
	for(vit = symbolTable.begin(); vit < symbolTable.end(); ++vit)
	{
		if(name == (*vit).externalName)
			return (*vit).value;
	}
	
	if(name == "")
		Error("reference to undefined constant2");
	
	return name;
	
}

// Returns the next token or end of file marker
string NextToken(){ 
	token = "";
	while (token == ""){
		if(charac == '{'){ //process comment
			while(true){
				NextChar();
				if(charac == END_OF_FILE){
					break;
				}else if(charac == '}'){
					NextChar();
					break;
				}
			}if(charac == END_OF_FILE){
				Error("unexpected end of file");
			}else if(charac == '}'){
				Error("token can't start \"}\"");
			}
		}else if(charac == '}'){
			Error("\"}\" cannot begin token");
		}else if(isspace(charac)){
			NextChar();
		}else if(isSpecial(charac)){            
			token = charac;
            NextChar();
        }else if(charac == ':'){
			token = charac;
			NextChar();
			if(charac == '='){
				token+=charac;
				NextChar();
			}
		}else if(charac=='<'){
			token = charac;
			NextChar();
			if(charac=='=' || charac == '>'){
				token+=charac;
				NextChar();
			}
		}else if(charac=='>'){
			token = charac;
			NextChar();
			if(charac=='='){
					token+=charac;
					NextChar();
			}
		}else if(charac == '_'){ //no leading _
			Error("\'_\' cannot start an identifier");
		}else if(isalpha(charac)){
			token = charac;
			charac = NextChar();
			if(charac == '_'){
				Error("\'_\' cannot start an identifier");
			}while(isLetter(charac)){ //search lowercase, nums, and spaces. ifit's none of these, than npos is reached.
				token+=charac;
				NextChar();
			}
		}else if(isdigit(charac)){
			token = charac;
			while(isdigit(NextChar())){
				token += charac;
				}
		}else if(charac == END_OF_FILE){
			token = charac;
		}else{
			Error("illegal symbol");
		}
	}if(token[0] == '_'){ //no start _
		Error("\"_\" cannot start an identifier");
	}if(token[token.length() - 1] == '_'){ //no end _
		Error("\"_\" cannot end an identifier");
	}return token;
}

//special character?
bool isSpecial(char chara){
	return (chara == ',' || charac == ';' || charac == '=' || charac == '+' || charac == '-' ||
		charac == '.' ||  charac =='(' || charac ==')'  || charac == '*');
}

//lowercase
bool isLetter(char chara){
	return (islower(chara) || isdigit(chara) || chara == '_');
}

char NextChar(){
	char myNext;
	sourceFile.get(myNext);
	
	//http://www.cplusplus.com/reference/ios/ios/good/
	if(!sourceFile.good()){
		charac = END_OF_FILE;
	}else{
		prevCharac = charac;
		charac = myNext;		
		if(lstLineNum == 0){
			lstLineNum++;
			listingFile << setw(5) << right << lstLineNum << '|';
		}else if(prevCharac == '\n'){
			lstLineNum++;
			listingFile << setw(5) << right << lstLineNum << '|';
		}
		listingFile << charac;
	}
	return charac;
}

// Displays errors to the listing file
void Error(string error){
	errorFound = true;
	listingFile << "\nError: Line " << lstLineNum << ": " << error << "\n";
	CreateListingTrailer();
	sourceFile.close();
	listingFile.close();
	objectFile.close();
	exit(EXIT_FAILURE);
}

// Generates the internal name
string genInternalName(storeType genType){
	ostringstream myOut;
	
	if(genType == INTEGER){
		myOut << "I" << intCount;
		intCount++;
	}else if(genType == BOOLEAN){
		myOut << "B" << boolCount;
		boolCount++;
	}else if(genType == PROG_NAME){
		if(progFound == false){
			myOut << "P0";
			progFound = true;
		}else{
			Error("only one program name allowed");
		}
	}
	return myOut.str();
}

// Returns true if token is a NonKeyID, and false otherwise
bool NonKeyID()
{
    if(token[0]=='_')
        Error("cannot begin with \"_\" ");
       
    if(isupper(token[0]))
		Error("upper case characters not allowed");
	
    if(!isalpha(token[0]))
		return false;
	
	
    // go through each char
    for(int x = 1; x < (int)token.length(); x++)
    {
        if(isupper(token[x]))
            Error("upper case characters not allowed");
        if( !isalpha(token[x]) && !isdigit(token[x]) && token[x] != '_' )
            return false;
    }
    return(find(keys, keys + keyCount, token) != keys + keyCount)? false : true; //token isn't key ID
}

//is it a boolean type?
bool isBoolean(){
	return(token=="true" || token == "false");
}

//all chars must be a digit forit to be an integer type. is it so?
bool isInteger(){
	for(int i = 0; i < (int)token.length(); i++){
		if(!isdigit(token[i]))
			return false;
	}
	return true;
}
      
/******************************************* END OF STAGE 0 Code ********************************************************/
      
/******************************************* START STAGE 1 Code **********************************************************/
const string EXPRESSFail = "Invalid expression: \"not\", \"true\", \"false\", \"(\", \"+\", \"-\", non-key ID, or integer expected";
//2
void EXEC_STMTS()
{
	if(NonKeyID() || token == "read" || token == "write" || token == "if" || token == "while" || token == "repeat" || token == "begin" ||  token == ";")
	{
		EXEC_STMT();
		EXEC_STMTS();
	}
	else if (token == "end" || token == "until")
	{
		;
	}
	else
	{
		Error("non-keyword identifier, \"read\", \"write\", or \"begin\" expected");
	}
}
      
void EXEC_STMT(){
	if(NonKeyID())
	{
		ASSIGN_STMT(); //ifit's not a keyID, assign it!
	}
	else if(token == "read")
	{
		READ_STMT();
	}
	else if(token == "write")
	{
		WRITE_STMT();
	}
	else if (token == "if")
	{
		IF_STMT();
	}
	else if (token == "while")
	{
		WHILE_STMT();
	}
	else if (token == "repeat")
	{
		REPEAT_STMT();
	}
	else if(token == ";")
	{
		NextToken(); //advance ;
	}
	else if(token == "begin")
	{
		BeginEndStmt();
	}
	else
	{
		Error("non-keyword id, read, or write statement expected");
	}
	
    if (token == ";")
        NextToken();
}
      
//4
void ASSIGN_STMT(){
	if(!NonKeyID())//check fornon key
		Error("non_key_id expected");
	string t = "" + token; //holds token
	pushOperand(t); //push the token and move on
	NextToken();
	if(token != ":=")//token HAS to be := here forassignment
		Error("Expected assignment operator");
	else
		pushOperator(":=");
		
	NextToken();
	if(isEXPRESS())
		EXPRESS();
	else
		Error(EXPRESSFail); //need proper keywords	
	if(token != ";")
		Error("\";\" expected ");   //expected semicolon
			
	Code(popOperator(), popOperand(), popOperand());
}
      
//5
void READ_STMT(){
	if(token!= "read")//to read, token must be read.
		Error("expected \"read\" forread statement");
	NextToken();	//see what's being read
	if(token != "(")//must be (forfunction
		Error("\"(\" expected");
	READ_LIST(); //read the list
	if(NextToken()!=  ";"){ //should be over
		Error("Expected ;");
	}
}
      
//6
void READ_LIST(){
	if(token!= "(")//currently must be at (
		Error("Expected (");
	NextToken(); //ifso, proceed
	string temp = Ids();
	if(token!= ")"){
		Error("Expected \",\" or \")\"");
	}else
		Code("read", temp);
}
      
//7
void WRITE_STMT(){
	if(token != "write")
		Error("Expected \"write\"");
	NextToken();	
	if(token != "(")
		Error("\"(\" expected");
		
	WRITE_LIST();
	if(NextToken()!=  ";")
		Error("Expected \";\"");
}
       
//8
void WRITE_LIST(){
	if(token != "(")
		Error("Expected \"(\"");
	
	NextToken();
	string temp = Ids();
	if(token != ")"){ //is "," needed to be tested here?
		Error("Expected \",\" or \")\"");
	}else   
		Code("write", temp);
}
      
//9
void EXPRESS(){
	if(token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger() && !NonKeyID())
		Error(EXPRESSFail);
	TERM();
	
	if(token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">"){
		EXPRESSES();
	}else if(token == ")" || token == ";" || token == "then" || token == "do" || token == "until" || token == "begin"){
		;
	}
	else
	{
		Error("Invalid expression1");
	}
}
       
//10
void EXPRESSES(){
	if(token != "<>" && token != "=" && token != "<=" && token != ">=" && token != "<" && token != ">")
		Error("<>, =, <=, >=, <, or > expected");
	
	pushOperator(token);
	NextToken();
	
	if(token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger() && !NonKeyID() )
		Error("Invalid expression:  'not', 'true', 'false', '(', '+', '-', integer, or non-keyword id expected");
	else
		TERM();
	
	Code(popOperator(), popOperand(), popOperand());
	
	if(token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">")
	{
		EXPRESSES();
	}
	else if(!(token == ")" || token == ";" || token == "then" || token =="do" || token =="until"))
	{
		Error("Invalid expression");
	}
}
      
//11
void TERM(){
	if(token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger() && !NonKeyID() )
		Error(EXPRESSFail);
		
	FACTOR();
		
	if(token == "-" || token == "+" || token == "or"){
		TERMS();
	}else if(token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">" || token == ")" || token == ";" || token == "then" || token =="do" || token =="until" || token == "begin"){
		;
	}
	else
	{
		Error("Invalid expression3");
	}
}
       
//12
void TERMS(){
	if(!ADD_LEVEL_OP())
		Error("\"-\", \"+\", or \"or\" expected");
		
	pushOperator(token);
	NextToken();
		
	if(!isEXPRESS())
		Error(EXPRESSFail);
	else
		FACTOR();
		
	Code(popOperator(), popOperand(), popOperand());
	
	if(ADD_LEVEL_OP()){
		TERMS();
	}else if(!REL_OP() && token != ")" && token != ";"){
		Error("Invalid expression4");
	}
}
      
//13
void FACTOR(){
	if(!isEXPRESS())
		Error(EXPRESSFail);
			
	PART();
	if(MULT_LEVEL_OP()){
		FACTORS();
	}else if(token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">" || token == ")" || token == ";" || token == "-" || token == "+" || token == "or" || token == "then" || token =="do" || token =="until" || token == "begin"){
		;
	}
	else
	{
		Error("Invalid expression5");
	}
}
       
//14
void FACTORS(){
	if(!MULT_LEVEL_OP())
		Error("invalid FACTORS: \"*\",\" div\", \"mod\", or \"and\" expected");
	
	pushOperator(token);
	NextToken();
	
	if(!isEXPRESS())
		Error(EXPRESSFail);
	else
		PART();
	
	Code(popOperator(), popOperand(), popOperand());
		
	if(MULT_LEVEL_OP()){
		FACTORS();
	}else if(!REL_OP() && !(token == ")" || token == ";") && !ADD_LEVEL_OP()){
		Error("Multiplicative level operator expected");
	}
}
      
//15
void PART(){
	if(token == "not"){
		NextToken();
		if(token == "("){
			NextToken();
			if(!isEXPRESS())
				Error(EXPRESSFail);
			EXPRESS();
			if(token !=")")
				Error("\")\" expected");
			NextToken();
			Code("not", popOperand());
		}else if(isBoolean()){
			if(token == "true"){
				pushOperand("false");
				NextToken();
			}else{
				pushOperand("true");
				NextToken();
			}
		}else if(NonKeyID()){
			Code("not", token);
			NextToken();
		}
	}else if(token == "+"){
		NextToken();
		if(token == "("){
			NextToken();
			if(!isEXPRESS())
				Error(EXPRESSFail);
			EXPRESS();
			if(token != ")"){
				Error("\")\" expected");
			}
			NextToken();
		}else if(isInteger()|| NonKeyID()){
			pushOperand(token);
			NextToken();
		}else{
			Error("\"(\", integer, or non-keyword id expected");
		}
	}else if(token == "-"){
		NextToken();
		if(token == "("){
			NextToken();
			if(!isEXPRESS())
					Error(EXPRESSFail);
			EXPRESS();
			if(token != ")")
				Error("\")\" expected");
			NextToken();
			Code("neg", popOperand());
		}else if(isInteger()){
			pushOperand("-" + token);
			NextToken();
		}else if(NonKeyID()){
			Code("neg", token);
			NextToken();
		}
	}else if(isInteger()|| isBoolean()|| NonKeyID()){
		pushOperand(token);
		NextToken();
	}else if(token == "("){
		NextToken();
		if(!isEXPRESS())
			Error(EXPRESSFail);
		EXPRESS();
		if(token !=")"){
			Error("\")\" expected");
		}else
		NextToken();
	}else{
		Error("Illegal keyword");
	}
}

//16
bool REL_OP(){
	return (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">");
}

//17
bool ADD_LEVEL_OP(){
	return (token == "-" || token == "+" || token == "or");
}

//18
bool MULT_LEVEL_OP(){
	return (token == "*" || token == "div" || token == "mod" || token == "and");
}
    
//code
void Code(string myOperator, string operand1, string operand2){
    if (myOperator == "program")
    {
        EmitProgramCode();
    }
    else if (myOperator == "end")
    {
        EmitEndCode();
    }
    else if (myOperator == "read")
    {
        EmitReadCode(operand1);
    }
    else if (myOperator == "write")
    {
        EmitWriteCode(operand1);
    }
    else if (myOperator == "+")
    {
        if (operand1 == "" || operand2 == "")
            Error("addition requires two operands");
              
        EmitAdditionCode(operand1,operand2);
    }
    else if (myOperator == "-")
    {
        if (operand1 == "" || operand2 == "")
            Error("subtraction requires two operands");
               
        EmitSubtractionCode(operand1,operand2);
    }
    else if (myOperator == "neg")
    {
        if (operand2 != "")
            Error("negation takes one operand");
        EmitNegCode(operand1);
    }
    else if (myOperator == "not")
    {
        if (operand2 != "")
            Error("not operation takes one operand");
        EmitNotCode(operand1);
    }
    else if (myOperator == "*")
    {
        if (operand1 == "" || operand2 == "")
            Error("addition requires two operands");
        EmitMultiplicationCode(operand1,operand2);
    }
    else if (myOperator == "div")
    {
        if (operand1 == "" || operand2 == "")
            Error("division requires two operands");
        EmitDivisionCode(operand1,operand2);
    }
    else if (myOperator == "mod")
    {
        if (operand1 == "" || operand2 == "")
            Error("mod requires two operands");
        EmitModuloCode(operand1,operand2);
    }
    else if (myOperator == "and")
    {
        if (operand1 == "" || operand2 == "")
            Error("and requires two operands");
        EmitAndCode(operand1,operand2);
    }
    else if (myOperator == "or")
    {
        if (operand1 == "" || operand2 == "")
            Error("or requires two operands");
        EmitOrCode(operand1,operand2);
    }
    else if (myOperator == ".")
    {
        ;
    }
    else if (myOperator == "=")
    {
        if (operand1 == "" || operand2 == "")
            Error("addition requires two operands");
        EmitEqualityCode(operand1,operand2);
    }
    else if (myOperator == ":=")
    {
        if (operand1 == "" || operand2 == "")
            Error("addition requires two operands");
        EmitAssignCode(operand1, operand2);
    }
    else if (myOperator == "<>")
    {
        if (operand1 == "" || operand2 == "")
            Error("<> requires two operands");
        EmitNotEqualCode(operand1,operand2);
    }
    else if (myOperator == ">")
    {
        if (operand1 == "" || operand2 == "")
            Error("> requires two operands");
        EmitGreaterThanCode(operand1,operand2);
    }
    else if (myOperator == ">=")
    {
        if (operand1 == "" || operand2 == "")
            Error(">= requires two operands");
        EmitGreaterThanEqualCode(operand1,operand2);
    }
    else if (myOperator == "<")
    {
        if (operand1 == "" || operand2 == "")
            Error("< requires two operands");
        EmitLessThanCode(operand1,operand2);
    }
    else if (myOperator == "<=")
    {
        if (operand1 == "" || operand2 == "")
            Error("<= requires two operands");
        EmitLessThanEqualCode(operand1,operand2);
    }
    //Stage 2
    else if (myOperator =="then")
    {
        EmitThenCode(operand1);
    }
    else if (myOperator =="else")
    {
        EmitElseCode(operand1);
    }
    else if (myOperator =="post_if")
    {
        EmitPostIfCode(operand1);
    }
    else if (myOperator =="while")
    {
        EmitWhileCode();
    }
    else if (myOperator =="do")
    {
        EmitDoCode(operand1);
    }
    else if (myOperator == "post_while")
    {
        EmitPostWhileCode(operand1, operand2);
    }
    else if (myOperator =="repeat")
    {
        EmitRepeatCode();
    }
    else if (myOperator =="until")
    {
        EmitUntilCode(operand1,operand2);
    }
    else
    {
		popOperator();
        Error("undefined operation");
    }
}
      
//**************Brendan**************

// add operand1 to operand2
void EmitAdditionCode(string operand1, string operand2)
{
	// iftype of either operand is not integer, process error : illegal type
	for(int i = 0; i < symbolTable.size(); ++i)
	{
	    if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2)
	    {
	        if(symbolTable[i].dataType != INTEGER)
	        {
	            Error("illegal type");
	        }
	    }
	}
		
	// ifA register holds a temp not operand1 nor operand2 then...
	if(regA != operand1 && regA != operand2 && regA[0] == 'T' && regA != "TRUE")
	{
	    objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign A-reg\n";
	    
	    for(int i = 0; i < symbolTable.size(); ++i)
	    {
	        if(symbolTable[i].internalName == "regA")
	        {
	            symbolTable[i].alloc = YES;
	        }
	    }
	}
	
	// ifA register holds a non-temp not operand1 nor operand2 then deassign it      ????????????
	
	if(regA != operand1 && regA != operand2)
	{
	    regA = operand2;
	    objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << regA << setw(5) << " " << "load opL into A-reg\n";
	}
	
	// emit code to perform register-memory addition
	if(regA == operand1)
	{
	    objectFile << left << setw(6) << " " << setw(3) << "IAD " << setw(4) << operand2 << setw(5) << " " << "add opL to opR\n";
	}
	else
	{
	    objectFile << left << setw(6) << " " << setw(3) << "IAD " << setw(4) << operand1 << setw(5) << " " << "add opR to opL\n";
	}
	
	// Free temp variables used in the operation
	if(operand1[0] == 'T' && operand1 != "TRUE")
	{
	    FreeTemp();
	}
	
	if(operand2[0] == 'T' && operand2 != "TRUE")
	{
	    FreeTemp();
	}
	
	// Get the next available temp and assign it to the A-reg
	regA = GetTemp();
	
	// Set the data type of A-reg to INTEGER
	for(int i = 0; i < symbolTable.size(); ++i)
	{
	    if(symbolTable[i].internalName == regA)
	    {
	        symbolTable[i].dataType = INTEGER;
	    }
	}
	
	// push the name of the result onto operandStk
	pushOperand(regA);
}

void EmitDivisionCode(string operand1, string operand2)
{
    // iftype of either operand is not integer
    for(int i = 0; i < symbolTable.size(); ++i)
    {
        if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2)
        {
            if(symbolTable[i].dataType != INTEGER)
            {
                Error("illegal type");
            }
        }
    }
    
    // ifregA holds a temp not operand 2...
    if(regA != operand2 && regA[0] == 'T' && regA != "TRUE")
    {
        objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign A-reg\n";
        
        for(int i = 0; i < symbolTable.size(); ++i)
        {
            if(symbolTable[i].internalName == regA)
            {
                symbolTable[i].alloc = YES;
            }
        }
    }
    
    // ifregA holds a non-temp not operand 2 then deassign it    ???????????????
    
    // ifopL is not in regA
    if(regA != operand2)
    {
        // emit instruction to do a register-memory load of operand2 into regA
        objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand2 << setw(5) << " " << "load opL into A-reg\n";
    }
    
    // emit code to do a register-memory division
    objectFile << left << setw(6) << " " << setw(3) << "IDV " << setw(4) << operand1 << setw(5) << " " << "divide opL by opR\n";
    
  // Free temp variables used in the operation
	if(operand1[0] == 'T' && operand1 != "TRUE")
	{
	    FreeTemp();
	}
	
	if(operand2[0] == 'T' && operand2 != "TRUE")
	{
	    FreeTemp();
	}
	
	// Set the data type of A-reg to INTEGER
	for(int i = 0; i < symbolTable.size(); ++i)
	{
	    if(symbolTable[i].internalName == regA)
	    {
	        symbolTable[i].dataType = INTEGER;
	    }
	}
	
	// push the name of the result onto operandStk
	pushOperand(regA);
}

void EmitMultiplicationCode(string operand1, string operand2)
{
	// iftype of either operand is not integer, process error : illegal type
	for(int i = 0; i < symbolTable.size(); i++)
	{
	    if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2)
	    {
	        if(symbolTable[i].dataType != INTEGER)
	        {
	            Error("illegal type");
	        }
	    }
	}
		
	// ifA register holds a temp not operand1 nor operand2 then...
	if(regA != operand1 && regA != operand2 && regA[0] == 'T' && regA != "TRUE")
	{
	    objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign A-reg\n";
	    
	    for(int i = 0; i < symbolTable.size(); ++i)
	    {
	        if(symbolTable[i].internalName == "regA")
	        {
	            symbolTable[i].alloc = YES;
	        }
	    }
	}
	
	// ifA register holds a non-temp not operand1 nor operand2 then deassign it      ????????????
	
	if(regA != operand1 && regA != operand2)
	{
	    regA = operand2;
	    objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << regA << setw(5) << " " << "load opL into A-reg\n";
	}
	
	// emit code to perform register-memory addition
	if(regA == operand1)
	{
	    objectFile << left << setw(6) << " " << setw(3) << "IMU " << setw(4) << operand2 << setw(5) << " " << "multiply opL and opR\n";
	}
	else
	{
	    objectFile << left << setw(6) << " " << setw(3) << "IMU " << setw(4) << operand1 << setw(5) << " " << "multiply opR and opL\n";
	}
	
	// Free temp variables used in the operation
	if(operand1[0] == 'T' && operand1 != "TRUE")
	{
	    FreeTemp();
	}
	
	if(operand2[0] == 'T' && operand2 != "TRUE")
	{
	    FreeTemp();
	}
	
	// Get the next available temp and assign it to the A-reg
	regA = GetTemp();
	
	// Set the data type of A-reg to INTEGER
	for(int i = 0; i < symbolTable.size(); i++)
	{
	    if(symbolTable[i].internalName == regA)
	    {
	        symbolTable[i].dataType = INTEGER;
	    }
	}
	
	// push the name of the result onto operandStk
	pushOperand(regA);
}

// push name onto operatorStk
void pushOperator(string name){
    Operator.push(name);
}

string GetMyJump(){
    string myLabel;
    stringstream s;
    
    myLabelCounter++;
    s << myLabelCounter;
    myLabel = 'L' + s.str();
    
    return myLabel;
}

void EmitSubtractionCode(string operand1, string operand2)
{
	// iftype of either operand is not integer, process error : illegal type
	for(int i = 0; i < symbolTable.size(); ++i)
	{
		if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2)
		{
			if(symbolTable[i].dataType != INTEGER)
			{
				Error("illegal type");
			}
		}
	}
		
	// ifA register holds a temp not operand1 nor operand2 then...
	if(regA != operand1 && regA != operand2 && regA[0] == 'T' && regA != "TRUE")
	{
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign A-reg\n";
		
		for(int i = 0; i < symbolTable.size(); ++i)
		{
			if(symbolTable[i].internalName == regA)
			{
				symbolTable[i].alloc = YES;
			}
		}
	}
	
	// ifA register holds a non-temp not operand1 nor operand2 then deassign it      ????????????
	
	if(regA != operand2){
		regA = operand2;
		objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand2 << setw(5) << " "<< '\n'; // load operand2
	}
		
	if(regA == operand2){
		objectFile << left << setw(6) << " " << setw(3) << "ISB " << setw(4) << operand1 << setw(5) << " "<<operand2<<" - "<<operand1<< endl;
	}
	
	if(operand1[0] == 'T' && operand1 != "TRUE"){
		FreeTemp();
	}
	if(operand2[0] == 'T' && operand2 != "TRUE"){
		FreeTemp();
	}
	
	regA = GetTemp();
	
	for(int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA)
				symbolTable[i].dataType=INTEGER; //change temp in regA alloc to yes in table
	}
	
	pushOperand(regA);
	objectFile << right;//set back to default
}

void EmitEndCode()
{
    istringstream s;
    string opcode ="";
    int value = 0;
   
     objectFile << left << setw(6) << " " <<  setw(13) << "HLT" << '\n';
     
     for(int i = 1; i < symbolTable.size(); i++)
     {
        if(symbolTable[i].alloc == YES)
        {
            if(symbolTable[i].mode == CONSTANT)
            {
                opcode = "DEC";
                if(symbolTable[i].value == "true" || symbolTable[i].value == "TRUE")
                {
                    value = 1;
                }
                else if(symbolTable[i].value == "false" || symbolTable[i].value == "FALSE")
                {
                    value = 0;
                }
                else
                {
                    s.clear();
                    s.str(symbolTable[i].value);
                    s >> value;
                }
            }
            else
            {
                opcode = "BSS";       
                value = 1;
            }
               
               
                   
            objectFile << left << setw(6) << symbolTable[i].internalName <<  setw(4) << opcode;
   
            if(symbolTable[i].value[0] == '-')
            {
                objectFile << '-' << right << setw(3) << setfill('0') << abs(value);
            }
                
            else
            {
                objectFile << right << setw(4) << setfill('0') << value;
            }
                
            objectFile << left << setw(5) << setfill(' ') << " ";
  
            if(isupper(symbolTable[i].externalName[0]) && symbolTable[i].value != "")
            {
                objectFile << symbolTable[i].value +'\n';
            }
            else
            {
                objectFile << symbolTable[i].externalName + "\n";
            }
        }
     }
     
     objectFile << "      END STRT\n";
       
     // set back to default
     objectFile << right;
}

void EmitEqualityCode(string operand1, string operand2)
{
    
    int op1 = -1;
    int op2 = -1;
    int numTrue = -1;
    int numFalse = -1;
    string myLabel;
   
    for(int i = 0; i < symbolTable.size(); ++i)
	  {
        if(symbolTable[i].internalName == operand1)
		    {
			    op1 = i;
	    	}
          
        if(symbolTable[i].internalName == operand2)
		    {
		    	op2 = i;
	    	}
              
        if(symbolTable[i].internalName == "TRUE")
	    	{
	    		numTrue = i;
	    	}
          
        if(symbolTable[i].internalName == "FALS")
    		{
		    	numFalse = i;
	    	}
   }
       
    // ifeither operand is undefined...
    if(op1 == -1 || op2 == -1)
	  {
		  Error("illegal character");
	  }
	
    // ifmismatched types...
    if(symbolTable[op1].dataType != symbolTable[op2].dataType)
	  {
		  Error("type mismatch");
  	}
       
    // ifregA holds a temp variable not op1 or op2
    if(regA[0] == 'T' && regA != operand1 && regA != operand2 && regA != "TRUE")
	  {
        objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n";
        for(int i = 0; i < symbolTable.size(); ++i)
		    {
            if(symbolTable[i].internalName == regA)
		      	{
                symbolTable[i].alloc=YES;
            }
        }
    }
      
    //ifregA holds neither operand1 or operand2...
    if(regA != operand1 && regA != operand2)
	  {
            regA = operand2;
            objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand2 << setw(5) << " "<< '\n'; // load operand2
    }
      
    //ifregA holds op2, then subtract op1
    if(regA == operand2)
	{
        objectFile << left << setw(6) << " " << setw(4) << "ISB" << setw(4) << operand1 << setw(5) << " " << operand2 << " = " << operand1 << endl;
    }
	else
	{
        objectFile << left << setw(6) << " " << setw(4) << "ISB" << setw(4) << operand2 << setw(5) << " " << operand2 << " = " << operand1 << endl;
    }
      
    //get myLabel forjump
    myLabel = GetMyJump();
      
    objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << myLabel << setw(5) << " " << '\n'; // jump to myLabel
	
    if(numFalse == -1)
    {
        Insert("FALS", BOOLEAN, CONSTANT, "FALSE", YES, 1);
        regA = "FALS";
        objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << setw(5) << " " << '\n';
    }
    else
    {
        objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << setw(5) << " " << '\n';
    }
	
    objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << myLabel << "+1" << setw(3) << " " << '\n';
      
    if(numTrue == -1)
    {
        Insert("TRUE", BOOLEAN, CONSTANT, "TRUE", YES, 1);
        regA = "TRUE";
        objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n';
    }
    else
    {
        objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT true
    }
    
    if(operand1[0] == 'T' && operand1 != "TRUE")
	{
        FreeTemp();
    }
    if(operand2[0] == 'T' && operand2 != "TRUE")
	{
        FreeTemp();
    }
    
    regA = GetTemp();
    
    for(int i = 0; i < symbolTable.size(); ++i){
        if(symbolTable[i].internalName == regA)
		{
			symbolTable[i].dataType = BOOLEAN;
		}
    }
    
    pushOperand(regA);
	
    objectFile << right;
}

void EmitNegCode(string operand1)
{
	int operand = -1;
	string temp = "";
	
	for(int i = 0; i < symbolTable.size(); ++i)
	{
		if(symbolTable[i].internalName == operand1)
		{
			if(symbolTable[i].dataType != INTEGER)
			{
				Error("type mismatch");
			}  
		}
	}
	
	if(regA[0] == 'T' && regA != operand1 && regA != "TRUE")
	{
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n";
			
			for(unsigned int i = 0; i < symbolTable.size(); i++)
			{
				if(symbolTable[i].internalName == regA)
				{
					symbolTable[i].alloc = YES;
				}
			}
	}
	
	for(int i = 0; i < symbolTable.size(); ++i)
	{
		if(symbolTable[i].mode == CONSTANT && symbolTable[i].value == "-1" && symbolTable[i].dataType == INTEGER)
		{
			operand = i;
		}
	}
	
	for(int i = 0; i < symbolTable.size(); ++i)
	{
		if(symbolTable[i].mode == VARIABLE && symbolTable[i].externalName == operand1 && symbolTable[i].dataType == INTEGER)
      operand1 = symbolTable[i].internalName;
	}
	
	
	if(operand == -1)
	{
		temp = genInternalName(INTEGER);
		Insert(temp, INTEGER, CONSTANT, "-1", YES, 1);
	}
	
	if(regA != operand1)
	{
	  regA = operand1;
		objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand1 << setw(5) << " " << "\n";
	}
	
	objectFile << left << setw(6) << " " << setw(3) << "IMU " << setw(4) << temp << setw(5) << " " << temp + " * -1\n";
	
	regA = GetTemp();
	
	for(int i = 0; i < symbolTable.size(); ++i)
	{
		if(symbolTable[i].internalName == regA)
		{
			symbolTable[i].dataType = INTEGER;
		}
	}
	
	pushOperand(regA); 
	
	objectFile << right;
}

void EmitWriteCode(string ids)
{
    string s;
    int myIndex;
    string::iterator it;
          
    for (it = ids.begin(); it < ids.end(); ++it)
    {
        s = "";
              
        while((*it != ',') && (it < ids.end()))
        {
			s = s + *it;
            ++it;
        }
          
        if (s.length() > 15)
		{
			s = s.substr(0, 15);
		}
          
        if (s != "")
        {
            myIndex = -1;
               
            for (int i = 0; i < symbolTable.size(); ++i)
            {
                if (symbolTable[i].externalName == s)
				{
					myIndex = i;
				}
            }
               
            if(myIndex == -1)
			{
				Error("undefined symbol");
			}
            else
			{
				objectFile << left << setw(6) << " " <<  setw(4) << "PRI" << setw(4) << symbolTable[myIndex].internalName << setw(5) << setfill(' ') << " " << "write out " + symbolTable[myIndex].externalName + "\n";
			}
        }
    }
	
    objectFile << right;
}

void EmitReadCode(string ids)
{
    string s;
    int myIndex;
    string::iterator it;
          
          
    for (it = ids.begin(); it < ids.end(); ++it)
    {
        s = "";
              
        while((*it != ',') && (it < ids.end()))
        {
			s = s + *it;
            ++it;
        }
          
        if (s.length() > 15)
		{
			s = s.substr(0, 15);
		}
              
        if (s != "")
        {
            myIndex = -1;
               
            for (int i = 0; i < symbolTable.size(); ++i)
            {
                if (symbolTable[i].externalName == s)
                {
                    if(symbolTable[i].mode == CONSTANT)
					{
						Error("cannot write to CONSTANT");
					}
                    else if(symbolTable[i].mode == VARIABLE)
					{
						myIndex = i;
					}
                }
            }
            
            if(myIndex == -1)
			{
				Error("undefined symbol");
			}
            else
			{
				objectFile << left << setw(6) << " " <<  setw(4) << "RDI" << setw(4) << symbolTable[myIndex].internalName << setw(5) << setfill(' ') << " " << "read in " + symbolTable[myIndex].externalName + "\n";
			} 
        }
    }
	
    objectFile << right;
}


//**************************EJ****************************

//push name onto operand stack PushOpAnd
void pushOperand(string name){
	int nameLocation = -1;
	string internalName = "";
	
	if(name[0] == 'L'){
		Operand.push(name);
		return; //pushed, done!
	}
	
	if(name.length() > 15)
			name = name.substr(0,15);
	
	for(int i = 0; i<symbolTable.size(); i++){
		if(symbolTable[i].externalName == name)
			nameLocation = i;
	}
		
	if(nameLocation == -1){
		for(int i = 0; i<symbolTable.size(); i++){
			if((symbolTable[i].value == name) && (symbolTable[i].mode == CONSTANT))
				nameLocation = i;
		}
	}
	
	if(nameLocation == -1){
		if((name == "true") || (name == "false")) //boolean keyword
			name = (name=="true")?"TRUE":"FALSE";
			
		for(int i = 0; i < symbolTable.size(); i++){
			if((symbolTable[i].value == name) && (symbolTable[i].mode == CONSTANT)){
				nameLocation = i;
			}
		}
		if((name == "TRUE") || (name == "FALSE")) //to non boolean keyword
			name = (name=="TRUE")?"true":"false";
	}
	
	if(nameLocation == -1){ // INSERT IT IF IT'S NOT IN TABLE
		internalName = genInternalName(WhichType(name));
		Insert(internalName,WhichType(name),CONSTANT,WhichValue(name),YES,1);
		Operand.push(internalName);
	}else
		Operand.push(symbolTable[nameLocation].internalName);
}

//isLit
bool isLiteral(string name){
    string value = "";
    bool isInt = true;
    
    if(name == "true" || name == "false") //boolean?
        return true;
    else if(isdigit(name[0]) || name[0] == '+' || name[0] == '-'){ //integer?
        for(int x = 1; x < name.length(); x++){
            if(!isdigit(name[x])) //all chars in name digits?
                isInt = false;
        }
    return (isInt)?true:false; //ifthey're not, return false
    }
}
//popOpOr
string popOperator(){
	string temp = "";
	if(!Operator.empty()){
		temp = Operator.top();
		Operator.pop();
	}else{
		Error("operator stack underflow1");
	}
	return temp;
}

//popOpAnd
string popOperand(){
	string temp = "";	
	if(!Operand.empty()){
		temp = Operand.top();
		Operand.pop();
	}else{
		Error("operator stack underflow");
	}
	return temp;
}

//**********Jacob*****************

void EmitAndCode(string operand1, string operand2){
   
   for(int i = 0; i < symbolTable.size(); ++i){
        if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2){
            if(symbolTable[i].dataType != BOOLEAN){
                Error("operator and requires boolean operands");
        }
    }
}
     
    if(regA[0] == 'T' && regA != operand1 && regA != operand2 && regA != "TRUE"){
       objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; 
            for(int i = 0; i < symbolTable.size(); ++i){
                if(symbolTable[i].internalName == regA){
				            symbolTable[i].alloc = YES;  //change temp in regA alloc to yes in table     
        }
    }
}
       
    if(regA != operand2 && regA != operand1){
          regA = operand2;
           objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << operand2 << setw(5) << " " << '\n'; 
           
}
         
    if(regA == operand2){
        objectFile << left << setw(6) << " " << setw(4) << "IMU" << setw(4) << operand1 << setw(5) << " " << operand2 << " and " << operand1 << endl;
}
  	else{
        objectFile << left << setw(6) << " " << setw(4) << "IMU" << setw(4) << operand2 << setw(5) << " " << operand2 << " and " << operand1 << endl;
}
  
   if(operand1[0] == 'T' && operand1 != "TRUE"){
        FreeTemp();
}
   if(operand2[0] == 'T' && operand2 != "TRUE"){
        FreeTemp();
}
     
   regA = GetTemp();
     
   for(int i = 0; i < symbolTable.size(); ++i){
           if(symbolTable[i].internalName == regA)
		          	symbolTable[i].dataType = BOOLEAN; //change temp in regA alloc to yes in table
}
    pushOperand(regA);
      
    //set back to default setting and return
    objectFile << right;
    return;
}

void EmitOrCode(string operand1, string operand2){
    string myLabel;
    int numTrue = -1;
       
    for(int i = 0; i < symbolTable.size(); ++i){
        if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2){
            if(symbolTable[i].dataType != BOOLEAN){
                Error("illegal type");
    }
}
           
        //check to find BOOLEAN constant with value 1 or 0
        if(symbolTable[i].dataType == BOOLEAN && symbolTable[i].mode == CONSTANT){
            if(symbolTable[i].internalName == "TRUE"){
                if(numTrue == -1)
                    numTrue = i;
        }
    }
}
    
    if(regA[0] == 'T' && regA != operand1 && regA != operand2 && regA != "TRUE"){
        objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; // store regA
          
        for(int i = 0; i < symbolTable.size(); ++i){
            if(symbolTable[i].internalName == regA){
		          	symbolTable[i].alloc = YES;  //change temp in regA alloc to yes in table   
        }	
    }
}
           
    if(regA != operand2 && regA != operand1){
            regA = operand2;
            objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand2 << setw(5) << " "<< '\n'; //load operand2
            
}
        
    if(regA == operand2){
        objectFile << left << setw(6) << " " << setw(3) << "IAD " << setw(4) << operand1 << setw(5) << " "<< operand1 <<" or "<< operand1 << endl;
}
   	else{
        objectFile << left << setw(6) << " " << setw(3) << "IAD " << setw(4) << operand2 << setw(5) << " "<< operand1 <<" or "<< operand2 << endl;
}
      
    myLabel = GetMyJump();
    objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << myLabel << "+1" << setw(3) << " " << '\n'; // jump to line after myLabel
       
    if(numTrue == -1){
        Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
        objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n'; // LDA TRUE
}
    else{
        objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT true
}
    
    if(operand1[0] == 'T' && operand1 != "TRUE"){
        FreeTemp();
}
    if(operand2[0] == 'T' && operand2 != "TRUE"){
        FreeTemp();
}
    
    regA = GetTemp();
    
    for(int i = 0; i < symbolTable.size(); ++i){
            if(symbolTable[i].internalName == regA)
               symbolTable[i].dataType = BOOLEAN; //change temp in regA alloc to yes in table
}
    
    pushOperand(regA);
      
     //set back to default setting and return
    objectFile << right;
    return;
}

void EmitEqualsCode(string operand1, string operand2){
    
    int opR = -1;
    int opL = -1;
    int numTrue = -1;
    int numFalse = -1;
    string myLabel;
   
    // find index of operands in symbolTable and locate ifTRUE or FALS is in table
    for(int i = 0; i < symbolTable.size(); ++i){
        if(symbolTable[i].internalName == operand1)
            opR = i;
          
        if(symbolTable[i].internalName == operand2)
            opL = i;
              
        if(symbolTable[i].internalName == "TRUE")
            numTrue = i;
          
        if(symbolTable[i].internalName == "FALS")
            numFalse = i;
    }
      
    // ifeither operand is undefined
    if(opR == -1 || opL == -1)
        Error("undefined symbol in equality statement");
    // ifthe types of operands are mismatched
    if(symbolTable[opR].dataType != symbolTable[opL].dataType)
        Error("Incompatible types forequality");
      
    //ifregA holds a TEMP variable not operand1 or operand2 store it
    if(regA[0] == 'T' && regA != operand1 && regA != operand2 && regA != "TRUE"){
        objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; // store regA
        for(int i = 0; i < symbolTable.size(); ++i){
            if(symbolTable[i].internalName == regA){
                symbolTable[i].alloc = YES;  //change temp in regA alloc to yes in table
            }
        }
    }
      
    //ifregA holds neither operand1 or operand2...then load operand 2 into regA    
    if(regA != operand2 && regA != operand1){
            regA = operand2;
            objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand2 << setw(5) << " "<< '\n'; // load operand2
    }
      
    //ifregA holds operand2 then subtract operand1 in RAMM code
    if(regA == operand2){
        objectFile << left << setw(6) << " " << setw(4) << "ISB" << setw(4) << operand1 << setw(5) << " "<< operand2 << " = " << operand1 << endl;
    // else operand1 must be in regA so subtract operand2 in RAMM code
    }else{
        objectFile << left << setw(6) << " " << setw(4) << "ISB" << setw(4) << operand2 << setw(5) << " "<< operand2 << " = " << operand1 << endl;
    }
      
    //get myLabel forjump
    myLabel = GetMyJump();
      
    objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << myLabel << setw(5) << " " << '\n'; // jump to myLabel
       
    //condition is false so now we want to load in false
    //no boolean constant false found previously...Insert one and then use it
    if(numFalse == -1)
    {
        Insert("FALS",BOOLEAN,CONSTANT,"FALSE",YES,1);
        regA = "FALS";
        objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << setw(5) << " " << '\n'; // LDA FALS
    }
    else
    {
        objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT false
    }
      
    //RAMM Code to jump past code forcondtion "true"
    objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << myLabel << "+1" << setw(3) << " " << '\n'; // jump to line after myLabel
      
    //condition is false so now we want to load in true
    //ifno boolean constant true found previously...Insert one and LDA it 
    if(numTrue == -1)
    {
        Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
        regA = "TRUE";
        objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n'; // LDA TRUE
        
    }
    //else use first boolean constant true in symabolTable
    else
    {
        objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT true
    }
    
    if(operand1[0] == 'T' && operand1 != "TRUE"){
        FreeTemp();
    }
    if(operand2[0] == 'T' && operand2 != "TRUE"){
        FreeTemp();
    }
    
    regA = GetTemp();
    
    for(int i = 0; i < symbolTable.size(); ++i){
        if(symbolTable[i].internalName == regA)
            symbolTable[i].dataType = BOOLEAN; //change temp in regA alloc to yes in table
    }
    
    pushOperand(regA);
 
     //set back to default setting and return
    objectFile << right;
    return;
}

void EmitAssignCode(string operand1, string operand2){
    int opR = 0;
    int opL = 0;
       
    for(int i = 0; i < symbolTable.size(); ++i){
        if(symbolTable[i].internalName == operand1){ //Find the internal name of operand1
            opR = i;
        }
                  
        if(symbolTable[i].internalName == operand2){ //Find the internal name of operand2
            opL = i;
        }
    }
       
    if(symbolTable[opL].dataType != symbolTable[opR].dataType){ //Checi the data types against each other. They must match.
        Error("incompatible types");
    }
       
    if(symbolTable[opL].mode != VARIABLE){ //Checi to see ifoperand 2 has a mode of VARIABLE.
        Error("left-hand side of assignment must be VARIABLE");
    }
       
    if(operand1 == operand2){
        return;
    }
       
    if(regA!=operand1){
        objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand1 << setw(5) << " " << endl;
    }
    
    //regA now holds operand2 due to assignment
    regA = operand2;
    
    objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << operand2 << setw(5) << " " << operand2 << " := "<< operand1 << endl;
        
    
    
      
    if(operand1[0] == 'T' && operand1 != "TRUE"){
        FreeTemp();
    }
   
    //default
    objectFile << right;
    return;
 }

string GetTemp()
{
    string temp;
    currentTempNo++;
	
	ostringstream convert;
	convert << currentTempNo;
	
    temp = 'T' + convert.str();
    
    if(currentTempNo > maxTempNo){
        Insert(temp, UNKNOWN, VARIABLE, "", NO, 1);
        maxTempNo++;
    }
    return temp;
}

void EmitGreaterThanCode(string operand1, string operand2){
	string myJump;
	int numTrue = -1;
	int numFalse = -1;
	
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2){
			if(symbolTable[i].dataType != INTEGER){
				Error("> requires integers");
			}
		}
		if(symbolTable[i].internalName == "TRUE")
			numTrue = i;
		else if(symbolTable[i].internalName == "FALS")
			numFalse = i;
	}
	
	if(regA[0] == 'T' && regA != operand1 && regA != "TRUE"){ //check temp, opR, and true
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; // store regA
			
			for(int i = 0; i < symbolTable.size(); i++){
				if(symbolTable[i].internalName == regA){
					symbolTable[i].alloc=YES;  //change temp in regA alloc to yes in table
				}
			}
	}
	
	if(regA != operand1){
	  regA = operand1;
		objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand1 << setw(5) << " "<< '\n'; // load operand1
		
	}
		
	if(regA == operand1){
		objectFile << left << setw(6) << " " << setw(3) << "ISB " << setw(4) << operand2 << setw(5) << " "<<operand2<<" > "<<operand1<< endl;
	}
	
	// GetMyJump forjumps
	myJump = GetMyJump();
	
	objectFile << left << setw(6) << " " << setw(4) << "AMJ" << setw(4) << myJump << setw(5) << " " << '\n'; // jump to myJump
	
	if(numFalse == -1){
		Insert("FALS",BOOLEAN,CONSTANT,"FALSE",YES,1);
		regA = "FALS";
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << setw(5) << " " << '\n'; // LDA FALS
		
	}else{
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT false
	}  
	
	objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << myJump << "+1" << setw(3) << " " << '\n'; //jump to line after myJump
	
	if(numTrue == -1){
		Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
		regA = "TRUE";
		objectFile << left << setw(6) << myJump << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n'; // LDA TRUE
		
	}else{ //else use first boolean constant true in symbolTable
		objectFile << left << setw(6) << myJump << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT true
	}
	
	if(operand1[0] == 'T' && operand1 != "TRUE"){ //free trues
		FreeTemp();
	}if(operand2[0] == 'T' && operand2 != "TRUE"){
		FreeTemp();
	}
	
	regA = GetTemp();
	
	for(int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA)
				symbolTable[i].dataType = BOOLEAN; //temp in reg-A to correct BOOLEAN in table
	}
	pushOperand(regA);
	objectFile << right; //default setting
}

void EmitGreaterThanEqualCode(string operand1, string operand2){
	string myJump;
	int numTrue = -1;
	int numFalse = -1;
	
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == operand1 || symbolTable[i].internalName == operand2){
			if(symbolTable[i].dataType != INTEGER){
				Error(">= requires integers");
			}
		}
		if(symbolTable[i].internalName == "TRUE")
			numTrue = i;
		if(symbolTable[i].internalName == "FALS")
			numFalse = i;
	}
	
	if(regA[0] == 'T' && regA != operand2 && regA != "TRUE"){
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; //store regA
			
		for(int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA){
				symbolTable[i].alloc=YES;  //change temp in regA alloc to yes in table
			}
		}
	}
	
	if(regA != operand2){
	  regA = operand2;
		objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operand2 << setw(5) << " "<< '\n'; // load operand2
		
	}
		
	if(regA == operand2){
		objectFile << left << setw(6) << " " << setw(3) << "ISB " << setw(4) << operand1 << setw(5) << " "<<operand2<<" >= "<<operand1<< endl;
	}
	
	myJump = GetMyJump(); //GetMyJump forjumps
	
	objectFile << left << setw(6) << " " << setw(4) << "AMJ" << setw(4) << myJump << setw(5) << " " << '\n'; // jump to myJump
	
	if(numTrue == -1){
		Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
		regA = "TRUE";
		objectFile << left << setw(6) << " "   << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n'; // LDA TRUE
		
	}else{//else use first BOOLEAN CONSTANT
		objectFile << left << setw(6) <<" " << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT
	}
	
	objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << myJump << "+1" << setw(3) << " " << '\n'; // jump to line after myJump
	
	if(numFalse == -1){
		Insert("FALS",BOOLEAN,CONSTANT,"FALSE",YES,1);
		regA = "FALS";
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << setw(5) << " " << '\n'; // LDA FALS
		
	}else{
	objectFile << left << setw(6) << myJump<< setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT false
	}  
	
	if(operand1[0] == 'T' && operand1 != "TRUE"){
		FreeTemp();
	}if(operand2[0] == 'T' && operand2 != "TRUE"){
		FreeTemp();
	}
	
	regA = GetTemp();
	
	for(int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA)
				symbolTable[i].dataType=BOOLEAN; //change temp in regA alloc to yes in table
	}
	pushOperand(regA);
	objectFile << right; //default settings
}

void EmitProgramCode(){
    objectFile << left <<"STRT  NOP" << setw(10) << " " << setw(13) << symbolTable[0].value + " - Eual Smith, Brendan Murphey, Jacob Causer\n";
    objectFile << right;//set back to default setting
}
void EmitNotCode(string operand1){
	string myLabel;
	int opR = -1;
	int numTrue = -1;
	int numFalse = -1;
	
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].externalName == operand1){
			if( symbolTable[i].dataType != BOOLEAN )
				Error("BOOLEANS required");
			else
				opR = i;
		}if(symbolTable[i].internalName == "TRUE")
			numTrue = i;
		
		if(symbolTable[i].internalName == "FALS")
			numFalse = i;
	}if(opR == -1){
		if(operand1 != "true" && operand1 != "false")
			Error("not takes type BOOLEAN");
	}if(opR == -1 && operand1 == "false"){
		if(numFalse == -1){
			Insert("FALS",BOOLEAN,CONSTANT,"FALSE",YES,1);
			objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << setw(5) << " " << '\n'; // LDA FALS
		}else
			objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << setw(5) << " " << '\n'; // LDA symbolTable[opR].internalName
	}else if(opR == -1 && operand1 == "true"){
		if(numTrue == -1){
			Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
			objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " "<< '\n'; // LDA TRUE
		}else
			objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA symbolTable[opR].internalName
	}else{
		if(regA != symbolTable[opR].internalName)
			objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[opR].internalName << setw(5) << " " << '\n'; // LDA symbolTable[opR].internalName
	}
	myLabel = GetMyJump();
	
	objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << myLabel << setw(5) << " " << "not " + operand1 + '\n'; // jump ifoperand1 is false   
	for(uint i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == "TRUE")
			numTrue = i;
		else if(symbolTable[i].internalName == "FALS")
			numFalse = i;
	}
	if(numFalse == -1){
		Insert("FALS",BOOLEAN,CONSTANT,"FALSE",YES,1);
		regA = "FALS";
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << setw(5) << " " << '\n'; // LDA FALS
		
	}else{
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT false
	}
	
	objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << myLabel << "+1" << setw(3) << " " << '\n';  // jump to line after myLabel

	if(numTrue == -1){
		Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
		regA = "TRUE";
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n'; // LDA TRUE
		
	}else{
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT true
	}
	
	if(operand1[0] == 'T' && operand1 != "TRUE"){
		FreeTemp();
	}
	regA = GetTemp();
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == regA)
			symbolTable[i].dataType=BOOLEAN;
	}
	
	pushOperand(regA); 
	objectFile << right;
}

void EmitModuloCode(string opR, string opL){
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == opR || symbolTable[i].internalName == opL){
			if(symbolTable[i].dataType != INTEGER){
				Error("integers required");
			}
		}      
	}
	
	if(regA[0] == 'T' && regA != opL && regA != "TRUE"){
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; // store regA
		for(int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA){
				symbolTable[i].alloc=YES;  //change temp in regA alloc to yes in table
			}
		}
	}
	
	if(regA != opL){
	  regA = opL;
		objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << opL << setw(5) << " " << '\n'; // load opL
		
	}if(regA == opL){
		objectFile << left << setw(6) << " " << setw(3) << "IDV " << setw(4) << opR << setw(5) << " " <<opL<<" mod "<<opR<< endl;
	}
	
	if(opR[0] == 'T' && opR != "TRUE"){
		FreeTemp();
	}if(opL[0] == 'T' && opL != "TRUE"){
		FreeTemp();
	}
	
	regA = GetTemp();
	
	for(int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == regA)
			symbolTable[i].dataType=INTEGER;
		symbolTable[i].alloc = YES;
	}
	
	objectFile << left << setw(6) << " " << setw(3) << "STQ " << setw(4) << regA << setw(5) << " " << "store remainder\n"; // store regA
	objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << regA << setw(5) << " " << "load remainder\n"; // LDA regA

	pushOperand(regA);
	objectFile << right; //defaults
}
void EmitLessThanCode(string operandR, string operandL){
	string myLabel;
	int numTrue = -1;
	int numFalse = -1;
	
	for (int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == operandR || symbolTable[i].internalName == operandL){
			if(symbolTable[i].dataType != INTEGER){
				Error("< requires integers");
			}
		}
		if(symbolTable[i].internalName == "TRUE")
			numTrue = i;
		if(symbolTable[i].internalName == "FALS")
			numFalse = i;
	}
	if(regA[0] == 'T' && regA != operandL && regA != "TRUE"){
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; // store regA
		for (int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA){
				symbolTable[i].alloc = YES;
			}
		}
	}
	
	if(regA != operandL){
	  regA = operandL;
		objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operandL << '\n';
		
	}if(regA == operandL)
		objectFile << left << setw(6) << " " << setw(3) << "ISB " << setw(4) << operandR << setw(5) << " "<<operandL<<" < "<<operandR<< endl;
	
	myLabel = GetMyJump(); //GetMyJump for jumps
	objectFile << left << setw(6) << " " << setw(4) << "AMJ" << setw(4) << myLabel << '\n';
	
	if(numFalse == -1){
		Insert("FALS",BOOLEAN,CONSTANT,"FALSE",YES,1);
		regA = "FALS";
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << '\n';
		
	}else{
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << '\n';
	}  
	
	objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << myLabel << "+1" << '\n';
	
	if(numTrue == -1){
		Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
		regA = "TRUE";
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << "TRUE" << '\n';
		
	}else{
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << '\n';
	}
	
	if(operandR[0] == 'T' && operandR != "TRUE"){
		FreeTemp();}
	if(operandL[0] == 'T' && operandL != "TRUE"){
		FreeTemp();
	}
	
	regA = GetTemp();
	
	for (int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA)
				symbolTable[i].dataType = BOOLEAN;
	}
	
	pushOperand(regA);
	objectFile << right; //defaults
}
  
void EmitLessThanEqualCode(string operandR, string operandL){
	string myLabel;
	int numTrue = -1;
	int numFalse = -1;
	
	for (int i = 0; i < symbolTable.size(); i++){
		if(symbolTable[i].internalName == operandR || symbolTable[i].internalName == operandL){
			if(symbolTable[i].dataType != INTEGER)
				Error(">= requires integers");
		}
		if(symbolTable[i].internalName == "TRUE")
			numTrue = i; 
		if(symbolTable[i].internalName == "FALS")
			numFalse = i;
	}
	
	if(regA[0] == 'T' && regA != operandL && regA != "TRUE"){
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; // store regA
		for (int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA){
				symbolTable[i].alloc = YES;
			}
		}
	}
	
	if(regA != operandL){
    regA = operandL;
		objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operandL << setw(5) << " " << '\n'; // load operandL
		
	}
		
	if(regA == operandL)
		objectFile << left << setw(6) << " " << setw(3) << "ISB " << setw(4) << operandR << setw(5) << " "<<operandL<<" <= "<<operandR<< endl;
	
	myLabel = GetMyJump(); //GetMyJump for jumps
	objectFile << left << setw(6) << " " << setw(4) << "AMJ" << setw(4) << myLabel << setw(5) << " " << '\n';  // jump to myLabel
	objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << myLabel << setw(5) << " " << '\n'; // jump to myLabel
	
	if(numFalse == -1){
		Insert("FALS",BOOLEAN,CONSTANT,"FALSE",YES,1);
		regA = "FALS";
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << "FALS" << setw(5) << " " << '\n'; // LDA FALS
		
	}else{
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << symbolTable[numFalse].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT false
	}  
	
	objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << myLabel << "+1" << setw(3) << " " << '\n'; // jump to line after myLabel
	
	if(numTrue == -1){
		Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
		regA = "TRUE";
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n'; //LDA TRUE
		
	}else{
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n'; // LDA BOOLEAN CONSTANT true
	}
	
	if(operandR[0] == 'T' && operandR != "TRUE")
		FreeTemp();
	if(operandL[0] == 'T' && operandL != "TRUE")
		FreeTemp();
	
	regA = GetTemp();
	for (int i = 0; i < symbolTable.size(); i++){
			if(symbolTable[i].internalName == regA)
				symbolTable[i].dataType = BOOLEAN;
	}
	
	pushOperand(regA);
	objectFile << right; //defaults
}

void EmitNotEqualCode(string operandR, string operandL){
	int opR = -1;
	int opL = -1;
	int numTrue = -1;
	string myLabel;
	
	for (int i = 0; i < symbolTable.size(); i++){
		if (symbolTable[i].internalName == operandR)
			opR = i;
		
		if (symbolTable[i].internalName == operandL)
			opL = i;
			
		if (symbolTable[i].internalName == "TRUE")
			numTrue = i;
	}
	if (opR == -1 || opL == -1)
		Error("undefined symbol in equality statement");
	if (symbolTable[opR].dataType != symbolTable[opL].dataType)
		Error("Incompatible types for equality");
	
	if (regA[0] == 'T' && regA != operandR && regA != operandL && regA != "TRUE"){
		objectFile << left << setw(6) << " " << setw(3) << "STA " << setw(4) << regA << setw(5) << " " << "deassign regA\n"; 
	
		for (int i = 0; i < symbolTable.size(); i++){
			if (symbolTable[i].internalName == regA){
				symbolTable[i].alloc = YES;
			}
		}
	}
	 
	if (regA != operandL && regA != operandR){
      regA = operandL;
			objectFile << left << setw(6) << " " << setw(3) << "LDA " << setw(4) << operandL << setw(5) << " "<< '\n'; // load operandL
			
	}
	
	if (regA == operandL){
		objectFile << left << setw(6) << " " << setw(4) << "ISB" << setw(4) << operandR << setw(5) << " "<<operandL<<" <> "<<operandR<< endl;
	}else{
		objectFile << left << setw(6) << " " << setw(4) << "ISB" << setw(4) << operandL << setw(5) << " "<<operandL<<" <> "<<operandR<< endl;
	}
	
	myLabel = GetMyJump();
	objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << myLabel << "+1" << setw(3) << " " << '\n';
	
	if(numTrue == -1){
		Insert("TRUE",BOOLEAN,CONSTANT,"TRUE",YES,1);
		regA = "TRUE";
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << "TRUE" << setw(5) << " " << '\n';
		
	}else{
		objectFile << left << setw(6) << myLabel << setw(4) << "LDA" << setw(4) << symbolTable[numTrue].internalName << setw(5) << " " << '\n';
	}
	
	if (operandR[0] == 'T' && operandR != "TRUE"){
		FreeTemp();
	}if (operandL[0] == 'T' && operandL != "TRUE"){
		FreeTemp();
	}
	regA = GetTemp();
	
	for (int i = 0; i < symbolTable.size(); i++){
		if (symbolTable[i].internalName == regA)
			symbolTable[i].dataType = BOOLEAN;
	}
	pushOperand(regA);
	objectFile << right; //defaults
}

void FreeTemp()
{
	currentTempNo--;
    if(currentTempNo < -1)
        Error("compiler error, currentTempNo should be >= -1");
}  
//**********************************************************************Stage 2 - EJ************************************************************************************
void IF_STMT(){
	int index = -1;
	if(token !="if")
		Error("keyword \"if\" expected"); 
	
	NextToken();
	
	if (!isEXPRESS())
		Error("Invalid expression: not, true, false, (, +, -, non-key ID, or integer expected");
	else
		EXPRESS();
	
	for (int i = 0; i < symbolTable.size(); ++i)
	{
		if (symbolTable[i].internalName == Operand.top())
			index =  i;   
	}
	
	if (symbolTable[index].dataType != BOOLEAN)
		Error("if predicate must be BOOLEAN");  
	
	if (token != "then")
		Error("then expected");
	
	Code("then", popOperand());
	NextToken();
	
	if (token == "read" || isEXEC())
		EXEC_STMT();   
	else
		Error("non-keyword identifier, \"read\", \"write\", \"if\", \"while\", \"repeat\", \";\", or \"begin\" expected");
		
	if (token == "else" || token == "end" || token == "until" || isEXEC())
		ELSE_PT();
	else
		Error("invalid ELSE_PT");
}
  
void ELSE_PT(){
	if(token == "else")
	{
		Code("else", popOperand());
		NextToken();
		if (token == "read" || isEXEC())
		{
			EXEC_STMT();
			Code("post_if", popOperand());
		}
		else
			Error("invalid EXEC_STMT after else");
	}
	else if (token == "end" || token == "until" || isEXEC())
		Code("post_if", popOperand());
	else
		Error("invalid ELSE_PT");
  }
  
  
void WHILE_STMT(){
	int index = -1;
	
	if(token !="while"){
		Error(" keyword \"while\" expected"); 
	}
	else
		Code("while");
	
	NextToken();
	if (!isEXPRESS() )
		Error("Invalid expression: not, true, false, (, +, -, non-key ID, or integer expected");
	else
		EXPRESS();
	for (int i = 0; i < symbolTable.size(); ++i)
	{
		if (symbolTable[i].internalName == Operand.top())
			index =  i;
	}
	if (symbolTable[index].dataType != BOOLEAN)
		Error("while predicate must be BOOLEAN");
	if (token != "do")
		Error("invalid do for while");
	else
		Code("do", popOperand());
	NextToken();
	if (token == "read" || isEXEC() || token == "end")
	{
		EXEC_STMT();
		Code("post_while", popOperand(), popOperand());
	}  
	else
		Error("invalid EXEC_STMT after else");
}
  
  
void REPEAT_STMT(){
	int index = -1;
	
	if(token !="repeat")
		Error("keyword \"repeat\" expected"); 
		
	else
		Code("repeat");
	
	NextToken();
	
	if (token == "read" || isEXEC() || token == "end")
		EXEC_STMTS();
	
	if (token != "until")
		Error("expected until");
	
	NextToken();
	
	if (!isEXPRESS() )
		Error("Invalid expression: not, true, false, (, +, -, non-key ID, or integer expected");
	else
	{
		EXPRESS();
		for (int i = 0; i < symbolTable.size(); ++i)
		{
			if (symbolTable[i].internalName == Operand.top())
				index = i;
		}
		if (symbolTable[index].dataType != BOOLEAN)
			Error("until predicate must be BOOLEAN"); 
		Code("until", popOperand(), popOperand());
	}
	if (token != ";")
		Error("semicolon expected");
	else
		NextToken();
}
  
void NULL_STMT(){
	if (token != ";")
		Error("invalid null statement");
	else
		NextToken();
}
  
void EmitThenCode(string opR){
	
	string tempLabel = GetMyJump();
	if (regA != opR)
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << opR << setw(5) << " " << '\n'; // "LDA " << opR + '\n';
		
	objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << tempLabel << setw(5) << " " << "jump if condition is false\n";
	pushOperand(tempLabel);
	
	if (opR[0] == 'T' && opR != "TRUE"){
		FreeTemp();
	}
	regA = "";
}
  
void EmitElseCode(string opR){
	
	string tempLabel = GetMyJump();
	objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << tempLabel << setw(5) << " " << "jump to end if\n";
	objectFile << left << setw(6) << opR << setw(4) << "NOP" << setw(4) << " " << setw(5) << " " << "else\n";
	pushOperand(tempLabel);
	
	regA = "";
}
  
void EmitPostIfCode(string opR){
	objectFile << left << setw(6) << opR << setw(4) << "NOP" << setw(4) << " " << setw(5) << " " << "end if\n";
	regA = "";
}
  
void EmitWhileCode(){
	string tempLabel = GetMyJump();
	objectFile << left << setw(6) << tempLabel << setw(4) << "NOP" << setw(4) << " " << setw(5) << " " << "while\n";
	pushOperand(tempLabel);
	regA = "";
}
  
void EmitDoCode(string opR){
	string tempLabel = GetMyJump();
	
	if (regA != opR)
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << opR << setw(5) << " " << '\n'; // LDA opR
	
	objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << tempLabel << setw(5) << " " << "do\n"; //jump if condition is false
	
	pushOperand(tempLabel);
	
	if (opR[0] == 'T' && opR != "TRUE"){
		FreeTemp();
	}
	
	regA = "";
}
  
void EmitPostWhileCode(string opR, string opL){
	objectFile << left << setw(6) << " " << setw(4) << "UNJ" << setw(4) << opL << setw(5) << " " << "end while\n";
	objectFile << left << setw(6) << opR << setw(4) << "NOP" << setw(4) << " " << setw(5) << " " << '\n';
	
	regA = "";
}
  
void EmitRepeatCode(){
	string tempLabel = GetMyJump();
	objectFile << left << setw(6) << tempLabel << setw(4) << "NOP" << setw(4) << " " << setw(5) << " " << "repeat\n";
	pushOperand(tempLabel);
	regA = "";
}
  
void EmitUntilCode(string opR, string opL){
	
	if (regA != opR)
		objectFile << left << setw(6) << " " << setw(4) << "LDA" << setw(4) << opR << setw(5) << " " << '\n'; //"LDA " << opR + '\n';
	
	objectFile << left << setw(6) << " " << setw(4) << "AZJ" << setw(4) << opL << setw(5) << " " << "until\n"; // "jump if condition is false\n";
	
	if (opR[0] == 'T' && opR != "TRUE"){
		FreeTemp();
	}
	regA = "";
}

bool isEXEC(){
	return (token == "write" || token == "if" || token == "while" || token == "repeat" || token == ";" || token == "begin" || NonKeyID());
}

bool isEXPRESS(){
	return (token == "not" || isBoolean() || token == "(" || token == "+" || token == "-" || isInteger() || NonKeyID());
}
