// Name: Dhruv Choudhari
// ID : 2101CS25
#include<bits/stdc++.h>	    
using namespace std;
struct table{ 
// The table will be store ASM code in following manner
//  Label | Mnemonic | Operand | Operand Type (Hex, Oct etc) | If label is present in that line
	string label;
	string mnemonic;
	string operand;
	int oprType;
	int labelPresent;
};
vector<table> data;							//Data vector in Table form
map<string, pair<string, string>> OPCode;	//Stores mnemonic && OPCode
vector<pair<int, string>> errors;			//Stores all the errors
vector<string> content_NoSpaces;			//Contains the code line in clean form w/o spaces
map<string, int> labels;					//Stores the labels && their declaration line
vector<pair<int, string>> machineCode;		//Stores the machine code && extra info gor generating listCode
vector<int> programCounter;					//This vector maintains Program counter on every line
int haltPresent = 0;						// Check if HALT is present in the Code
string InputFile;							// Name of the file

int Digi(char c);							//These are helping functions && not related to assembler
int abc(char c);
int isDecimal(string s);
int chkOctal(char c);						
int isOctal(string s);						//These are helping functions && not related to assembler
int isHexadecimal(string s);				// Thus they are defined completely below main
int validName(string cur);

string OctalToDec(string num);
string HexToDec(string num);
string DecToHex(int num,int add);


string clean(string s, int line){
// This function remove unnecessary spaces
	
	for(int i = 0; i < 2; i++){
		reverse(s.begin(), s.end());
		//removing the front and end spaces of the string, thus loop of two only
		//reversing string twice gives the same string again, wtih no starting or ending spaces 
		while(s.back() == ' ' || s.back() == '\t'){
			s.pop_back();
		}
	}
	string temp;
	for(int i = 0; i < (int)s.size(); i++){
		if(s[i] == ';')// end of line , now comments are there which are not needed
			break;
		if(s[i] == ':'){
			temp += ":";
			if(i == (int)s.size() - 1 || s[i + 1] != ' ')
				temp += " ";
			continue;
		}
		if(s[i] != ' ' && s[i] != '\t'){
			temp += s[i];
			continue;
		}
		temp += " ";
		int j = i;
		// j captures the in between spaces. as j increments till normal work is found , if it is space then it continues
		// when normal word is found i is placed there thus skipping in between spaces 
		while(s[i] == s[j] && j < (int) s.size()) j++;
		i = j - 1;
	}
	// if any ending space is present or end line is present in temp then removing it
	while(!temp.empty() && (temp.back() == ' ' || temp.back() == '\t'))
		temp.pop_back();
	
	//calculating number of spaces in temp string
	int spac = 0;
	for(auto to: temp)
		spac += (to == ' ');

	//space should not be greater then 2 as 1 spaces are reqired between two words in instruction
	//maximum space will be utilised when instruction is of type A B C then 2 spaces will be taken other wise 1 or 0
	if(spac > 2)
		errors.push_back({line + 1, "Error at line: " + to_string(line+1) + " -- Type: Invalid syntax"});
	return temp;
}

void pushSETinstructions(vector<string> &temp, string token, string s, int j){
// Following mnemonic are used in implmenting SET mnemonic in assembler
	if(s.size() <= j + 5){
		return;
	}
	temp.push_back("adj 10000");			//stack pointer at random memory
	temp.push_back("stl -1");				//Here to load A
	temp.push_back("stl 0");				//Here B
	temp.push_back("ldc " + s.substr(j + 6, s.size() - (j + 6)));
	temp.push_back("ldc " + token.substr(0, j));
	temp.push_back("stnl 0");				//Load A, B
	temp.push_back("ldl 0");				
	temp.push_back("ldl -1");
	temp.push_back("adj -10000");			// Adjust stack ptr
}

