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
struct THREAD;   // Instruction array with length and pc
struct FQ;         // Single element in Fetch Queue
struct FQ_ARR;	   // Fetch Queue
struct RAT;        // Single element in Register Address Table
struct RAT_ARR;        // Register Address Table
struct RS;         // Single element in Reservation Station
struct RS_ARR;         //Reservation Station
struct ROB;        // Single element in ReOrder Buffer
struct RS_ARR;      //ReOrder Buffer
struct CA_status;  // Status of cyclic array
struct LL_status;  // Status of Limited Linked list
struct LSQ;// Load Store queue
struct LSQ_ARR;// Load Store queue

//creator & deletor

struct THREAD THREAD_create(struct INST* inst_arr, int inst_len);
void THREAD_delete(struct THREAD* thread);

struct RAT_ARR RAT_create(int num_of_register);
void RAT_delete(struct RAT_ARR rat);

struct FQ_ARR FQ_create(int size_of_queue);
void FQ_delete(struct FQ_ARR fq_arr);

struct RS_ARR RS_create(int size_of_queue);
void RS_delete(struct RS_ARR rs_arr);

struct ROB_ARR ROB_create(int size_of_queue);
void ROB_delete(struct ROB_ARR rob_arr);

struct LSQ_ARR LSQ_create(int size_of_queue);
void LSQ_delete(struct LSQ_ARR rob_arr);

//for debuging
void INST_printer(const struct INST* printed); 
void FQ_printer(const struct FQ* printed);
void CONFIG_printer(const struct CONFIG* printed);
void RAT_printer(const struct RAT* printed);
void RS_printer(const struct RS* printed, const struct LL_status* rob_status);
void ROB_printer(const struct ROB* printed);
void LSQ_printer(const struct LSQ* printed, const struct LL_status* rob_status);

void FQ_arr_printer(const struct FQ_ARR* fq);
void RAT_arr_printer(const struct RAT_ARR* rat);
void RS_arr_printer(const struct RS_ARR *rs, const struct ROB_ARR *rob);
void ROB_arr_printer(const struct ROB_ARR *rob);
void LSQ_arr_printer(const struct LSQ_ARR *lsq, const struct ROB_ARR *rob);
//for reporting
void RS_reporter(const struct RS* printed, const struct LL_status* rob_status);
void ROB_reporter(const struct ROB* printed);
void LSQ_reporter(const struct LSQ* printed, const struct LL_status* rob_status);
void REPORT_reporter(const struct REPORT* printed);
void RS_arr_reporter(const struct RS_ARR *rs, const struct ROB_ARR *rob);
void ROB_arr_reporter(const struct ROB_ARR *rob);
void LSQ_arr_reporter(const struct LSQ_ARR *lsq, const struct ROB_ARR *rob);

void REPORT_fprinter(const struct REPORT* printed, FILE* fileID);

//for ca
void ca_cnt_push(struct CA_status *status);
void ca_cnt_pop(struct CA_status *status);
int ca_next_pos(struct CA_status *status);
int ca_get_cidx(int idx,struct CA_status *status);//진짜 어레이 인덱스를 head부터의 거리로 바꿈

//for ll
struct LL_status ll_cnt_init(int size);
void ll_cnt_pop(struct LL_status *status, int pop_num);
void ll_cnt_push(struct LL_status *status);
int ll_next_pos(const struct LL_status *status, const int origin_pos);
int ll_get_cidx(const struct LL_status *status, const int target_idx);
void ll_delete(struct LL_status *status);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//

struct CONFIG//config
{
	int Dump;
	int Width;
	int ROB_size;
	int RS_size;
	int LSQ_size;
	int Cache_size;
};

struct REPORT
{
	int Cycles;
	double IPC;
	int Total_Insts;
	int IntAlu;
	int MemRead;
	int MemWrite;
	int num_of_inst;
	int*Inst_per_thread;
};

struct CA_status
{
	int size;
	int head;
	int occupied;
};

struct LL_status
{
	int head;
	int* next;
	int* prev;
	int tail;

	int size;
	int occupied;
};

struct INST//instructrion
{
	enum instruction opcode;
	int dest;
	int oprd_1;
	int oprd_2;
};

struct THREAD//pc and length instruction
{
	struct INST* instruction;
	int length;
	int pc;
};

struct FQ//equal INST, it mean fetch queue
{
	enum instruction opcode;
	int dest;
	int oprd_1;
	int oprd_2;
	int inst_num;
};

struct FQ_ARR//fetch queue
{
	struct FQ* fq;
	struct CA_status ca;
};

struct RAT//Register AT
{
	bool RF_valid;
	int Q;
};

struct RAT_ARR//Register AT
{
	struct RAT* rat;
	int size;
};

struct RS//Res Staton
{
	int rob_dest;
	int lsq_dest;
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
	bool is_completed_this_cycle;
};

struct RS_ARR
{
	struct RS* rs;
	int size;
};

struct ROB// Reorder Buffer
{
	enum instruction opcode; // operation
	int dest; // Destination ( Must point to one of elements in reg[16] !
	enum is_complete status; // status = P; means pending, status = C; means completed
	int rs_dest;//linked RS index
	int inst_num;//where it from
	int lsq_source;
};

struct ROB_ARR
{
	struct ROB* rob;
	struct LL_status ll;
};

enum miss_type { HIT = 1, MISS = 0, FORWARD = -1 };

struct LSQ// Load Store queue
{
	enum instruction opcode; // operation
	int time; //
	int rob_dest; // status = P; means pending, status = C; means completed
	int address;//where it from
	enum miss_type was_hit;
	enum is_complete status;
};

struct LSQ_ARR// Load Store queue
{
	struct LSQ * lsq;
	struct LL_status ll;
};
#endif // !DATA_TYPES
