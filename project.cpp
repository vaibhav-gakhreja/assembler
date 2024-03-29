#include <bits/stdc++.h>

using namespace std;

int pc,sp,a,b;//declaration of registers
long long pow2[33];
//pc starts from 0 as code segment start from zero
//initialize sp to correct value.

bool NaN[3],extraValue;//checks validity of number
map<string,int> label_addr,labelLineNumber;//stores the address of each label.
map<int,string> code;//stores the code corresponding to each pc.
map<int,pair<int,string>> original;//stores the code corresponding to each line number.
map<int,bool> isInstruction,isEmpty;//stores boolean , if the line is an instruction or if it is empty.
set<string> validLabels,usedLabels;//holds the names of all the valid labels.
int errorLineNumber=-1;
vector<string> binaryRes;

char get_char(int val){//this function is used to get the hex value for each number between 0 to 15.
	switch(val){
		case 0: return '0';break;
		case 1: return '1';break;
		case 2: return '2';break;
		case 3: return '3';break;
		case 4: return '4';break;
		case 5: return '5';break;
		case 6: return '6';break;
		case 7: return '7';break;
		case 8: return '8';break;
		case 9: return '9';break;
		case 10: return 'A';break;
		case 11: return 'B';break;
		case 12: return 'C';break;
		case 13: return 'D';break;
		case 14: return 'E';break;
		case 15: return 'F';break;
	}
}

string to_hex(long long val,int padding){//it gives you the hex string corresponding to the input int
	bool neg=0;
	if(val<0) neg=1;
	val=abs(val);
	if(neg){
		val=pow2[4*padding]-val;
	}
	string ret;
	int temp=padding-1;
	while(temp!=-1){
		int t=val%16;
		ret+=get_char(t);
		val/=16;
		temp--;
	}
	reverse(ret.begin(),ret.end());
	return ret;
}

string remove_semicolon(string x)//this function removes everything after semicolon from the instruction.
{
	string ret=x;
	for(int i=0;i<x.size();i++)
	{
		if(x[i]==';')
		{
			ret=x.substr(0,i);
			break;
		}
	}
	return ret;
}

string remove_spaces(string x)//this function trims spaces/tabs from before and after the instruction.
{
	int start=0,end=x.size()-1;
	while(start<=end&&(x[start]=='	'||x[start]==' ')) start++;
	while(end>=start&&(x[end]=='	'||x[end]==' '||x[end]=='\n')) end--;
	if(start>end) return "";//empty line
	return x.substr(start,end-start+1);
}

string capitalise(string x)//this function capitalises the input string.
{
	for(int i=0;i<x.size();i++)
	{
		if(x[i]<='z'&&x[i]>='a')
		{
			x[i]=x[i]-'a'+'A';
		}
	}
	return x;
}

int toNumber(string s)//this function converts the input string into a number,
{//and delivers an error if the string is not a valid number.
	int p=1,ret=0,mul=10,sign=0;
	if(s[0]=='-')
	{
		sign=-1;
		s=s.substr(1,s.size()-1);
	}
	if(s[0]=='+')
	{
		sign=1;
		s=s.substr(1,s.size()-1);
	}
	if(s.size()>=2&&s[0]=='0'&&s[1]=='X') {//hexadecimal number
		mul=16;
		s=s.substr(2,s.size()-2);
		for(int i=0;i<s.size();i++)
		{
			if(!((s[i]>='0'&&s[i]<='9')||(s[i]>='A'&&s[i]<='F')))
			{
				NaN[0]=1;
				if(s[i]==',')
				{
					extraValue=1;
				}
				return 0;
			}
		}
	}else if(s.size()>=1&&s[0]=='0') {//octal number
		mul=8;
		s=s.substr(1,s.size()-1);
		for(int i=0;i<s.size();i++)
		{
			if(!(s[i]>='0'&&s[i]<='7'))
			{
				NaN[1]=1;
				if(s[i]==',')
				{
					extraValue=1;
				}
				return 0;
			}
		}
	}else
	{
		for(int i=0;i<s.size();i++)
		{
			if(!(s[i]>='0'&&s[i]<='9'))
			{
				NaN[2]=1;
				if(s[i]==',')
				{
					extraValue=1;
				}
				return 0;
			}
		}
	}
	for(int i=s.size()-1;i>=0;--i)
	{
		if(s[i]>='0'&&s[i]<='9') ret+=(s[i]-'0')*p;
		if(s[i]>='A'&&s[i]<='F') ret+=(s[i]-'A'+10)*p;
		p*=mul;
	}
	if(sign==-1) ret=-ret;
	return ret;
}