int implementSET(){
// This function implements SET mnemonic given.
// Since SET is pseudo instruction, we implement it with other mnemonic
	vector<string> temp;
	for(int i = 0; i < (int) content_NoSpaces.size(); i++){
		string cur;
		int state = 0;
		for(int j = 0; j < (int) content_NoSpaces[i].size(); j++){
			cur += content_NoSpaces[i][j];
			if(content_NoSpaces[i][j] == ':'){
				cur.pop_back();
				if(content_NoSpaces[i].size() > j + 5 && content_NoSpaces[i].substr(j + 2, 3) == "SET"){
					state = 1;
					if(abs(labels[cur]) == i){
						labels[cur] = (int)temp.size() - 1;
						temp.push_back(content_NoSpaces[i].substr(0, j + 1) + " data " + content_NoSpaces[i].substr(j + 6, (int)content_NoSpaces[i].size() - (j + 6)));
					}
					else{
						pushSETinstructions(temp, cur, content_NoSpaces[i], j);
					}
					break;
				}
			}
		}
		if(!state && !content_NoSpaces[i].empty())
			temp.push_back(content_NoSpaces[i]);
	}
	content_NoSpaces = temp;
	return 0;
}

// Function to find all the errors && also each line in 
// {pctr, label, mnemonic, operand} form so as to use it while 
// calculating machine codes in pass2
int processLabel(){
// This function processes Lables && stores the labels map, with the postion they are declared
	for(int i = 0; i < (int) content_NoSpaces.size(); i++){
		string cur;
		for(int j = 0; j < (int) content_NoSpaces[i].size(); j++){
			if(content_NoSpaces[i][j] == ':'){
                //name is valid
                if(validName(cur)){
                    if(labels.count(cur)){
                        if(content_NoSpaces[i].size() > j + 4 && content_NoSpaces[i].substr(j + 2, 3) == "SET"){
                            continue;
                        }
                        else if(content_NoSpaces[i].size() > j + 5 && content_NoSpaces[i].substr(j + 2, 4) == "data" && labels[cur] < 0){
                            labels[cur] = i;
							continue;
                        }
                        else {
							errors.push_back({i + 1, "Error at line: " + to_string(i+1) + " -- Type: Multiple declaration of label: " + cur});
                        }
                    }
                    if(content_NoSpaces[i].size() > j + 4 && content_NoSpaces[i].substr(j + 2, 3) == "SET"){
						//means value is set
                        labels[cur] = -i;
						continue;
                    }
					else {
                    	labels[cur] = i;
						break;
					}
                }
                //name is not valid
				else{
                    int index = i+1;
					errors.push_back({index, "Error at line: " + to_string(index) + " -- Type: Invalid label name"});
					break;
				}
				break;
			}
			//stroing the string berfore encountring :
			else {
				cur += content_NoSpaces[i][j];
			}
		}
	}
	return 0;
}

int fillData(int i, string one, string two, string three, int type){
// Fills the data vector to reduce code size && make clean
	data[i].label = one;
	data[i].mnemonic = two;
	data[i].operand = three;
	data[i].oprType = type;
	return 0;
}

int calType(string s){
// Thname_is_valid functions return whether the operand is label/ Hex value/ Decimal value/ Octal value
	if(s.empty()) return 0;
	if(s[0] == '+' || s[0] == '-') s.erase(0,1);
	
	if(s.empty()) return -1;
	else if(isDecimal(s)) return 10;
	else if(isOctal(s)) return 8;
	else if(isHexadecimal(s)) return 16;
	else if(validName(s)) return 1;
	return -1;
}

