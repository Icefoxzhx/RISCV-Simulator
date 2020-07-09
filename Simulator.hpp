#ifndef CODE_SIMULATOR_HPP
#define CODE_SIMULATOR_HPP
#include <cstdio>
#include <iostream>
#include <cstring>
using namespace std;
typedef unsigned char uchar;
typedef unsigned int uint;
enum IS_names{
	NOP,//0
	ADD,SUB,SLL,SLT,SLTU,XOR,SRL,SRA,OR,AND, //R-type 1-10
	JALR,LB,LH,LW,LBU,LHU,ADDI,SLTI,SLTIU,XORI,ORI,ANDI,SLLI,SRLI,SRAI, //I-type
	SB,SH,SW, //S-type
	BEQ,BNE,BLT,BGE,BLTU,BGEU, //B-type
	LUI,AUIPC, //U-type
	JAL //J-type
};
char NAME[][10]={
		"NOP",//0
		"ADD","SUB","SLL","SLT","SLTU","XOR","SRL","SRA","OR","AND", //R-type 1-10
		"JALR","LB","LH","LW","LBU","LHU","ADDI","SLTI","SLTIU","XORI","ORI","ANDI","SLLI","SRLI","SRAI", //I-type
		"SB","SH","SW", //S-type
		"BEQ","BNE","BLT","BGE","BLTU","BGEU",//B-type
		"LUI","AUIPC", //U-type
		"JAL" //J-type
};
class Simulator{
public:
	uchar mem[8388608];
	uint X[32],pc;
	struct _IF_ID{
		uint IR;
		uint npc;
		_IF_ID(){
			IR=0;npc=0;
		}
		void clear(){
			IR=0;npc=0;
		}
		bool empty(){
			return IR==0;
		}
	}IF_ID;
	struct _ID_EX{
		uint rs1,rs2,rd,imm,npc;
		IS_names name;
		_ID_EX(){
			rs1=rs2=rd=imm=npc=0;
			name=NOP;
		}
		void clear(){
			rs1=rs2=rd=imm=npc=0;
			name=NOP;
		}
		bool empty(){
			return name==NOP;
		}
	}ID_EX;
	struct _EX_MEM{
		uint res,rd,npc;
		int tim;
		IS_names name;
		_EX_MEM(){
			res=rd=npc=tim=0;
			name=NOP;
		}
		void clear(){
			res=rd=npc=tim=0;
			name=NOP;
		}
		bool empty(){
			return name==NOP;
		}
	}EX_MEM;
	struct _MEM_WB{
		uint res,rd;
		IS_names name;
		_MEM_WB(){
			res=rd=0;
			name=NOP;
		}
		void clear(){
			res=rd=0;
			name=NOP;
		}
		bool empty(){
			return name==NOP;
		}
	}MEM_WB;
	void print(){
		printf("%04x:\t%08x\t\t%s rs1: x%u rs2: x%u rd: x%u imm:%u\n",pc,IF_ID.IR,NAME[ID_EX.name],ID_EX.rs1,ID_EX.rs2,ID_EX.rd,ID_EX.imm);
	}
	void workI(){
		ID_EX.rd=(IF_ID.IR>>7u)&31u;
		ID_EX.rs1=(IF_ID.IR>>15u)&31u;
		ID_EX.imm=((IF_ID.IR>>31u)&1u)?0xfffff000:0;
		ID_EX.imm|=(IF_ID.IR>>20u)&4095u;
	}
	void workB(){
		ID_EX.rs1=(IF_ID.IR>>15u)&31u;
		ID_EX.rs2=(IF_ID.IR>>20u)&31u;
		ID_EX.imm=((IF_ID.IR>>31u)&1u)?0xfffff000:0;
		ID_EX.imm|=((IF_ID.IR>>7u)&1u)<<11u;
		ID_EX.imm|=((IF_ID.IR>>25u)&63u)<<5u;
		ID_EX.imm|=((IF_ID.IR>>8u)&15u)<<1u;
	}
	void workS(){
		ID_EX.rs1=(IF_ID.IR>>15u)&31u;
		ID_EX.rs2=(IF_ID.IR>>20u)&31u;
		ID_EX.imm=((IF_ID.IR>>31u)&1u)?0xfffff000:0;
		ID_EX.imm|=(IF_ID.IR>>7u)&31u;
		ID_EX.imm|=((IF_ID.IR>>25u)&127u)<<5u;
	}
	bool IF(){
		memcpy(&IF_ID.IR,mem+pc,4);
		if(IF_ID.IR==0x0ff00513) return 0;
		IF_ID.npc=pc+4;
		return 1;
	}
	void ID(){
		ID_EX.npc=IF_ID.npc;
		uint op=IF_ID.IR&127u;
		switch(op){
			case 0x37://LUI
				ID_EX.name=LUI;
				ID_EX.rd=(IF_ID.IR>>7u)&31u;
				ID_EX.imm=IF_ID.IR&0xfffff000;
				break;
			case 0x17://AUIPC
				ID_EX.name=AUIPC;
				ID_EX.rd=(IF_ID.IR>>7u)&31u;
				ID_EX.imm=IF_ID.IR&0xfffff000;
				break;
			case 0x6f://JAL
				ID_EX.name=JAL;
				ID_EX.rd=(IF_ID.IR>>7u)&31u;
				ID_EX.imm=((IF_ID.IR>>31u)&1)?0xfff00000:0;
				ID_EX.imm|=IF_ID.IR&0x000ff000;
				ID_EX.imm|=((IF_ID.IR>>20u)&1u)<<11u;
				ID_EX.imm|=((IF_ID.IR>>21u)&1023u)<<1u;
				break;
			case 0x67://JALR
				ID_EX.name=JALR;
				workI();
				break;
			case 0x63://Bxx
				workB();
				switch((IF_ID.IR>>12u)&7u){
					case 0:ID_EX.name=BEQ;break;
					case 1:ID_EX.name=BNE;break;
					case 4:ID_EX.name=BLT;break;
					case 5:ID_EX.name=BGE;break;
					case 6:ID_EX.name=BLTU;break;
					case 7:ID_EX.name=BGEU;break;
				}
				break;
			case 0x03://Lx(x)
				workI();
				switch((IF_ID.IR>>12u)&7u){
					case 0:ID_EX.name=LB;break;
					case 1:ID_EX.name=LH;break;
					case 2:ID_EX.name=LW;break;
					case 4:ID_EX.name=LBU;break;
					case 5:ID_EX.name=LHU;break;
				}
				break;
			case 0x23://Sx
				workS();
				switch((IF_ID.IR>>12u)&7u){
					case 0:ID_EX.name=SB;break;
					case 1:ID_EX.name=SH;break;
					case 2:ID_EX.name=SW;break;
				}
				break;
			case 0x13://ADDI...
				workI();
				switch((IF_ID.IR>>12u)&7u){
					case 0:ID_EX.name=ADDI;break;
					case 2:ID_EX.name=SLTI;break;
					case 3:ID_EX.name=SLTIU;break;
					case 4:ID_EX.name=XORI;break;
					case 6:ID_EX.name=ORI;break;
					case 7:ID_EX.name=ANDI;break;
					case 1:ID_EX.name=SLLI;break;
					case 5:ID_EX.name=((IF_ID.IR>>30u)&1u)?SRAI:SRLI;break;
				}
				break;
			case 0x33://ADD...
				ID_EX.rs1=(IF_ID.IR>>15u)&31u;
				ID_EX.rs2=(IF_ID.IR>>20u)&31u;
				ID_EX.rd=(IF_ID.IR>>7u)&31u;
				switch((IF_ID.IR>>12u)&7u){
					case 0:ID_EX.name=((IF_ID.IR>>30u)&1u)?SUB:ADD;break;
					case 1:ID_EX.name=SLL;break;
					case 2:ID_EX.name=SLT;break;
					case 3:ID_EX.name=SLTU;break;
					case 4:ID_EX.name=XOR;break;
					case 5:ID_EX.name=((IF_ID.IR>>30u)&1u)?SRA:SRL;break;
					case 6:ID_EX.name=OR;break;
					case 7:ID_EX.name=AND;break;
				}
				break;
		}
//		print();
		ID_EX.rs1=X[ID_EX.rs1];
		ID_EX.rs2=X[ID_EX.rs2];
		IF_ID.clear();
	}
	void EX(){
		EX_MEM.name=ID_EX.name;
		EX_MEM.rd=ID_EX.rd;
		EX_MEM.npc=ID_EX.npc;
		EX_MEM.tim=0;
		switch(EX_MEM.name){
			case LB:case LH:case LW:case LBU:case LHU:
				EX_MEM.res=ID_EX.rs1+ID_EX.imm;break;

			case SB:case SH:case SW:
				EX_MEM.res=ID_EX.rs1+ID_EX.imm;
				EX_MEM.rd=ID_EX.rs2;break;

			case ADD:EX_MEM.res=ID_EX.rs1+ID_EX.rs2;break;
			case SUB:EX_MEM.res=ID_EX.rs1-ID_EX.rs2;break;
			case XOR:EX_MEM.res=ID_EX.rs1^ID_EX.rs2;break;
			case OR:EX_MEM.res=ID_EX.rs1|ID_EX.rs2;break;
			case AND:EX_MEM.res=ID_EX.rs1&ID_EX.rs2;break;

			case ADDI:EX_MEM.res=ID_EX.rs1+ID_EX.imm;break;
			case XORI:EX_MEM.res=ID_EX.rs1^ID_EX.imm;break;
			case ORI:EX_MEM.res=ID_EX.rs1|ID_EX.imm;break;
			case ANDI:EX_MEM.res=ID_EX.rs1&ID_EX.imm;break;

			case SLLI:EX_MEM.res=ID_EX.rs1<<(ID_EX.imm&31u);break;
			case SRLI:EX_MEM.res=ID_EX.rs1>>(ID_EX.imm&31u);break;
			case SRAI:EX_MEM.res=((int)ID_EX.rs1)>>(ID_EX.imm&31u);break;

			case SLL:EX_MEM.res=ID_EX.rs1<<(ID_EX.rs2&31u);break;
			case SRL:EX_MEM.res=ID_EX.rs1>>(ID_EX.rs2&31u);break;
			case SRA:EX_MEM.res=((int)ID_EX.rs1)>>(ID_EX.rs2&31u);break;

			case SLT:EX_MEM.res=(int)ID_EX.rs1<(int)ID_EX.rs2;break;
			case SLTU:EX_MEM.res=ID_EX.rs1<ID_EX.rs2;break;

			case SLTI:EX_MEM.res=(int)ID_EX.rs1<(int)ID_EX.imm;break;
			case SLTIU:EX_MEM.res=ID_EX.rs1<ID_EX.imm;break;

			case LUI:EX_MEM.res=ID_EX.imm;break;
			case AUIPC:EX_MEM.res=ID_EX.imm+pc;break;

			case BEQ:if(ID_EX.rs1==ID_EX.rs2) EX_MEM.npc=pc+ID_EX.imm;break;
			case BNE:if(ID_EX.rs1!=ID_EX.rs2) EX_MEM.npc=pc+ID_EX.imm;break;
			case BLT:if((int)ID_EX.rs1<(int)ID_EX.rs2) EX_MEM.npc=pc+ID_EX.imm;break;
			case BGE:if((int)ID_EX.rs1>=(int)ID_EX.rs2) EX_MEM.npc=pc+ID_EX.imm;break;
			case BLTU:if(ID_EX.rs1<ID_EX.rs2) EX_MEM.npc=pc+ID_EX.imm;break;
			case BGEU:if(ID_EX.rs1>=ID_EX.rs2) EX_MEM.npc=pc+ID_EX.imm;break;

			case JAL:EX_MEM.res=EX_MEM.npc;EX_MEM.npc=pc+ID_EX.imm;break;
			case JALR:EX_MEM.res=EX_MEM.npc;EX_MEM.npc=(ID_EX.rs1+ID_EX.imm)&~1;break;//...

			default:break;
		}
		ID_EX.clear();
	}
	void MEM(){
		MEM_WB.name=EX_MEM.name;
		MEM_WB.rd=EX_MEM.rd;
		MEM_WB.res=EX_MEM.res;
		pc=EX_MEM.npc;
		if(!((MEM_WB.name>=LB&&MEM_WB.name<=LHU)||(MEM_WB.name>=SB&&MEM_WB.name<=SW))){
			EX_MEM.clear();
			return;
		}
		/*if(EX_MEM.tim!=3){
			++EX_MEM.tim;
			return;
		}*/
		switch(MEM_WB.name){
			case LB:
				signed char res_LB;
				memcpy(&res_LB,mem+EX_MEM.res,1);
				MEM_WB.res=res_LB;//check
				break;
			case LH:
				signed short res_LH;
				memcpy(&res_LH,mem+EX_MEM.res,2);
				MEM_WB.res=res_LH;//check
				break;
			case LW:
				memcpy(&MEM_WB.res,mem+EX_MEM.res,4);
				break;
			case LBU:
				uchar res_LBU;
				memcpy(&res_LBU,mem+EX_MEM.res,1);
				MEM_WB.res=res_LBU;//check
				break;
			case LHU:
				unsigned short res_LHU;
				memcpy(&res_LHU,mem+EX_MEM.res,2);
				MEM_WB.res=res_LHU;//check
				break;

			case SB:
				char res_SB;
				res_SB = EX_MEM.rd;
				memcpy(mem+EX_MEM.res,&res_SB,1);
				break;
			case SH:
				short res_SH;
				res_SH=EX_MEM.rd;
				memcpy(mem+EX_MEM.res,&res_SH,2);
				break;
			case SW:
				memcpy(mem+EX_MEM.res,&EX_MEM.rd,4);
				break;

			default:break;
		}
		EX_MEM.clear();
	}
	void WB(){
		switch(MEM_WB.name){
			case BEQ:case BNE:case BLT:case BGE:case BLTU:case BGEU:
			case SB:case SH:case SW:break;
			default:
				if(MEM_WB.rd) X[MEM_WB.rd]=MEM_WB.res;break;
		}
		MEM_WB.clear();
	}
	Simulator(){
		memset(mem,0,sizeof(mem));
		memset(X,0,sizeof(X));
		pc=0;
	}
	void init(){
		char buf[10];
		int mem0=0;
		while(~scanf("%s",buf)){
			if(buf[0]=='@'){
				sscanf(buf+1,"%X",&mem0);
				continue;
			}
			sscanf(buf,"%X",&mem[mem0]);
			scanf("%X",&mem[mem0+1]);
			scanf("%X",&mem[mem0+2]);
			scanf("%X",&mem[mem0+3]);
			//swap(mem[mem0],mem[mem0+3]);
			//swap(mem[mem0+1],mem[mem0+2]);
			mem0+=4;
		}
	}
	void run(){
		while(IF()){
			ID();EX();MEM();WB();
		}
		printf("%u",X[10]&255u);
	}
	void printIS(){
		for(int i=0;i<8388608;i+=4){
			printf("%x:\t%02x%02x%02x%02x\n",i,mem[i],mem[i+1],mem[i+2],mem[i+3]);
		}
	};
};
#endif //CODE_SIMULATOR_HPP
