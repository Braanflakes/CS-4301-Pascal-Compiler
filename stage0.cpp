#include <ctime>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream>

using namespace std;

const int MAX_SYMBOL_TABLE_SIZE = 256;
enum storeType {INTEGER, BOOLEAN, PROG_NAME};
enum allocation {YES, NO};
enum modes {VARIABLE, CONSTANT};
string keys[10] = {"program", "begin", "end", "var", "const",
					"integer", "boolean", "true", "false", "not"};
						
struct entry	// define symbol table entry format
{
	string internalName;
	string externalName;
	storeType dataType;
	modes mode;
	string value;
	allocation alloc;
	int units;
};
vector<entry> symbolTable;
ifstream sourceFile;
ofstream listingFile, objectFile;
string token;
char charac;
const char END_OF_FILE = '$';	// arbitrary choice

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

int main(int argc, char **argv)
{
	// this program is the stage0 compiler for Pascallite. It will accept
	// input from argv[1], generating a listing to argv[2], and object code to
	// argv[3]
	
	sourceFile.open(argv[1]);
	listingFile.open(argv[2]);
	objectFile.open(argv[3]);
	
	CreateListingHeader();
	Parser();
	CreateListingTrailer();
	
	PrintSymbolTable();
	
	return 0;
}

void CreateListingHeader()
{
	time_t now = time(NULL);
	listingFile << left << "STAGE0:\t" << "Brendan Murphey, EJ Smith, Jacob Causer\t" << ctime(&now) << "\n";
	listingFile << left << setw(15)<< "LINE NO:" << "SOURCE STATEMENT\n";	
	//line numbers and source statements should be aligned under the headings
}

void Parser()
{
	NextChar();
		// charac must be initialized to the first character of the source file
	if (NextToken().compare("program"))
		Error("keyword \"program\" expected");	// process error: keyword "program" expected;
		
	Prog();
		// parser implements the grammar rules, calling first rule
}

void CreateListingTrailer()
{
	listingFile << "COMPILATION TERMINATED\t" << "0 ERRORS ENCOUNTERED";
}

void PrintSymbolTable()
{
	// print symbol table to object file
	vector<entry>::iterator it;
	
	for (it = symbolTable.begin(); it < symbolTable.end(); ++it)
	{
		objectFile << left << setw(15) << it->externalName;
		objectFile << left << setw(4) << it->internalName;
		objectFile << setw(9) << it->dataType;
		objectFile << setw(8) << it->mode;
		objectFile << setw(15) << it->value;
		objectFile << setw(3) << it->alloc;
		objectFile << setw(3) << it->units;
		objectFile << "\n";
	}
}

void Prog() //token should be "program"
{
	if (token != "program")
	{
		Error("keyword 'program' expected");
	}
	ProgStmt();
	if (token == "const")
	{ 
		Consts();
	}
	if (token == "var")
	{
		Vars();
	}
	if (token != "begin")
	{
		Error("keyword 'begin' expected");
	}
	BeginEndStmt();
	if (token[0] != END_OF_FILE)
	{
		Error("no text may follow end");
	}
}

void ProgStmt() //token should be "program"
{
	string x;
	if (token != "program")
		//process error: keyword "program" expected
		Error("keyword 'program' expected");
	x = NextToken();
	// note: NonKeyID only checks TOKENS
	if (!NonKeyID())
		//process error: program name expected
		Error("program name expected");
	if (NextToken() != ";")
		//process error: semicolon expected
		Error("semicolon expected");
	NextToken();
	Insert(x,PROG_NAME,CONSTANT,x,NO,0);
}

void Consts() //token should be "const"
{
	if (token != "const")
		Error("keyword \"const\" expected");
	if (!NonKeyID())
		Error("non-keyword identifier must follow \"const\"");
	ConstStmts();
}

void Vars() //token should be "var"
{
	if (token != "var")
		//process error: keyword "var" expected
		Error("keyword 'var' expected");
	if (!NonKeyID())
		//process error: non-keyword identifier must follow "var"
		Error("non-keyword identifier  must follow 'var'");
	VarStmts();
}

void BeginEndStmt() //token should be "begin"
{
	if (token != "begin")
		Error("keyword 'begin' expected");
	if (NextToken() != "end")
		Error("keyword 'end' expected");
	if (NextToken() != ".")
		Error("period expected");
	NextToken();
}

