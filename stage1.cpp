// CS 4301 - Stage0 - EJ Smith, Brendan Murphey, Jacob Causer							this is the current build, copy and pasted from stage0

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
bool errorFound = false;
int lstLineNum = 0;
char prevCharac = ' ';
int boolCount = 0;
int intCount = 0;
int overflow = 0;
bool progFound = false;
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
	sourceFile.open(argv[1]); //accepts input from argv[1]
	listingFile.open(argv[2]); //generates a listing to argv[2]
	objectFile.open(argv[3]); //generates an object code to argv[3]
	
	CreateListingHeader(); //top
	Parser(); //main
	CreateListingTrailer(); //end
	PrintSymbolTable(); //object file
	return 0; //complete
}

// Creates the header for the listing file
void CreateListingHeader()
{
	time_t now = time(NULL); //time (as given)
	listingFile << left << "STAGE0:  " << "EJ Smith, Brendan Murphey, Jacob Causer\t" << ctime(&now) << "\n"; //names
	listingFile << setw(22) << "LINE NO." << "SOURCE STATEMENT\n\n";	//line numbers and source statements should be aligned under the headings
}

// Begins the program
void Parser()
{
	NextChar();// charac must be initialized to the first character of the source file
	if (NextToken().compare("program")) //if the next token
		Error("keyword \"program\" expected");	// process error: keyword "program" expected;
	Prog();// parser implements the grammar rules, calling first rule
}

// Creates the trailer for the listing file
void CreateListingTrailer()
{
	listingFile << "\nCOMPILATION TERMINATED\t" << setw(5) << ((errorFound)? '1' : '0') << " ERRORS" << " ENCOUNTERED\n";
}

// Prints the symbol table into the object file
void PrintSymbolTable()
{
	vector<entry>::iterator it;
	time_t now = time(NULL);
	
	objectFile << left << "STAGE0:  " << "EJ Smith, Brendan Murphey, Jacob Causer\t" << ctime(&now) << "\n";
	objectFile << "Symbol Table\n\n";
	
	for (it = symbolTable.begin(); it < symbolTable.end(); ++it)
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
void Prog() //token should be "program"
{
	if (token != "program"){
		Error("keyword 'program' expected");
	}
	ProgStmt();
	if (token == "const"){ 
		Consts();
	}
	if (token == "var"){
		Vars();
	}
	if (token != "begin"){
		Error("keyword 'begin' expected");
	}
	BeginEndStmt();
	if (token[0] != END_OF_FILE){
		Error("no text may follow end");
	}
}

// Production 2
void ProgStmt() //token should be "program"
{
	string x;
	if (token != "program")
		Error("keyword 'program' expected");
	x = NextToken();
	// note: NonKeyID only checks TOKENS
	if (!NonKeyID())
		Error("program name expected");
	if (NextToken() != ";")
		Error("semicolon expected");
	NextToken();
	Insert(x,PROG_NAME,CONSTANT,x,NO,0);
}

// Production 3
void Consts() //token should be "const"
{
	if (!islower(token))																				// Error from Stage0	*need to double check to make sure it is correct*
		Error("illegal symbol");																		// fixed 11/16/15
	if (token != "const")
		Error("keyword \"const\" expected");
	NextToken();
	if (!NonKeyID())
		Error("non-keyword identifier must follow \"const\"");
	ConstStmts();
}

// Production 4
void Vars() //token should be "var"
{
	if (token != "var")
		Error("keyword 'var' expected");
	NextToken();
	if (!NonKeyID())
		Error("non-keyword identifier  must follow 'var'");
	VarStmts();
}

// Production 5
void BeginEndStmt() //token should be "begin"
{
	if (token != "begin")
		Error("keyword 'begin' expected");
		// need to add EXEC_STMTS																		// need to add EXEC_STMTS
	if (NextToken() != "end")
		Error("keyword 'end' expected");
	if (NextToken() != ".")
		Error("period expected");
	NextToken();
}

// Production 6
void ConstStmts() //token should be NON_KEY_ID
{
	string x,y;
	int spot = -1;
		
	if (!NonKeyID())
		Error("non-keyword identifier expected"); 
	
	x = token;
	
	if (NextToken() != "="){
		Error("\"=\" expected");
	}
	
	y = NextToken();
	
	if (y != "+" && y != "-" && y !="not" && !NonKeyID() && !isBoolean() && !isInteger())
		Error("token to right of \"=\" illegal");
			
	if (y == "+" || y == "-"){
	NextToken();
	if(!isInteger())
		Error("integer expected after sign");
	y = y + token;
	}
	if (y == "not"){
		NextToken();
		for (uint i = 0; i < symbolTable.size(); i++){
			if (symbolTable[i].externalName == token)
				spot = i;
		}
		
		if (spot != -1){
			if (symbolTable[spot].dataType != BOOLEAN)
				Error("boolean expected after not");
		}

		else if(!isBoolean())
			Error("boolean expected after not");
			
		// token was external name?
		if (spot != -1){
			if (symbolTable[spot].value == "true")
				y = "false";
			else if (symbolTable[spot].value == "false")
				y = "true";
			else
				Error("invalid value for BOOLEAN type");
		}
		else if (token == "true"){ 
			y = "false";
		}else{
				y = "true";
		}
	}
	if (NextToken() != ";")
		Error("\":\" expected");
			
	Insert(x,WhichType(y),CONSTANT,WhichValue(y),YES,1);
		
	NextToken();
		
	if (token != "begin" && token != "var" && !NonKeyID())
		Error("non-keyword identifier, \"begin\", or \"var\" expected");

	if (NonKeyID())
		ConstStmts();
}

// Production 7
void VarStmts()	//token should now be NON_KEY_ID
{
	string x, y;
	if (!NonKeyID())
		Error("non-keyword identifier expected");
	x = Ids();
	if (token != ":")
		Error("\":\" expected");
	NextToken();
	if (token != "integer" && token != "boolean")
		Error("illegal type follows \":\"");
	y = token;
	if (NextToken() != ";")
		Error("semicolon expected");
	
	if (y == "integer")
		Insert(x, INTEGER, VARIABLE, "", YES, 1);
	else if (y == "boolean")
		Insert(x, BOOLEAN, VARIABLE, "", YES, 1);
	else if (y == "program")
		Insert(x, PROG_NAME, VARIABLE, "", NO, 0);
	else
		Error("not a valid storeType");
	
	NextToken();
	//if this doesn't work, put nexttoken before the if, and replace it with token
	if (token != "begin" && !NonKeyID())
		Error("non-keyword identifier or \"begin\" expected");
	if (NonKeyID())
		VarStmts();
}

// Production 8
string Ids() //token should be NON_KEY_ID
{
	string temp,tempString;
	
	if (!NonKeyID())
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

// Inserts a new entry into the symbol table
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
			++overflow;
			if (overflow > 256)
				Error("symbol table entries have exceeded the maximum allowed value");
		}
	}
}

