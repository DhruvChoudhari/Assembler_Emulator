// Name: Dhruv Choudhari
// ID : 2101CS25
#include<bits/stdc++.h>        
using namespace std;
int Memory[1 << 24];    //It is the main memory of code
vector<string> code;    // This stores the code
int A, B, PC, SP, idx, cont, exec;
array<int, 2> change;
map<int,string>mnemonic;
void ldc(int value){                        //Functions to perform as per Mnemonic
    // Load accumulator with the value specified 
    B = A;
    A = value;
}
void adc(int value){
    A += value;
}
void ldl(int offset){
    B = A;
    A = Memory[SP + offset];
    change = {SP + offset, 0};
    exec = 1;
}
void stl(int offset){
    change = {Memory[SP + offset], A};
    Memory[SP + offset] = A;
    exec = 2;
    A = B;
}
void ldnl(int offset){
    A = Memory[A + offset];
    change = {SP + offset, 0};
    exec = 1;
}
void stnl(int offset){
    change = {Memory[A + offset], B};
    Memory[A + offset] = B;
    exec = 2;
}
void add(int off){
    A += B;
}
void sub(int off){
    A = B - A;
}
void shl(int off){
    A = B << A;
}
void shr(int off){ 
    A = B >> A;
}
void adj(int value){
    SP = SP + value;
}
void a2sp(int off){
    SP = A;
    A = B;
}
void sp2a(int off){
    B = A;
    A = SP;
}
void call(int offset){
    B = A;
    A = PC;
    PC += offset;
}
void ret(int offset){
    PC = A;
    A = B;
}
void brz(int offset){
    if(A == 0)
        PC = PC + offset;
}
void brlz(int offset){
    if(A < 0)
        PC = PC + offset;
}
void br(int offset){ 
    PC = PC + offset;
}
void (*call_func[])(int) = {ldc, adc, ldl, stl, ldnl, stnl, add, sub, shl, shr, adj, a2sp, sp2a, call, ret, brz, brlz, br}; 

string dec_hex(unsigned int num){
// This function converts decimal to hex
    if (num == 0)return "00000000";
    string s = "";
    while (num) {
        int temp = num % 16;

        if (temp <= 9) s += (48 + temp);
        else s += (87 + temp);

        num = num / 16;
    }
    reverse(s.begin(), s.end());
    while((int)s.size() < 8) s = '0'+s;
    return s;
}
void inputMacCode(){
// Input function
    cout << "Opening file machineCode.o" << endl;
    ifstream file ("machineCode.o", ios::in | ios::binary);
    unsigned int read;
    int pos = 0;
    // cout<<s<<endl;
    while(file.read((char*)&read,sizeof(int))) { 
        Memory[pos++] = read;
        code.push_back(dec_hex(read));
    }
}
void showMem(){
// Prints the memory
    for(int i = 0; i < (int) code.size(); i+=4){
        //it shows location
        cout << dec_hex(i) << " ";
        //showing byt0 ,byt1 , byt2 , byt3 thus min i+4 
        for(int j = i; j < min(i + 4, (int) code.size()); j++){
            cout << dec_hex(Memory[j]) << " ";
        }
        cout << endl;
    }
}
void showReg(){
// Showa the register
    cout << "A : " << dec_hex(A) << "  " << "B : " << dec_hex(B) << "  "  << "SP : " << dec_hex(SP) << "  " << "PC : " << dec_hex(PC + 1) << "  " << mnemonic[PC] << endl;  
}
void readPrint(){
    cout << "Reading memory[" <<dec_hex(PC) << "], has value: " << dec_hex(change[0]) << endl;
}
void writePrint(){
    cout << "Writing memory[" <<dec_hex(PC) << "], from " << dec_hex(change[0]) << " to " << dec_hex(change[1]) << endl;
}
void inset(){
    cout<<"OpMachineCode Mnemonic Operand : "<<'\n';
    cout<<"0      ldc      value "<< endl;
    cout<<"1      adc      value "<< endl;
    cout<<"2      ldl      offset "<< endl;
    cout<<"3      stl      offset "<< endl;
    cout<<"4      ldnl     offset "<< endl;
    cout<<"5      stnl     offset "<< endl;
    cout<<"6      add            "<< endl;
    cout<<"7      sub            "<< endl;
    cout<<"9      shr            "<< endl;
    cout<<"10     adj      value "<< endl;
    cout<<"11     a2sp           "<< endl;
    cout<<"12     sp2a           "<< endl;
    cout<<"13     call     offset"<< endl;
    cout<<"14     return         "<< endl;
    cout<<"15     brz      offset"<< endl;
    cout<<"16     brlz     offset"<< endl;
    cout<<"17     br       offset"<< endl;
    cout<<"18     HALT           "<< endl;
}
bool runCode(int oper){
// This runs the code or perform operations
    int iter = 0;
     int times = (1 << 25);
    while(times-- and PC < code.size()){
        cont++;
        if(PC >= code.size() or cont > (int)3e7){
            cout << " Error " << endl;
            return false;
        }
        string cur = code[PC];
        int op = stoi(cur.substr(6, 2), 0, 16);
        if(op == 18){
            cout << "HALT found" << endl;
            cout << cont << " statements were executed total" << endl;
            return true;
        }
        int val = stoi(cur.substr(0, 6), 0, 16);
        if(val >= (1 << 23)){
            val -= (1 << 24);
        }
        if(times == 0)
            showReg();

        // cout<<val<<endl<<endl;
        (call_func[op])(val);

        if(oper == 1 and exec == 1){
            readPrint();
            exec = 0;
        }
        else if(oper == 2 and exec == 2){
            writePrint();
            exec = 0;
        }
        PC++;
        idx++;
    }
    return true;
}

void runEmu(){
    while(true){

        string s;
        cin >> s;
        if(s == "0"){
            exit(0);
        }
        else if(s == "-dump"){
            showMem();
        }
        else if(s == "-reg"){
            showReg();
        }
        else if(s == "-run"){
            bool ret = runCode(0);
        }
        else if(s == "-isa"){
            inset();
        }
        else if(s == "-read"){
            bool ret = runCode(1);
        }
        else if(s == "-write"){
            bool ret = runCode(2);
        }
        else{
            cout << "Write codes defined above" << endl;
        }
    }
}

void fillmnem(){

}

int main() {   
    //filling mnemoniccs
    mnemonic[0] = "ldc"; mnemonic[1] = "adc"; mnemonic[2] = "ldl"; 
    mnemonic[3] = "stl"; mnemonic[4] = "ldnl"; mnemonic[5] = "stnl";
    mnemonic[6] = "add"; mnemonic[7] = "sub"; mnemonic[8] = "shl"; 
    mnemonic[9] = "shr"; mnemonic[10] = "adj"; mnemonic[11] = "a2sp"; 
    mnemonic[12] = "sp2a"; mnemonic[13] = "call"; mnemonic[14] = "return";
    mnemonic[15] =  "brz";mnemonic[16] =  "brlz"; mnemonic[17] =  "br";mnemonic[18] =  "HALT";
    
    inputMacCode();
    
    cout << "Welcome to Emulator" << endl;
    cout << "Following functions are implemented:" << endl;
    cout << "1. Memory Dump using instruction: -dump" << endl;
    cout << "2. Emulate code one line at time: using instruction: -t" << endl;
    cout << "3. Emulate till the end using -run" << endl;
    cout << "4. Show registers and SP values: -reg" << endl;
    cout << "5. Show instruction set using : -isa" << endl;
    cout << "6. Read operations using -read" << endl;
    cout << "7. Write operations using -write" << endl;

    runEmu();
    return 0;
}