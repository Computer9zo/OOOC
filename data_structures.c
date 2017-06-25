#define DUMP_WIDTH 4  //어레이 형태로 레지스터 출력시 가로 줄 개수

#include <stdlib.h>
#include <stdio.h>
#include "data_structures.h"

const char* instruction_name[3] = { "IntALU", "MemRead" , "MemWrite" };

struct THREAD THREAD_create(struct INST* inst_arr, int inst_len)
{
	struct THREAD result;
	result.instruction = inst_arr;
	result.length = inst_len;
	result.pc = 0;
	return result;
}

void THREAD_delete(struct THREAD* thread)
{
	free(thread->instruction);
}

struct RAT_ARR RAT_create(int num_of_register)
{
	struct RAT_ARR result;
	result.rat = (struct RAT*)calloc(num_of_register, sizeof(struct RAT));
	result.size = num_of_register;

	if (result.rat == NULL) { result.size = 0; } //no mem

	for (int i = 0; i < result.size; i++) {
		result.rat[i].Q = 0; result.rat[i].RF_valid = true;
	}//initialize
	return result;
}
void RAT_delete(struct RAT_ARR rat)
{
	free(rat.rat);
}

struct FQ_ARR FQ_create(int size_of_queue)
{
	struct FQ_ARR FQ;
	FQ.fq = calloc(size_of_queue , sizeof(struct FQ));//struct FQ fetch_queue[2 * (*config).Width]
	FQ.ca.size = size_of_queue;
	FQ.ca.head = 0;
	FQ.ca.occupied = 0;

	if (FQ.fq == NULL) { FQ.ca.size = 0; }

	return FQ;
}
void FQ_delete(struct FQ_ARR fq_arr)
{
	free(fq_arr.fq);
}

struct RS_ARR RS_create(int size_of_queue)
{
	struct RS_ARR result;
	
	result.rs = (struct RS *)calloc(size_of_queue, sizeof(struct RS));
	result.size = size_of_queue;

	if (result.rs == NULL ) { result.size = 0; }

	for (int i = 0; i < result.size; i++) {
		result.rs[i].is_valid = false; result.rs[i].is_completed_this_cycle = false;
	}//initialize

	return result;
}
void RS_delete(struct RS_ARR rs_arr)
{
	free(rs_arr.rs);
}

struct ROB_ARR ROB_create(int size_of_queue)
{
	struct ROB_ARR result;
	result.rob = (struct ROB*)calloc(size_of_queue, sizeof(struct ROB));
	result.ll = LL_create(size_of_queue);

	if ( (result.rob == NULL) || (result.ll.next == NULL) || (result.ll.prev = NULL) ) { result.ll.size = 0; }

	return result;
}
void ROB_delete(struct ROB_ARR rob_arr)
{
	free(rob_arr.rob);
	LL_delete(&(rob_arr.ll));
}

struct LSQ_ARR LSQ_create(int size_of_queue)
{
	struct LSQ_ARR result;
	result.lsq = (struct LSQ*)calloc(size_of_queue, sizeof(struct LSQ));
	result.ll = LL_create(size_of_queue);

	if (result.lsq == NULL) { result.ll.size = 0; }

	return result;
}
void LSQ_delete(struct LSQ_ARR lsq_arr)
{
	free(lsq_arr.lsq);
	LL_delete(&(lsq_arr.ll));
}

void INST_printer(const struct INST* printed)
{//print inst
	printf("%-10s", instruction_name[printed->opcode]);
	printf("%3d", printed->dest);
	printf("%5d", printed->oprd_1);
	printf("%5d  ", printed->oprd_2);
}

void FQ_printer(const struct FQ* printed)
{
	INST_printer((struct INST*)printed);
	printf("T%-2d", printed->inst_num + 1);
}

void CONFIG_printer(const struct CONFIG* printed)
{
	printf("Dump = %d, ", printed->Dump);
	printf("Width =%5d, ", printed->Width);
	printf("ROB size =%5d, ", printed->ROB_size);
	printf("RS size =%5d", printed->RS_size);
}

void RAT_printer(const struct RAT* printed)
{
	printf("%s ", (printed->RF_valid)?"Valid":"Dirty");
	if (printed->RF_valid)
		printf("    -");
	else
		printf("%5d", printed->Q+1);
}

