/*----------------------------------------------------*
/* data_structure.h
/* 17-06-26 21:37
/*----------------------------------------------------*
/* What is data_structure do?
/* - Give basic templet of datas
/* - example) Instruction, FQ_element.
/*----------------------------------------------------*/

#ifndef __DATA_STRUCTURE__
#define __DATA_STRUCTURE__

typedef enum   OP_code     opcode;   //Operation code.

typedef struct Instruction inst;     //Instruction.
typedef struct FQ_element  fq_elem;  //Fetch_queue element
typedef struct RS_element  rs_elem;  //Reservatoin_station element

struct Instruction 
{
	opcode opcode;//Operation code. It save type of operation.
	char     dest;//Destination. It save target RAT.
	char    oprd1;//Operand 1. It save RAT reference.
	char    oprd2;//Operand 2. It save RAT reference.
};

struct Fetch_queue_element
{
	inst inst;    //Fetched instruction. It save infomation of instruction
	int  inst_from;//Number of Instruction from. It save file that instruction from 
};

struct Reservation_Station_element
{
	inst inst;    //Fetched instruction. It save infomation of instruction

};


#endif // !__DATA_STRUCTURE__