void ConstStmts() //token should be NON_KEY_ID
{
	string x,y;
	unsigned int i = 0;
	bool isExternal = false;
	
	if (!NonKeyID())
		Error("non-keyword identifier expected");
	x = token;
	if (NextToken() != "=")
		Error("\"=\" expected"");
	y = NextToken();
	
	
	if (y != "+" || y != "-" || y != "not" || y != "true" || y != "false" || !NonKeyID()) || !isInteger() || !isBoolean())
		Error("token to right of \"=\" illegal");
		
	if (y == "+" || y == "-"){
		NextToken();
		//check if the token is an integer
		if(!isInteger())
			Error("integer expected after sign");
		y = y + token;
	}
	
	if (y == "not"){
		NextToken();
		
		//token == externalName?
		for(; i < symbolTable.size(); ++i)
		{
			if(token == symbolTable[i].externalName){
				isExternal = true;
				break;	
			}
		}
		
		if((isExternal && symbolTable[i].dataType != BOOLEAN)|| !isBoolean())
			Error("boolean expected after not")
			
		if(isExternal){
			if(symbolTable[i].value == "true"){
				y = "false";
			}else if(symbolTable[i].value == "false"){
				y = "true";
			}else
				Error("invalid value for BOOLEAN type");
		}else if(token == "true"){
			y = "false";
		}else{
			y = "true";
		}
	}
	
	if (NextToken() != ";")
		Error("semicolon expected");
	
	Insert(x,WhichType(y),CONSTANT,WhichValue(y),YES,1);
	
	NextToken();
	
	if (token != "begin" && token != "var" && !NonKeyID())
		Error("non-keyword identifier, \"begin\" or \"var\" expected");
	
	if (NonKeyID())
		ConstStmts();
}

void VarStmts()	//token should now be NON_KEY_ID
{
	string x, y;
	if (!NonKeyID(token))
		Error("non-keyword identifier expected");
	x = Ids();
	if (token != ":")
		Error("\":\" expected");
	NextToken();
	if (token != "integer" && token != "boolean")
		Error(" illegal type follows \":\"");
	y = token;
	if (NextToken() != ";")
		Error("semicolon expected");
	
	//-----DEBUGGING HERE-----
	if (y == "integer")
		Insert(x, INTEGER, VARIABLE, "", YES, 1);
	else if (y == "boolean")
		Insert(x, BOOLEAN, VARIABLE, "", YES, 1);
	else if (y == "program")
		Insert(x, PROG_NAME, VARIABLE, "", NO, 0);
	else
		Error("not a valid storeType");
	
	//if this doesn't work, put nexttoken before the if, and replace it with token
	if (NextToken() != "begin" || !NonKeyID())
		Error("non-keyword identifier or \"begin\" expected");
	if (!NonKeyID(token))
		VarStmts();
}

string Ids() //token should be NON_KEY_ID
{
	string temp,tempString;
	
	if (!NonKeyID(token))
		Error("non-keyword identifier expected");
	tempString = token;
	temp = token;
	if(NextToken() == ",")
	{
		NextToken();
		if (!NonKeyID())
			Error("non-keyword identifier expected");
		tempString = temp + "," + Ids();
	}
	return tempString;
}

 //create symbol table entry for each identifier in list of external names
 //Multiply inserted names are illegal
void Insert(string externalName, storeType inType, modes inMode, string inValue,
 allocation inAlloc, int inUnits)
{
    string name;
    string::iterator end = externalName.end();
    for (string::iterator a = externalName.begin(); a < externalName.end(); a++){
        name = ""; //initialize a new name
        while((*a != ',') && (a < end) ){ //fill in name appropriately
			name += *a;
			a++;
        }
		if(!name.empty()){
			if(name.length() > 15){ //if the name is too big (over 15), ignore characters past 15
				name = name.substr(0,15);
			}
			for (unsigned int i = 0; i < symbolTable.size(); i++){ //if the name is already there, error!
				if (symbolTable[i].externalName == name){
					Error("multiple name definition");
				}
			}   
			if(find(keys, keys + 10, name) != keys + 10){ //see if name matches keys
				Error("illegal use of keyword"); //if so, error
			}
			else{ //otherwise, set up the push_back
				entry my;
				if (name.length() > 15){
					my.externalName = name.substr(0,15);
				}else{
					my.externalName = name;
				}if(isupper(name[0])){
					my.internalName = name;
				}else{
					my.internalName = genInternalName(inType);
				}
				my.dataType = inType;
				my.mode = inMode;
				my.value = inValue;
				my.alloc = inAlloc;
				my.units = inUnits;
				symbolTable.push_back(my);
			}
		}
	}
}

storeType WhichType(string name){
	string::iterator it;
	vector<entry>::iterator vit;
	bool isInteger = true;
	
	if (name == "true" || name == "false")
		return BOOLEAN;
	if (isdigit(name[0]) || name[0] == '+' || name[0] == '-')
	{
		for (it = name.begin() + 1; it < name.end(); ++it)
		{
			if (!isdigit(*it))
				isInteger = false;
		}
		
		if (isInteger)
			return INTEGER;
	}
	
	for (vit = symbolTable.begin(); vit < symbolTable.end(); ++vit)
	{
		if (name == (*vit).externalName)
			return (*vit).dataType;
	}
	
	Error("reference to undefined constant");
}



