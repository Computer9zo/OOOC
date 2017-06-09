// Data struectures for OoO Execution CPU
#ifndef DATA_TYPES
#define DATA_TYPES

#include <stdbool.h>
#include <stdio.h>

enum flag { Q = 0, V = 1 }; // has V? or Q?
enum instruction { IntAlu = 0, MemRead = 1, MemWrite = 2 }; // Instruction Type
enum is_complete { P = 0, C = 1 }; // P means pending, C means completed
const char* instruction_name[3];

struct CONFIG;     // Config file
struct REPORT;     // Report file
struct INST;       // Single Instruction
struct FQ;         // Single element in Fetch Queue
struct RAT;        // Single element in Register Address Table
struct RS;         // Single element in Reservation Station
struct ROB;        // Single element in ReOrder Buffer
struct CA_status;  // Status of cyclic array


//for debuging
void INST_printer(const struct INST* printed); 
void FQ_printer(const struct FQ* printed);
void Config_printer(const struct CONFIG* printed);
void RAT_printer(const struct RAT* printed);
void RS_printer(const struct RS* printed, struct CA_status* rob_status);
void ROB_printer(const struct ROB* printed);

void FQ_arr_printer(const struct FQ* fq, struct CA_status fq_status);
void RAT_arr_printer(const struct RAT* rat, int rat_size);
void RS_arr_printer(const struct RS *rs, int rs_size,struct CA_status* rob_status);
void ROB_arr_printer(const struct ROB *rob, struct CA_status rob_status);

//for reporting
void RS_reporter(const struct RS* printed, struct CA_status* rob_status);
void ROB_reporter(const struct ROB* printed);
void REPORT_reporter(const struct REPORT* printed);
void RS_arr_reporter(const struct RS *rs, int rs_size, struct CA_status* rob_status);
void ROB_arr_reporter(const struct ROB *rob, struct CA_status rob_status);

void REPORT_fprinter(const struct REPORT* printed, FILE* fileID);

//for ca
void ca_cnt_push(struct CA_status *status);
void ca_cnt_pop(struct CA_status *status);
int ca_next_pos(struct CA_status *status);
int ca_get_cidx(int idx,struct CA_status *status);//진짜 어레이 인덱스를 head부터의 거리로 바꿈

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

struct CONFIG//config
{
	int Dump;
	int Width;
	int ROB_size;
	int RS_size;
};

struct REPORT
{
	int Cycles;
	double IPC;
	int Total_Insts;
	int IntAlu;
	int MemRead;
	int MemWrite;
};

struct CA_status
{
	int size;
	int head;
	int occupied;
};

struct INST//instructrion
{
	enum instruction opcode;
	int dest;
	int oprd_1;
	int oprd_2;
};

struct FQ//equal INST, it mean fetch queue
{
	enum instruction opcode;
	int dest;
	int oprd_1;
	int oprd_2;
};

struct RAT//Register AT
{
	bool RF_valid;
	int Q;
};

struct RS//Res Staton
{
	int rob_dest;
	bool is_valid; // Busy or not
	enum instruction opcode;
	int time_left; // Shows cycles left to be finished. -1 means not started
	struct
	{
		enum flag state;
		union 
		{
			int v;
			int q;
		} data;
	} oprd_1;
	struct
	{
		enum flag state;
		union
		{
			int v;
			int q;
		} data;
	} oprd_2;
};

struct ROB// Reorder Buffer
{
	enum instruction opcode; // operation
	int dest; // Destination ( Must point to one of elements in reg[16] !
	enum is_complete status; // status = P; means pending, status = C; means completed
	int rs_dest;//linked RS index
};


#endif // !DATA_TYPES