// Determines which data type "name" has
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
	
	Error("reference to undefined constant1");
}


// Determines which value "name" has
string WhichValue(string name)
{
	string::iterator it;
	vector<entry>::iterator vit;
	bool isLiteral = true;

	if (name == "true" || name == "false")
		isLiteral = true;
	else if (isdigit(name[0]) || name[0] == '+' || name[0] == '-')
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
	
	if (name == "")
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
			}
			if(charac == END_OF_FILE){
				Error("unexpected end of file");
			}else if(charac == '}'){
				Error("token can't start \'}\'");
			}
		}else if(charac == '}'){
			Error("\'}\' cannot begin token");
		}else if(isspace(charac)){
			NextChar();
		}
		else if (isSpecial(charac)){            
			token = charac;
            NextChar();
        }
		else if(charac == ':'){
			token = charac;
			NextChar();
			if(charac == '='){
				token+=charac;
				NextChar();
			}
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
		}else if (prevCharac == '\n'){
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

// Returns true if token is a NonKeyID, and false otherwise
bool NonKeyID()
{
    if(token[0]=='_')
        Error("cannot begin with \"_\" ");
       
    if(isupper(token[0]))
		Error("upper case characters not allowed");
	
    if (!isalpha(token[0]))
		return false;
	
	
    // go through each char
    for (int x = 1; x < (int)token.length(); x++)
    {
        if(isupper(token[x]))
            Error("upper case characters not allowed");
        if ( !isalpha(token[x]) && !isdigit(token[x]) && token[x] != '_' )
            return false;
    }
    return(find(keys, keys+10, token) != keys+10)? false : true; //token isn't key ID
}

//is it a boolean type?
bool isBoolean(){
	return(token=="true" || token == "false");
}

//all chars must be a digit for it to be an integer type. is it so?
bool isInteger(){
	for(int i = 0; i < (int)token.length(); i++){
		if(!isdigit(token[i]))
			return false;
	}
	return true;
}

// function to emit the addition code
void EmitAdditionCode(string operand1, string operand2)
{
	// if type of either operand is not integer, process error : illegal type
	if (!isdigit(operand1) || !isdigit(operand2))
		Error("illegal type");
		
	// if A register holds a temp not operand1 nor operand2 then...
}