int tableForm(){
// This functions process the data as:
//  Label | Mnemonic | Operand | Operand Type (Hex, Oct etc)
// Stores the data in aboe form in table vector
	int pc = 0;
	//ans[0] stores label name if exists, ans[1] stores instruction name if exists,
	// and ans[2] stores the num after instruction like 0 ->  label: ,1 -> ldc 2-> -5
	for(int i = 0; i < (int) content_NoSpaces.size(); i++){
		string ans[10] = {"", "", "", ""}, cur = "";
		int ptr = 1;
		
		for(int j = 0; j < (int) content_NoSpaces[i].size(); j++){

			if(content_NoSpaces[i][j] == ':'){
				ans[0] = cur;
				// cout<<" huh "<<cur<<endl;
				cur = "";
				j++;
				continue;
			}
			else if(content_NoSpaces[i][j] == ' '){
				ans[ptr++] = cur;
				// cout<<" huh2 "<<cur<<" "<<ptr<<endl;
				cur = "";
				// cout<<"hello"<<endl<<endl;
				continue;
			}
			cur += content_NoSpaces[i][j];
			// cout<<i<<" "<<j<<" "<<cur<<endl;
			if(j == (int)content_NoSpaces[i].size() - 1) ans[ptr++] = cur;
		}
		if(!ans[1].empty()){
			data[i].labelPresent = 1;
		}
		else{
			data[i].labelPresent = 0;
		}

		if(ans[1] == "HALT") haltPresent = 1;
		if(!ans[0].empty()) labels[ans[0]] = pc;
		programCounter[i] = pc;
		if(ptr == 1){
			fillData(i, ans[0], "", "", 0);
			continue;
		}
		pc++;	
		int ind = i+1;	
		if(!OPCode.count(ans[1])){
			// cout<<ind<<endl;
			errors.push_back({ind, "Error at line: " + to_string(ind) + " -- Type: Invalid Mnemonic"});
			continue;
		}

		int val =0;
		// seeing the operand of mnemonic, if it is nulll type val  =0, and so on
		if(OPCode[ans[1]].second == "value") val = 1;
		else if(OPCode[ans[1]].second == "offset") val = 2;

		// if it is value or offset then  it will have one mnemonic and one numericle only
		// thus pc  = 3 and val = 1 or 2. But if it is null type then it will have 1 mnemonic and two ints
		// like add 5,6 therefore pc = 4 and val = 0,
		// SO error will be shown when there is <2 ints are used in null type or more than 1 ints are used in value or offset type
		if(min(val, 1) != ptr - 2){
			// cout<<"here here"<<val<<endl;
			errors.push_back({ind, "Error at line: " + to_string(ind) + " -- Type: Invalid OPCode-Syntax combination"});
			continue;
		}
		// it encodes the line into label, mnemonic, operand, oprtype(dec,hex...)
		fillData(i, ans[0], ans[1], ans[2], calType(ans[2]));

		if(data[i].oprType == 1 && !labels.count(data[i].operand)){
			errors.push_back({ind, "Error at line: " + to_string(ind) + " -- Type: No such label / data variable"});
		}
		else if(data[i].oprType == -1){
			errors.push_back({ind, "Error at line: " + to_string(ind) + " -- Type: Invalid number"});
			
		}
	}
	return 0;
}

int makeDataSegment(){
// This function, seprates the code in DATA segment, so the instructions are executed properly
	vector<string> instr, dataseg;
	for(int i = 0; i < (int)content_NoSpaces.size(); i++){
		int state = 0;
		int chk = 0;
		for(int j = 0; j < content_NoSpaces[i].size(); j++){
			if(content_NoSpaces[i].back() == ':' && i + 1 < (int)content_NoSpaces.size() && content_NoSpaces[i + 1].substr(0, 4) == "data"){
				chk = 1;
				break;
			}
			if(content_NoSpaces[i].substr(j, 4) == "data" && j + 4 < content_NoSpaces[i].size()){
				chk = 1;
				break;
			}
		}

		if(chk == 1){
			dataseg.push_back(content_NoSpaces[i]);
			state = 1;
		}
		else instr.push_back(content_NoSpaces[i]);

	}
	// this is to insert dataseg vector at the end of the instrucion vector
	instr.insert(instr.end(), dataseg.begin(), dataseg.end());
	content_NoSpaces = instr;
	
	return 0;
}

int pass_one() {
// First pass of assembler which uses the functions declared above
	ifstream TestFile;
	cout<<"Write the file name (input.asm):"<<endl;
	cin>>InputFile;
	TestFile.open(InputFile);
	if(TestFile.fail()){
		cout << "Try Again, there is some error in file name" << endl;
		exit(0);
	}
	string s;
	// getline is used to get the whole line as string as normally spaces are not covered in string;
	while(getline(TestFile, s)) {
		string cur = clean(s, (int) content_NoSpaces.size());
		// cout<<s<<" ->  "<<cur<<endl;
		content_NoSpaces.push_back(cur);
	}
    // cout<<s<<endl;
	return 0;
	
}

