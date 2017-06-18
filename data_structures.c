#define DUMP_WIDTH 4  //어레이 형태로 레지스터 출력시 가로 줄 개수

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
	result.ll = ll_cnt_init(size_of_queue);

	if ( (result.rob == NULL) || (result.ll.next == NULL) || (result.ll.prev = NULL) ) { result.ll.head = -1; }

	return result;
}
void ROB_delete(struct ROB_ARR rob_arr)
{
	free(rob_arr.rob);
	ll_delete(&(rob_arr.ll));
}

void INST_printer(const struct INST* printed)
{//print inst
	printf("%-10s", instruction_name[printed->opcode]);
	printf("%5d", printed->dest);
	printf("%5d", printed->oprd_1);
	printf("%5d", printed->oprd_2);
}

void FQ_printer(const struct FQ* printed)
{
	INST_printer((struct INST*)printed);
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

void RS_printer(const struct RS* printed, struct CA_status* rob_status)
{
	if (printed->is_valid)
	{
		printf("ROB%-5d", ca_get_cidx(printed->rob_dest, rob_status) + 1);
		
		if (printed->oprd_1.state == V){printf("V     ");}
		else { printf("Q:%-3d ", ca_get_cidx(printed->oprd_1.data.q, rob_status) + 1); }
		
		if (printed->oprd_2.state == V) { printf("V     "); }
		else { printf("Q:%-3d ", ca_get_cidx(printed->oprd_2.data.q, rob_status) + 1); }

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
}

void FQ_arr_printer(const struct FQ* fq, struct CA_status fq_status)
{
	printf("Fetch queue\n");

	const struct FQ* fq_idx = NULL;
	int idx;
	for (idx = 0; idx < fq_status.size; ++idx)
	{
		printf("| FQ%-4d: ", idx + 1);

		if (idx < fq_status.occupied)
		{//데이터가 있으면 출력한다.
			fq_idx = fq + ((fq_status.head + idx) % fq_status.size);
			FQ_printer(fq_idx);
		}
		else 
		{//실제 원소 개수 이상의 공간은 쓰레기값이므로 공백을 출력한다.
			printf("                         "); 
		}

		printf(" ");
		
		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) {printf("|\n");	}//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) {printf("\n");}//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

void RAT_arr_printer(const struct RAT* rat, int rat_size)
{
	printf("RAT\n");
	const struct RAT *rat_idx = NULL;
	int idx;
	for (idx = 0; idx < rat_size-1; ++idx)
	{
		//레지스터 0번은 존재하지 않는 주소(상수)이므로, 1번부터 출력한다..
		rat_idx = rat + (idx+1);
		printf("| R%-4d : ", idx+1);
		RAT_printer(rat_idx);
		printf(" ");

		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) { printf("|\n"); }//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) { printf("\n"); }//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

void RS_arr_printer(const struct RS *rs, int rs_size, struct CA_status* rob_status)
{
	printf("Reservation station\n");

	const struct RS *rs_idx = NULL;//편의를 위한 임시 저장 변수
	int idx;

	for (idx = 0; idx < rs_size; ++idx)
	{//모든 RS를 출력한다.
		rs_idx = rs + (idx);
		printf("| RS%-4d : ", idx + 1);
		RS_printer(rs_idx, rob_status);

		printf(" ");

		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) { printf("|\n"); }//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) { printf("\n"); }//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

void ROB_arr_printer(const struct ROB *rob, struct CA_status rob_status)
{
	printf("Reorder buffer\n");

	const struct ROB *rob_idx = NULL;
	int idx;
	for (idx = 0; idx < rob_status.size; ++idx)
	{
		printf("| ROB%-4d: ", idx + 1);
		if (idx < rob_status.occupied)
		{//데이터가 있으면 출력한다.
			rob_idx = rob + ((rob_status.head + idx) % rob_status.size);
			ROB_printer(rob_idx);
		}
		else
		{//실제 원소 개수 이상의 공간은 쓰레기값이므로 공백을 출력한다.
			printf("                           ");
		}

		if (idx % DUMP_WIDTH == DUMP_WIDTH - 1) { printf("|\n"); }//줄바꿈을 위한 구문
	}
	if (idx % DUMP_WIDTH != 0) { printf("\n"); }//DUMP_WIDTH 배수가 아닌 경우. 구분을 위해 줄바꿈을 한번 해준다.
}

//for reporting
void RS_reporter(const struct RS* printed, struct CA_status * rob_status)
{
	if (printed->is_valid)
	{
		printf("ROB%-5d", ca_get_cidx(printed->rob_dest, rob_status)+1);
		(printed->oprd_1.state == V) ? printf("    V") : printf("%5d", ca_get_cidx(printed->oprd_1.data.q, rob_status) + 1);
		(printed->oprd_2.state == V) ? printf("    V") : printf("%5d", ca_get_cidx(printed->oprd_2.data.q, rob_status) + 1);
	}
	else
		printf("ROB0        0    0");
}
void ROB_reporter(const struct ROB* printed)
{
	printf("%c", (printed->status==C)?'C':'P');
}
void REPORT_reporter(const struct REPORT* printed)
{
	printf("%-15s%d\n", "Cycles", printed->Cycles);
	printf("%-15s%f\n", "IPC", printed->IPC);
	printf("%-15s%d\n", "Total Insts", printed->Total_Insts);
	printf("%-15s%d\n", "IntAlu", printed->IntAlu);
	printf("%-15s%d\n", "MemRead", printed->MemRead);
	printf("%-15s%d\n", "MemWrite", printed->MemWrite);
}
void RS_arr_reporter(const struct RS *rs, int rs_size, struct CA_status * rob_status)
{
	const struct RS *rs_idx = NULL;
	int idx;
	for (idx = 0; idx < rs_size; ++idx)
	{
		rs_idx = rs + (idx);
		printf("RS%-4d : ", idx + 1);
		RS_reporter(rs_idx, rob_status);
		printf("\n");
	}
}
void ROB_arr_reporter(const struct ROB *rob, struct CA_status rob_status)
{
	const struct ROB *rob_idx = NULL;
	int idx;
	for (idx = 0; idx < rob_status.size; ++idx)
	{
		printf("ROB%-4d: ", idx + 1);
		
		if (idx < rob_status.occupied)
		{
			rob_idx = rob + ((rob_status.head + idx) % rob_status.size);
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
}


//for ca using
void ca_cnt_push(struct CA_status *status)
{
	(*status).occupied++;
}

void ca_cnt_pop(struct CA_status *status)
{
	(*status).head = ((*status).head + 1) % (*status).size;
	(*status).occupied--;
}

int ca_next_pos(struct CA_status *status)
{
	return ( (*status).head + (*status).occupied ) % (*status).size ;
}

int ca_get_cidx(int idx, struct CA_status *status)
{
	return ( idx - (*status).head + (*status).size ) % (*status).size;
}

struct LL_status ll_cnt_init(int size)
{
	struct LL_status result;
	result.head = 0;
	result.tail = 0;
	result.occupied = 0;
	result.size = size;
	result.next = (int*)calloc(size, sizeof(int));
	result.prev = (int*)calloc(size, sizeof(int));
	if (result.next == NULL || result.prev == NULL) { result.size = 0; return result; }

	for (int i = 0; i<result.size; ++i){
		result.next[i] = i + 1;
		result.prev[i] = i - 1;
	}
	result.next[result.size -1] = 0;
	result.prev[0] = size-1;

	return result;
}

void ll_cnt_pop(struct LL_status *status, int pop_num)
{
	//전단계의 원소의 next를 현재 원소의 next로 다음 단계의 prev를 현재 원소의 prev로
	status->next[status->prev[pop_num]] = status->next[pop_num];
	status->prev[status->next[pop_num]] = status->prev[pop_num];

	//현재 원소는 맨 끝의 원소 뒤에 붙이고, 0을 가르키게 한다.
	status->next[status->prev[status->head]] = pop_num;
	status->prev[pop_num] = status->prev[status->head];
	status->next[pop_num] = status->head;
	status->prev[status->head] = pop_num;

	//그리고 점유를 하나 뺀다
	--(status->occupied);
}

void ll_cnt_push(struct LL_status *status)
{
	++(status->occupied);
	status->tail = status->next[status->tail];
}

int ll_next_pos(struct LL_status *status, int origin_pos)
{
	return status->next[origin_pos];
}

void ll_delete(struct LL_status *status)
{
	free(status->next);
	free(status->prev);
}