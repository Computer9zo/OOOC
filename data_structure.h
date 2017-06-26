/*----------------------------------------------------*
/* data_structure.h
/* 17-06-26 21:37
/*----------------------------------------------------*
/* What is disp_message do?
/* - Show awesome texts and graphs
/* - example) main screen, program manual.
/* - Pause program until press enter
/* - Show error messages
/*----------------------------------------------------*/

#ifndef __DATA_STRUCTURE__
#define __DATA_STRUCTURE__

typedef enum Operation_code opcode;


typedef struct Instruction inst;



struct Instruction 
{
	opcode opcode;
	char     dest;
	char    oprd1;
	char    oprd2;
};




#endif // !__DATA_STRUCTURE__