// Stores the errors && warnings in the file: logFile.log
int seeErrors() {
	int res = 0;
	
	ofstream WriteInFile("logFile.log");
	WriteInFile << "Log code generated in: logFile.log"<< endl;

	if(errors.empty() == 0){
		sort(errors.begin(), errors.end());
		cout << (int)errors.size()<<" errors encountered! See logFile.log" << endl;
		for(auto to: errors){
			WriteInFile << to.second << endl;
		}
	}
	else if(errors.empty() == 1){
		res = 1;
		cout << "No errors found!" << endl;
		// HALT means it tells the emulator to stop, its a safer and right practice to stop the emulator by command rather thand doing it manually, thus warning is given
		if(haltPresent == 0){
			cout << "1 warning detected" << endl;
			WriteInFile << "Warning: HALT not present!" << endl;
		}
		WriteInFile << "Machine code generated in: machineCode.o" << endl;
		WriteInFile << "Listing code generated in: listCode.l" << endl;
	}
	WriteInFile.close();
	return res;
}

//  adding zero at start like 2 changes to 000002
string appZero(string s, int sz){
	while((int) s.size() < sz) s = '0'+s;
	return s;
}

// Second pass of assembler Its converts the code to machine code && also generates Listing code
int pass_two() {
	// cout<<data.size()<<endl;
	for(int i = 0; i < (int) data.size(); i++){
		// cout<<i<<" hey ";
		if(content_NoSpaces[i].empty()){
			continue;
		}
		//stores the loaction , in machine code format 
		string location = appZero(DecToHex(programCounter[i],24),6);

		// cout<< data[i].oprType<<" hey2 "<<data[i].mnemonic<<endl;
		string curMacCode = "        ";
		if(data[i].mnemonic == ""){
			machineCode.push_back({i, curMacCode});
			continue;
		} 
		// oprtype will be 1 only when label is there
		if(data[i].oprType == 1){
			int decForm = labels[data[i].operand];
			if(OPCode[data[i].mnemonic].second == "offset"){
				decForm -=  (programCounter[i] + 1);	
			}
			curMacCode = appZero(DecToHex(decForm,24),6) + OPCode[data[i].mnemonic].first;
			// cout<<" 1 "<<curMacCode<<endl;
			machineCode.push_back({i, curMacCode});
		}
		else if(data[i].oprType == 0){
			curMacCode = "000000" + OPCode[data[i].mnemonic].first;
			// cout<<" 0 "<<curMacCode<<endl;
			machineCode.push_back({i, curMacCode});
		}
		else {
			int sz = 6, add = 24;
			if(data[i].mnemonic == "data") sz = 8, add = 32;
			int decForm = stoi(data[i].operand, 0, data[i].oprType);
			curMacCode = appZero(DecToHex(decForm, add), sz) + OPCode[data[i].mnemonic].first;
			// cout<<" 2 "<<curMacCode<<endl;
			machineCode.push_back({i, curMacCode});
		}
	}
	return 0;
}

// used for writing in  listCode.txt
int WriteIn_listCode(){
	ofstream outList("listCode.l");
	for(auto to: machineCode){
		outList << appZero(DecToHex(programCounter[to.first],24),6) << " " << to.second << " " << content_NoSpaces[to.first] << endl;
	}
	outList.close();
	return 0;
}

//  used for writing in machineCode.txt
int WriteIn_machineCode(){
	ofstream outMachineCode;
	outMachineCode.open("machineCode.o",ios::binary | ios::out);
	for(auto to: machineCode){ 
		unsigned int x;
		int chk = 0;
		if(to.second.empty() || to.second == "        ") chk = 1;
		if(chk == 0){
			stringstream ss;
			ss << hex << to.second;
			ss >> x; // output it as a signed type
			static_cast<int>(x);
			outMachineCode.write((const char*)&x, sizeof(unsigned int));
		}
	}
	outMachineCode.close();
	return 0;
}

//Check Functions 
int abc(char c) {
	c = tolower(c);
	if(c>='a' && c<='z') return 1;
	return 0;
}

int Digi(char c) {
	if(c>='0' && c<='9') return 1;
	return 0;
}

// if string is in decimal format or not
int isDecimal(string s){
	int ok = 1;
	for(auto c : s){
		if(!Digi(c)){
			ok=0;
			break;
		} 
	} 
	return ok;
}

