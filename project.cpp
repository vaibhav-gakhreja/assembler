#include <bits/stdc++.h>

using namespace std;
//change var names to intuitive names as per their function

int pc,sp,a,b;//declaration of registers
long long pow2[33];
//pc starts from 0 as code segment start from zero
//initialize sp to correct value.
//errors being handled :->
//1)duplicate labels must not exist : I am reporting the error and stopping exec just after first pass.
//2)

//clarify behaviour of data and set mnemonic , from ammar, abhay.

bool NaN,extraValue;//checks validity of number
map<string,int> label_addr;//stores the address of each label.
map<int,string> code;//stores the code corresponding to each pc.
map<int,pair<int,string>> original;//stores the code corresponding to each line number.
map<int,bool> isInstruction,isEmpty;//stores boolean , if the line is an instruction or if it is empty.
set<string> validLabels;//holds the names of all the valid labels.

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
	if(s.size()>=2&&s[0]=='0'&&s[1]=='x') {
		mul=16;
		s=s.substr(2,s.size()-2);
		for(int i=0;i<s.size();i++)
		{
			if(!((s[i]>='0'&&s[i]<='9')||(s[i]>='A'&&s[i]<='F')))
			{
				NaN=1;
				if(s[i]==',')
				{
					extraValue=1;
				}
				return 0;
			}
		}
	}else if(s.size()>=1&&s[0]=='0') {
		mul=8;
		s=s.substr(1,s.size()-1);
		for(int i=0;i<s.size();i++)
		{
			if(!(s[i]>='0'&&s[i]<='7'))
			{
				NaN=1;
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
				NaN=1;
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
		if(!(s[i]<='9'&&s[i]>='0')) continue; 
		ret+=(s[i]-'0')*p;
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
//.o ,symbolTable , warnings ,read pdf for more.
int main()
{
	pow2[0]=1;
	for(int i=1;i<33;i++)
	{
		pow2[i]=(pow2[i-1]*2ll);
	}
	FILE *fp1,*fp2,*sys,*fp3,*fp4;//set up file pointers.
	fp1=fopen("input.asm","r");
	fp2=fopen("output.o","w");
	sys=fopen("symbolTable.txt","w");
	fp3=fopen("listing.l","w");
	fp4=fopen("errors.log","w");
	char temp[300];//this temporary variable is used to write strings to file using fprintf
	char *ix;//this is used to take input
	string x;//this is used to store input(rather that char *), for easy handling
	int line_number=0;
	string addr;
	size_t len=0;
	vector<pair<string,int>> errors;
	// first pass
	while(getline(&ix,&len,fp1)!=-1)
	{
		line_number++;
		x.clear();
		x=ix;
		original[line_number]={pc,x};
		// remove semicolon, if not found, issue error message and exit;
		x=remove_semicolon(x);
		// remove unnecessary padding of spaces;
		x=remove_spaces(x);
		if(x.empty()){
			// cout<<"Syntax error : empty instruction, line number : "<<line_number<<'\n';
			// return 0;
			isEmpty[line_number]=1;
			continue;
		}
		// capitalise the instruction(just for uniformity and ease of check);
		x=capitalise(x);
		// remove label
		bool label=0;
		string labelName;
		for(int i=0;i<x.size();i++)
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
			if(!isValidLabelName(labelName))
			{
				string errorMsg="A bogus label name was found on line";
				errors.push_back({errorMsg,line_number});
			}
			addr=to_hex(pc,4);
			for(int i=0;i<addr.size();i++)
			{
				temp[i]=addr[i];
			}
			temp[addr.size()]='\0';
			fprintf(sys,"%s\n",temp);
			if(label_addr.find(labelName)!=label_addr.end()) {
				string errorMsg="A duplicate label was found on line";
				errors.push_back({errorMsg,line_number});
			}
			validLabels.insert(labelName);
			label_addr[labelName]=(pc);
		}
		if(x.empty()) continue;
		isInstruction[line_number]=1;
		//x now has only operation code and operand
		//note that label has been capitalised.
		//if the value(operand) has an alphabet it will be capitalised.
		code[pc]=x;
		pc++;
	}
	//second pass
	for(auto &it:original)
	{
		pc=it.second.first;
		x=code[pc];
		int opcode=0,oprnd=0;
		if(x.substr(0,3)=="ADD")
		{
			// a=b+a;
			opcode=6;
			if(x.size()!=3) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
		}else if(x.substr(0,3)=="SUB")
		{
			// a=b-a;
			opcode=7;
			if(x.size()!=3) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
		}else if(x.substr(0,3)=="SHL")
		{
			// a=(b<<a);
			opcode=8;
			if(x.size()!=3) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
		}else if(x.substr(0,3)=="SHR")
		{
			// a=(b>>a);
			opcode=9;
			if(x.size()!=3) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
		}else if(x.substr(0,4)=="A2SP")
		{
			// sp=a;
			// a=b;
			opcode=11;
			if(x.size()!=4) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
		}else if(x.substr(0,4)=="SP2A")
		{
			// b=a;
			// a=sp;
			opcode=12;
			if(x.size()!=4) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
		}else if(x.substr(0,6)=="RETURN")
		{
			// pc=a;
			// a=b;
			opcode=14;
			if(x.size()!=6) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
		}else if(x.substr(0,4)=="HALT")
		{
			opcode=18;
			if(x.size()!=4) {
				string temp="A numerical value was not expected on line";
				errors.push_back({temp,it.first});
			}
			//stop the emulator;
		}else
		{
			// cout<<it.first<<' '<<x<<'\n';
			//these are instructions which have operand.
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
			if(operandPresent==0)
			{
				operation=x;
			}
			while(ptr<x.size()&&(x[ptr]==' '||x[ptr]=='	')) ptr++;
			bool label=0;
			operand=x.substr(ptr,x.size()-ptr);
			if((operand[0]>='0'&&operand[0]<='9')||operand[0]=='-'||operand[0]=='+')
			{
				//immediate value
				NaN=0;
				extraValue=0;
				oprnd=toNumber(operand);
				if(NaN) {
					if(extraValue) {
						string temp="Only one numerical value was expected on line";
						errors.push_back({temp,it.first});
					}else {
						string temp="There is a non-numerical value on line";
						errors.push_back({temp,it.first});
					}
				}
			}else
			{
				//its a label
				label=1;
			}
			//operand is not always a number.
			// if(it.first==7) cout<<oprnd<<' '<<operand<<'\n';
			if(operation=="DATA")
			{
				NaN=0;
				extraValue=0;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					continue;
				}
				opcode=toNumber(operand);
				if(NaN) {
					if(extraValue) {
						string temp="Only one numerical value was expected on line";
						errors.push_back({temp,it.first});
					}else {
						string temp="There is a non-numerical value on line";
						errors.push_back({temp,it.first});
					}
				}
				// if(label)
				// {

				// }
			}else if(operation=="LDC")
			{
				opcode=0;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
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
					continue;
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
					}else{
						oprnd=label_addr[operand]-pc-1;
					}
				}
			}else if(operation=="SET")
			{
				NaN=0;
				extraValue=0;
				if(operandPresent==0) {
					string temp="A numerical value was expected on line";
					errors.push_back({temp,it.first});
					continue;
				}
				opcode=toNumber(operand);
				if(NaN) {
					if(extraValue) {
						string temp="Only one numerical value was expected on line";
						errors.push_back({temp,it.first});
					}else {
						string temp="There is a non-numerical value on line";
						errors.push_back({temp,it.first});
					}
				}
				if(label)
				{
					if(isInstruction[it.first]==1&&validLabels.count(operand)==0) {
						string temp="A non existant label was found on line";
						errors.push_back({temp,it.first});
					}else{
						oprnd=label_addr[operand];
					}
				}
			}else
			{
				// invalid instruction
				if(isInstruction[it.first]) {
					string temp="An unknown mnemonic was found on line";
					errors.push_back({temp,it.first});
				}
			}
		}
		// make operand + opcode as machine code. 
		// write to fp2 pc+[operand][opcode].
		string tmp;
		tmp=to_hex(pc,4);
		for(int i=0;i<tmp.size();i++)
		{
			temp[i]=tmp[i];
		}
		temp[tmp.size()]='\0';
		if(isEmpty[it.first]==0) fprintf(fp3,"%s ",temp);
		tmp=to_hex(oprnd,6);
		// cout<<oprnd<<"\n";
		string res=to_hex(opcode,2);
		// cout<<operation<<" "<<opcode<<" "<<operand<<"\n";
		// cout<<tmp<<' '<<res<<'\n';
		tmp+=res;
		for(int i=0;i<tmp.size();i++)
		{
			temp[i]=tmp[i];
		}
		temp[tmp.size()]='\0';
		if(isInstruction[it.first]==0)
		{
			for(int i=0;i<tmp.size();i++)
			{
				temp[i]=' ';
			}
		}
		fprintf(fp3,"%s ",temp);
		for(int i=0;i<it.second.second.size();i++)
		{
			temp[i]=it.second.second[i];
		}
		temp[it.second.second.size()]='\0';
		fprintf(fp3,"%s",temp);
	}
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
	for(auto &it:errors)
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
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
	fclose(sys);
}