string toString(int num)//this function converts a number into a string.
{
	string ret;
	while(num>0)
	{
		ret+=(char)('0'+num%10);
		num/=10;
	}
	reverse(ret.begin(),ret.end());
	return ret;
}

bool isValidLabelName(string name)//this function checks if the input string is a valid label name.
{
	if(name.empty()) return 0;
	if(!(name[0]>='A'&&name[0]<='Z')) return 0;
	return 1;
}

int main(int argc,char **argv)
{
	pow2[0]=1;
	for(int i=1;i<33;i++)
	{
		pow2[i]=(pow2[i-1]*2ll);
	}
	string fname=argv[1],objectFile,listingFile,errorFile;
	objectFile=fname.substr(0,fname.size()-4)+".o";
	listingFile=fname.substr(0,fname.size()-4)+".l";
	errorFile=fname.substr(0,fname.size()-4)+".log";
	FILE *fp1,*fp2,*fp3,*fp4;//set up file pointers.
	char temp1[300],temp2[300],temp3[300],temp4[300];
	for(int i=0;i<fname.size();i++)
	{
		temp1[i]=fname[i];
	}
	temp1[fname.size()]='\0';
	fp1=fopen(temp1,"r");
	for(int i=0;i<listingFile.size();i++)
	{
		temp3[i]=listingFile[i];
	}
	temp3[listingFile.size()]='\0';
	fp3=fopen(temp3,"w");
	for(int i=0;i<errorFile.size();i++)
	{
		temp4[i]=errorFile[i];
	}
	temp4[errorFile.size()]='\0';
	fp4=fopen(temp4,"w");
	char temp[300];//this temporary variable is used to write strings to file using fprintf
	char *ix;//this is used to take input
	string x;//this is used to store input(rather that char *), for easy handling
	int line_number=0;
	string addr;
	size_t len=0;
	bool firstPrint=0;
	vector<pair<string,int>> errors;//this will store the error message along with the line number as they occur.
	// first pass
	while(getline(&ix,&len,fp1)!=-1)//read a line into varibele ix
	{
		line_number++;
		x.clear();
		x=ix;
		original[line_number]={pc,x};//storing pc along with original code.
		x=remove_semicolon(x);
		x=remove_spaces(x);
		if(x.empty()){//this line only had a comment.
			isEmpty[line_number]=1;
			continue;
		}
		x=capitalise(x);
		// remove label
		bool label=0;
		string labelName;
		for(int i=0;i<x.size();i++)//we find labels here, and store their addresses.
		{
			if(x[i]==':')
			{
				label=1;
				labelName=remove_spaces(x.substr(0,i));
				x=x.substr(i+1,x.size()-i-1);
				x=remove_spaces(x);
				break;
			}
		}
		if(label)
		{
			if(!isValidLabelName(labelName))//we store an error message if the the label name is invalid.
			{
				string errorMsg="A bogus label name was found on line";
				errors.push_back({errorMsg,line_number});
				if(errorLineNumber==-1) errorLineNumber=line_number;
			}else
			{
				if(labelLineNumber.find(labelName)==labelLineNumber.end()) labelLineNumber[labelName]=line_number;
			}
			if(label_addr.find(labelName)!=label_addr.end()) {//if labelName already exists
				string errorMsg="A duplicate label was found on line";//we store error message for dupicate label.
				errors.push_back({errorMsg,line_number});
				if(errorLineNumber==-1) errorLineNumber=line_number;
			}
			validLabels.insert(labelName);//this set stores the name of all valid labels.
			label_addr[labelName]=(pc);//storing the address of pc.
			if(x.substr(0,3)=="SET")
			{
				string t1=x.substr(3,x.size()-3);
				t1=remove_spaces(t1);
				label_addr[labelName]=toNumber(t1);
			}
		}else if(x.substr(0,3)=="SET")
		{
			errors.push_back({"Missing label before SET mnemonic on line",line_number});
			if(errorLineNumber==-1) errorLineNumber=line_number;
		}
		if(x.empty()) continue;
		isInstruction[line_number]=1;
		//x now has only operation code and operand
		//note that label has been capitalised.
		//if the value(operand) has an alphabet it will be capitalised.
		code[pc]=x;
		pc++;
	}
	//closing the input file.
	fclose(fp1);
	//second pass
	for(auto &it:original)//we traverse each line of code.
	{
		bool noOpcode=0;
		pc=it.second.first;
		x=code[pc];
		int opcode=0,oprnd=0;
		if(x.substr(0,3)=="ADD")//one by one we check for the match of operation.
		{
			// a=b+a;
			opcode=6;
			if(x.size()!=3) {//if the first three chars match to add, but has some more chars after it as operand, then issue an error.
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
		}else if(x.substr(0,3)=="SUB")
		{
			// a=b-a;
			opcode=7;
			if(x.size()!=3) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
		}else if(x.substr(0,3)=="SHL")
		{
			// a=(b<<a);
			opcode=8;
			if(x.size()!=3) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
		}else if(x.substr(0,3)=="SHR")
		{
			// a=(b>>a);
			opcode=9;
			if(x.size()!=3) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
		}else if(x.substr(0,4)=="A2SP")
		{
			// sp=a;
			// a=b;
			opcode=11;
			if(x.size()!=4) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
		}else if(x.substr(0,4)=="SP2A")
		{
			// b=a;
			// a=sp;
			opcode=12;
			if(x.size()!=4) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
		}else if(x.substr(0,6)=="RETURN")
		{
			// pc=a;
			// a=b;
			opcode=14;
			if(x.size()!=6) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
		}else if(x.substr(0,4)=="HALT")
		{
			opcode=18;
			if(x.size()!=4) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
				if(errorLineNumber==-1) errorLineNumber=it.first;
			}
			//stop the emulator;
		}else
		{
			//these are instructions which have one soperand.
			string operation,operand;
			bool operandPresent=0;
			int ptr=0;
			for(int i=0;i<x.size();i++)
			{
				if(x[i]==' '||x[i]=='	')
				{
					operandPresent=1;
					operation=x.substr(0,i);
					break;
				}
				ptr++;
			}
			if(operandPresent==0)//if operand is not present then we need to issue an error
			{//either the mnemonic is bogus or we lack an operand.
				operation=x;
			}
			while(ptr<x.size()&&(x[ptr]==' '||x[ptr]=='	')) ptr++;
			bool label=0;
			//operand can be a number or a label.
			operand=x.substr(ptr,x.size()-ptr);//we extract the operation and operand.
			// correct it to handle hex,octal,decimal numbers, if error comes that means it is a label
			if((operand[0]>='0'&&operand[0]<='9')||operand[0]=='-'||operand[0]=='+')
			{
				//immediate value
				NaN[0]=0;
				NaN[1]=0;
				NaN[2]=0;
				extraValue=0;
				oprnd=toNumber(operand);//convert string to number.
				if(NaN[0]||NaN[1]||NaN[2]) {//if conversion issues an error, then report it.
					if(extraValue) {//if there are multiple values then issue this error.
						string temp="Only one numerical value was expected on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else {//if the number is wronly written then issue this error.
						if(NaN[0])
						{
							string temp="There is invalid hexadecimal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}else if(NaN[1])
						{
							string temp="There is invalid octal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}else
						{
							string temp="There is invalid decimal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}
					}
				}
			}else
			{
				//its a label
				label=1;
			}
			usedLabels.insert(operand);
			if(operation=="DATA")
			{
				NaN[0]=0;
				NaN[1]=0;
				NaN[2]=0;
				extraValue=0;
				if(operandPresent==0) {//if operand is not recieved , then issue error saying lack of operand.
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				noOpcode=1;
				if(NaN[0]||NaN[1]||NaN[2]) {
					if(extraValue) {
						string temp="Only one numerical value was expected on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else {
						if(NaN[0])
						{
							string temp="There is invalid hexadecimal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}else if(NaN[1])
						{
							string temp="There is invalid octal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}else
						{
							string temp="There is invalid decimal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}
					}
				}
			}else if(operation=="LDC")
			{
				opcode=0;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand];
					}
				}
			}else if(operation=="ADC")
			{
				opcode=1;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand];
					}
				}
			}else if(operation=="LDL")
			{
				opcode=2;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="STL")
			{
				opcode=3;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="LDNL")
			{
				opcode=4;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="STNL")
			{
				opcode=5;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="ADJ")
			{
				opcode=10;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand];
					}
				}
			}else if(operation=="CALL")
			{
				opcode=13;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="BRZ")
			{
				opcode=15;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="BRLZ")
			{
				opcode=16;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="BR")
			{
				opcode=17;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="SET")
			{
				NaN[0]=0;
				NaN[1]=0;
				NaN[2]=0;
				extraValue=0;
				noOpcode=1;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
					continue;
				}
				opcode=toNumber(operand);
				if(NaN[0]||NaN[1]||NaN[2]) {
					if(extraValue) {
						string temp="Only one numerical value was expected on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else {
						if(NaN[0])
						{
							string temp="There is invalid hexadecimal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}else if(NaN[1])
						{
							string temp="There is invalid octal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}else
						{
							string temp="There is invalid decimal value on line";
							errors.push_back({temp,it.first});
							if(errorLineNumber==-1) errorLineNumber=it.first;
						}
					}
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
						if(errorLineNumber==-1) errorLineNumber=it.first;
					}else{
						oprnd=label_addr[operand];
					}
				}
			}else
			{
				// invalid instruction
				if(isInstruction[it.first]) {//if the line had an instruction, and it does not match to any valid instruction,
					string temp="An unknown mnemonic was found on line";//then issue error saying invalid mnemonic.
					errors.push_back({temp,it.first});
					if(errorLineNumber==-1) errorLineNumber=it.first;
				}
			}
		}
		if(errorLineNumber!=-1&&errorLineNumber<it.first) continue;//we do not write anything to listing or object file if there is an error.
		// make operand(6 hex digits,24 bits) + opcode(2 hex digits,8 bits) as machine code. 
		// write pc in the listing file
		string tmp;
		tmp=to_hex(pc,4);
		for(int i=0;i<tmp.size();i++)
		{
			temp[i]=tmp[i];
		}
		temp[tmp.size()]='\0';
		//if the line is not empty(has label or mnemonic) then write pc into listing file.
		//note that blank lines or comments do not have their pc.
		if(isEmpty[it.first]==0) fprintf(fp3,"%s ",temp);
		if(noOpcode) {//if there is no opcode(data or set instructions)
			tmp=to_hex(oprnd,8);//32 bit operand
		}else{
			tmp=to_hex(oprnd,6);//24 bit operand
		}
		if(noOpcode==0)//appending opcode if it exists
		{
			string res=to_hex(opcode,2);//8 bit opcode
			tmp+=res;//appending 8 bit opcode, now tmp is 32 bit machine code.
		}
		for(int i=0;i<tmp.size();i++)
		{
			temp[i]=tmp[i];
		}
		temp[tmp.size()]='\0';
		if(isInstruction[it.first]==0)//write machine code only if the line has an instruction.
		{
			for(int i=0;i<tmp.size();i++)//if it is not instruction then we just print whitespaces.
			{
				temp[i]=' ';
			}
		}else
		{
			binaryRes.push_back(tmp);
		}
		fprintf(fp3,"%s ",temp);
		//now print the original code in the listing file.
		for(int i=0;i<it.second.second.size();i++)
		{
			temp[i]=it.second.second[i];
		}
		temp[it.second.second.size()]='\0';
		fprintf(fp3,"%s",temp);
	}
	//print the contents of object file if there is no error
	if(errors.empty())
	{
		for(int i=0;i<objectFile.size();i++)
		{
			temp2[i]=objectFile[i];
		}
		temp2[objectFile.size()]='\0';
		fp2=fopen(temp2,"w");
		for(auto &it:binaryRes)
		{
			if(firstPrint) fprintf(fp2,"\n");
			for(int i=0;i<it.size();i++)
			{
				temp[i]=it[i];
			}
			temp[it.size()]='\0';
			fprintf(fp2,"%s",temp);
			firstPrint=1;
		}
		fclose(fp2);
	}
	//now we sort the error messages as per their line numbers.
	vector<pair<int,string>> tempv;
	for(auto &it:errors)
	{
		tempv.push_back({it.second,it.first});
	}
	sort(tempv.begin(),tempv.end());
	errors.clear();
	for(auto &it:tempv)
	{
		errors.push_back({it.second,it.first});
	}
	for(auto &it:errors)//write the error messages into the log file.
	{
		string tmp;
		tmp="ERROR: "+it.first;
		tmp=tmp+" ";
		tmp=tmp+toString(it.second);
		for(int i=0;i<tmp.size();i++)
		{
			temp[i]=tmp[i];
		}
		temp[tmp.size()]='\0';
		fprintf(fp4,"%s\n",temp);
	}
	for(auto &it:labelLineNumber)
	{
		if(usedLabels.count(it.first)==0)
		{
			string tmp="WARNING: Unused label declared on line";
			tmp=tmp+" ";
			tmp=tmp+toString(it.second);
			for(int i=0;i<tmp.size();i++)
			{
				temp[i]=tmp[i];
			}
			temp[tmp.size()]='\0';
			fprintf(fp4,"%s\n",temp);
		}
	}
	fclose(fp3);
	fclose(fp4);
}