//TODO: Fix RS_printer, LSQ_printer, RS_reporter, LSQ_reporter. And ll_next_pos, ll_get_cidx has different declaration from header 
void RS_printer(const struct RS* printed, const struct LL_status* rob_status)
{
	if (printed->is_valid)
	{
		// printf("ROB%-5d", ll_get_cidx(printed->rob_dest, rob_status) + 1);
		printf("ROB%-5d", LL_get_cidx(rob_status, printed->rob_dest) + 1);
		
		if (printed->oprd_1.state == V){printf("V     ");}
		else { printf("Q:%-3d ", LL_get_cidx(rob_status, printed->oprd_1.data.q) + 1); }
		
		if (printed->oprd_2.state == V) { printf("V     "); }
		else { printf("Q:%-3d ", LL_get_cidx(rob_status, printed->oprd_2.data.q) + 1); }

		printf("left%2d", printed->time_left);
	}
	else
		printf("                          ");
}

void ROB_printer(const struct ROB* printed)
{
	printf("%-10s", instruction_name[printed->opcode]);
	printf("R%-5d ", printed->dest);
	printf("%c   ", (printed->status==C)?'C':'P');
	printf("RS%-4d", printed->rs_dest+1);
	printf("INST%-2d", printed->inst_num + 1);
}

void LSQ_printer(const struct LSQ* printed, const struct LL_status* rob_status)
{
	printf("%-10s", instruction_name[printed->opcode]);
	printf("ROB%-5d ", LL_get_cidx(rob_status, printed->rob_dest) + 1);
	printf("addr%-9X ", printed->address);
	printf("T%-2d", printed->time);
	printf(" %c", (printed->status == C) ? 'C' : 'P');
}