string WhichValue(string name)
{
	string::iterator it;
	vector<entry>::iterator vit;
	bool isLiteral = true;

	if (name == "true" || name == "false")
		isLiteral = true;
		
	if (isdigit(name[0]))
	{
		for (it = name.begin() + 1; it < name.end(); ++it)
		{
			if (!isdigit(*it))
				isLiteral = false;
		}
		
		if (isLiteral)
			return name;
	}
	
	for (vit = symbolTable.begin(); vit < symbolTable.end(); ++vit)
	{
		if (name == (*vit).externalName)
			return (*vit).value;
	}
	
	Error("reference to undefined constant");
	
	
}

string NextToken(){ //returns the next token or end of file marker
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
			}
			if(charac == END_OF_FILE){
				Error("unexpected end of file");
			}else if(charac == '}'){
				Error("token can't start \'}\'");
			}
		}
		else if(charac == '}'){
			Error("\'}\' cannot begin token");
		}
		else if(isspace(charac)){
			NextChar();
		}
		else if (isSpecial(charac)){            
			token = charac;
            NextChar();
        }
		else if(charac == '_'){ //no leading _
			Error("\'_\' cannot start an identifier");
		}
		else if(isalpha(charac)){
			token = charac;
			charac = NextChar();
			if(charac == '_'){
				Error("\'_\' cannot start an identifier");
			}
			while(isLetter(charac)){ //search lowercase, nums, and spaces. if it's none of these, than npos is reached.
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
	}
	
	if (token[0] == '_'){ //no start _
		Error("\"_\" cannot start an identifier");
	}if (token[token.length() - 1] == '_'){ //no end _
		Error("\"_\" cannot end an identifier");
	}
	return token;
}

bool isSpecial(char chara){
	return (chara == ',' || charac == ';' || charac == '=' || charac == '+' || charac == '-' ||
		charac == '.' ||  charac =='(' || charac ==')'  || charac == '*');
}

bool isLetter(char chara){
	return (islower(chara) || isdigit(chara) || chara == '_');
}

char NextChar(){
	char myNext;
	sourceFile.get(myNext);
	static int lstLineNum = 0;
	static char prevCharac = ' ';
	
	//http://www.cplusplus.com/reference/ios/ios/good/
	if(!sourceFile.good()){
		charac = END_OF_FILE;
	}else{
		prevCharac = charac;
		charac = myNext;		
		if(lstLineNum == 0){
			lstLineNum++;
			listingFile << setw(5) << lstLineNum << '|';
		}else if (prevCharac == '\n'){
			lstLineNum++;
			listingFile << setw(5) << lstLineNum << '|';
		}
		listingFile << charac;
	}
	return charac;
}

void Error(string error){
	listingFile << "Error: " << error << "\n";
	CreateListingTrailer();
	sourceFile.close();
	listingFile.close();
	objectFile.close();
	terminate();
}

string genInternalName(storeType genType){
	static int boolCount = 0;
	static int intCount = 0;
	static bool progFound = false;
	ostringstream myOut;
	
	if(genType == INTEGER){
		myOut << intCount;
		myOut << "I" << boolCount;
		intCount++;
	}else if(genType == BOOLEAN){
		myOut << intCount;
		myOut << "B" << intCount;
		boolCount++;
	}else if (genType == PROG_NAME){
		if (progFound == false){
			myOut << "P0";
			progFound = true;
		}else{
			Error("only one program name allowed");
		}
	}
	return myOut.str();
}

//NonKeyID CHECKS TOKEN!
bool NonKeyID(){
	if(token[0] == '_') //check if first character is '_'
		Error("cannot begin with \"_\"");
	
	if (!isalpha(token[0]))	// check if the first char is alpha
		return false;
		
	for (int i = 0; i < (int)token.length(); i++)	// check token
	{
		if(isupper(token[i]))
			Error("upper case characters not allowed");
		if (!isalpha(s[i]) && s[i] != '_' && !isdigit(s[i]))
			return false;
	}
	if(find(keys, keys+10, token) != keywords+10) //make sure token isn't a key...
		return false;
	return true;
}

//if the token is true or false, it's a BOOLEAN
bool isBoolean(){
	return(token=="true" || token == "false");
}

//every character in token must be a digit to be an integer
bool isInteger(){
	for(int i = 0; i < (int)token.length(); i++){
		if(!isdigit(token[i]))
			return false;
	}
	return true;
}