//function used for help in 'isOctal' function
int chkOctal(char c){
	int chk =0;
	if(c>='0' && c<='7')chk = 1;
	return chk;
}
// if string is in Octal format or not
int isOctal(string s){
    int ok = 1;
    if((int) s.size() < 2 || s[0] != '0') return 0;

    for(int i = 1; i < (int)s.size(); i++){
		if(!chkOctal(s[i])) {
			ok = 0;
			break;
		}
    }
    return ok ;
}

// if string is in hexadecimal format or not
int isHexadecimal(string s){
    int state = 1 ;
    if((int)s.size() < 3) return 0;
	if(s[0] != '0') state  =0;
	if(tolower(s[1]) != 'x' ) state  =0;

    for(int i = 2; i < (int) s.size(); i++){
        if(state = 0)break;
		int st =0;
        if(Digi(s[i])) st = 1;
		if( tolower(s[i]) >= 'a' && tolower(s[i]) <= 'f')st = 1;
		if(st = 0)state = 0;
    }
    return state;
}

// Check whether the label  name is valid. Ex: 1val is invalid, val1 is valid
//it is defined on the basis of criterias given in question paper
int validName(string cur){
	int chk =0;
	for(auto to: cur){
		if(abc(to))continue;
		else if(Digi(to))continue;
		else if(to == '_')continue;
		else return chk;
	}

	if(abc(cur[0]))	chk = 1;
	if(cur[0] == '_')	chk = 1;
	
	return chk;
}

//Some Functions used for conversion like dec to hex etc,,

string DecToHex(int number, int add = 24){
// This functions converts decimal number to Hexadecimal
	if(add == 32){
		unsigned int num = number;
		stringstream ss; 
		//converting number to hex and then after storing it in string ss
    	ss << hex << num; 
    	return ss.str();
	}
	//converting number to pos number then converting it to hex and then after storing it in string ss
	if(number < 0)
		number += (1 << add);
	stringstream ss; 
    ss << hex << number; 
    return ss.str();
}



int main() {	
	// Calling required functions

    //CHANGE OP CODE FORMAT

	pass_one();
    // op code is stored in hexadecimal format
	// Initializing the mnemonics
	OPCode["data"] = {"","value"}, OPCode["ldc"] = {"00","value"}, OPCode["adc"] = {"01","value"};
    OPCode["ldl"] = {"02","offset"}, OPCode["stl"] = {"03","offset"}, OPCode["ldnl"] = {"04","offset"};
    OPCode["stnl"] = {"05","offset"}, OPCode["add"] = {"06","none"}, OPCode["sub"] = {"07","none"};
    OPCode["shl"] = {"08","none"}, OPCode["shr"] = {"09","none"}, OPCode["adj"] = {"0A" ,"value"};
    OPCode["a2sp"] = {"0B" ,"none"}, OPCode["sp2a"] = {"0C" ,"none"}, OPCode["call"] = {"0D" ,"offset"};
    OPCode["return"] = {"0E" ,"none"}, OPCode["brz"] = {"0F" ,"offset"}, OPCode["brlz"] = {"10" ,"offset"};
    OPCode["br"] = {"11","offset"}, OPCode["HALT"] = {"12","none"}, OPCode["SET"] = {"" ,"value"};
	
	processLabel();					// Process labels in the code Ex: var1:
    if(errors.empty()){
		implementSET();				// Implementing SET mnemonic
    }
	data.resize((int) content_NoSpaces.size()); //Allocates memory for table
	programCounter.resize((int) content_NoSpaces.size());	//Allocates memory for programCounter array
	// cout<<content_NoSpaces.size()<<endl<<endl;
	//ERROR IN THIS FUNCTION, SIZE OF CONTENT IS REDUCING HERE
	makeDataSegment();						//Seprates the code in data segment && code
	// cout<<data[0].label<<" "<<data[1].label<<" "<<data[2].label<<endl<<endl;
	
	tableForm();						// Makes the code in table form as states in he problem
    if(seeErrors() == 1){
		pass_two();

		WriteIn_listCode();
		WriteIn_machineCode();
		
		cout << "Log code generated in: logFile.log" << endl;
		cout << "Machine code generated in: machineCode.o" << endl;
		cout << "Listing code generated in: listCode.l" << endl;
	}
	system("pause");
	return 0;
}