void FQ_arr_printer(const struct FQ_ARR* fq)
{
	printf("Fetch queue\n");

	const struct FQ* fq_idx = NULL;
	int idx;
	for (idx = 0; idx < fq->ca.size; ++idx)
	{
		printf("| FQ%-4d: ", idx + 1);

		if (idx <  fq->ca.occupied)
		{//데이터가 있으면 출력한다.
			fq_idx = (fq->fq) + ((fq->ca.head + idx) % fq->ca.size);
			FQ_printer(fq_idx);
		}
		else 
		{//실제 원소 개수 이상의 공간은 쓰레기값이므로 공백을 출력한다.
			printf("                            "); 
		}

		printf(" ");
		
		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) {printf("|\n");	}//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) {printf("\n");}//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

void RAT_arr_printer(const struct RAT_ARR* rat)
{
	printf("RAT\n");
	const struct RAT *rat_idx = NULL;
	int idx;
	for (idx = 0; idx < rat->size-1; ++idx)
	{
		//레지스터 0번은 존재하지 않는 주소(상수)이므로, 1번부터 출력한다..
		rat_idx = (rat->rat) + (idx+1);
		printf("| R%-4d : ", idx+1);
		RAT_printer(rat_idx);
		printf(" ");

		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) { printf("|\n"); }//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) { printf("\n"); }//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

void RS_arr_printer(const struct RS_ARR *rs, const struct ROB_ARR *rob)
{
	printf("Reservation station\n");

	const struct RS *rs_idx = NULL;//편의를 위한 임시 저장 변수
	int idx;

	for (idx = 0; idx < rs->size; ++idx)
	{//모든 RS를 출력한다.
		rs_idx = (rs->rs) + (idx);
		printf("| RS%-4d : ", idx + 1);
		RS_printer(rs_idx, &(rob->ll));

		printf(" ");

		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) { printf("|\n"); }//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) { printf("\n"); }//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

void ROB_arr_printer(const struct ROB_ARR *rob)
{
	printf("Reorder buffer\n");

	const struct ROB *rob_idx = NULL;
	int idx;
	int ptr = rob->ll.head;
	for (idx = 0; idx < rob->ll.size; ++idx)
	{
		printf("| ROB%-4d: ", idx + 1);
		if (idx <  rob->ll.occupied)
		{//데이터가 있으면 출력한다.
			rob_idx = (rob->rob) + (ptr);
			ptr = rob->ll.next[ptr];
			ROB_printer(rob_idx);
		}
		else
		{//실제 원소 개수 이상의 공간은 쓰레기값이므로 공백을 출력한다.
			printf("                                 ");
		}

		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) { printf("|\n"); }//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) { printf("\n"); }//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

void LSQ_arr_printer(const struct LSQ_ARR *lsq, const struct ROB_ARR *rob)
{
	printf("Load store queue\n");

	const struct LSQ *lsq_idx = NULL;
	int idx;
	int ptr = lsq->ll.head;
	for (idx = 0; idx < lsq->ll.size; ++idx)
	{
		printf("| LSQ%-4d: ", idx + 1);
		if (idx <  lsq->ll.occupied)
		{//데이터가 있으면 출력한다.
			lsq_idx = (lsq->lsq) + (ptr);
			ptr = lsq->ll.next[ptr];
			LSQ_printer(lsq_idx,&(rob->ll));
		}
		else
		{//실제 원소 개수 이상의 공간은 쓰레기값이므로 공백을 출력한다.
			printf("                                   ");
		}

		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) { printf("|\n"); }//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) { printf("\n"); }//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

//for reporting
void RS_reporter(const struct RS* printed, const struct ROB_ARR* rob)
{
	const struct LL_status* rob_status = &(rob->ll);
	if (printed->is_valid)
	{
		printf("ROB%-5d", LL_get_cidx(rob_status, printed->rob_dest)+1);
		(printed->oprd_1.state == V) ? printf("V") : printf("%5d", LL_get_cidx(rob_status, printed->oprd_1.data.q) + 1);
		(printed->oprd_2.state == V) ? printf("  V  ") : printf("%5d", LL_get_cidx(rob_status, printed->oprd_2.data.q) + 1);
		printf("T%d", rob->rob[printed->rob_dest].inst_num);
	}
	else
		printf("ROB0    0    0    T-");
}
void ROB_reporter(const struct ROB* printed)
{
	printf("%c  ", (printed->status==C)?'C':'P');
	printf("T%d", printed->inst_num);
}
void LSQ_reporter(const struct LSQ* printed, const struct ROB_ARR* rob)
{
	const struct LL_status* rob_status = &(rob->ll);
	printf("%c  ", (printed->opcode == MemRead) ? 'L' : 'S');
	printf("ROB%-5d", LL_get_cidx(rob_status, printed->rob_dest) + 1);
	if (printed->address < 0) 
	{
		printf("%9X", 0);
	}
	else
	{
		printf("%9X", printed->address);
	}
	printf("T%d", rob->rob[printed->rob_dest].inst_num);
}
void REPORT_reporter(const struct REPORT* printed)
{
	printf("%-15s%d\n", "Cycles", printed->Cycles);
	printf("%-15s%f\n", "IPC", printed->IPC);
	printf("%-15s%d\n", "Total Insts", printed->Total_Insts);
	printf("%-15s%d\n", "IntAlu", printed->IntAlu);
	printf("%-15s%d\n", "MemRead", printed->MemRead);
	printf("%-15s%d\n", "MemWrite", printed->MemWrite);
	for (int i = 0; i < printed->num_of_inst; ++i){
		printf("%s%-8d%d\n", "Inst T", i + 1, (printed->Inst_per_thread)[i]);
	}
}
void RS_arr_reporter(const struct RS_ARR *rs, const struct ROB_ARR *rob)
{
	const struct RS *rs_idx = NULL;
	int idx;
	for (idx = 0; idx < rs->size; ++idx)
	{
		rs_idx = (rs->rs) + (idx);
		printf("RS%-4d : ", idx + 1);
		RS_reporter(rs_idx, rob);
		printf("\n");
	}
}

void LSQ_arr_reporter(const struct LSQ_ARR *lsq, const struct ROB_ARR *rob)
{
	const struct LSQ *lsq_idx = NULL;
	int idx;
	int ptr = rob->ll.head;
	for (idx = 0; idx < lsq->ll.size; ++idx)
	{
		printf("LSQ%-4d: ", idx + 1);
		
		if (idx < lsq->ll.occupied)
		{
			lsq_idx = (lsq->lsq) + (ptr);
			ptr = lsq->ll.next[ptr];
			LSQ_reporter(lsq_idx,rob);
		}
		printf("\n");

	}
}

void ROB_arr_reporter(const struct ROB_ARR *rob)
{
	const struct ROB *rob_idx = NULL;
	int idx;
	int ptr = rob->ll.head;
	for (idx = 0; idx < rob->ll.size; ++idx)
	{
		printf("ROB%-4d: ", idx + 1);

		if (idx < rob->ll.occupied)
		{
			rob_idx = (rob->rob) + (ptr);
			ptr = rob->ll.next[ptr];
			ROB_reporter(rob_idx);
		}
		else
		{
			printf("P");
		}
		printf("\n");

	}
}

void REPORT_fprinter(const struct REPORT* printed, FILE* fileID)
{
	fprintf(fileID, "%-15s%d\n", "Cycles", printed->Cycles);
	fprintf(fileID, "%-15s%f\n", "IPC", printed->IPC);
	fprintf(fileID, "%-15s%d\n", "Total Insts", printed->Total_Insts);
	fprintf(fileID, "%-15s%d\n", "IntAlu", printed->IntAlu);
	fprintf(fileID, "%-15s%d\n", "MemRead", printed->MemRead);
	fprintf(fileID, "%-15s%d\n", "MemWrite", printed->MemWrite);
	for (int i = 0; i < printed->num_of_inst; ++i) {
		fprintf(fileID,"%s%-8d%d\n", "Inst T", i + 1, (printed->Inst_per_thread)[i]);
	}
}


