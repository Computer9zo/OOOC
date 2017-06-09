#define DUMP_WIDTH 4  //어레이 형태로 레지스터 출력시 가로 줄 개수

#include <stdio.h>
#include "data_structures.h"

const char* instruction_name[3] = { "IntALU", "MemRead" , "MemWrite" };

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

void Config_printer(const struct CONFIG* printed)
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

	struct FQ* fq_idx = NULL;
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
	struct RAT *rat_idx = NULL;
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

	struct RS *rs_idx = NULL;//편의를 위한 임시 저장 변수
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

	struct ROB *rob_idx = NULL;
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
	struct RS *rs_idx = NULL;
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
	struct ROB *rob_idx = NULL;
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

//
//#include <stdlib.h>
//
//
//int main(void)
//{
//	struct INST inst = { 2,2,2,3 };
//	struct FQ fq = { 1,2,2,3 };
//	struct Config config = { 0 ,1 , 2 , 3 };
//	struct RAT rat1 = { Q , 12 };
//	struct RAT rat2 = { V , 12 };
//	struct RS rs1 = { 10, false ,1, 2, {V, 12},{ Q, 13 } };
//	struct RS rs2 = { 10, true ,2, -2,{ V, 12 },{ Q, 13 } };
//	struct ROB rob1 = { 0 ,1 , C };
//	struct ROB rob2 = { 0 ,2 , P };
//	struct ROB rob3 = { 1 ,-2 , P };
//
//	INST_printer(&inst);
//	FQ_printer(&fq);
//	Config_printer(&config);
//	RAT_printer(&rat1);
//	RAT_printer(&rat2);
//	RS_printer(&rs1);
//	RS_printer(&rs2);
//	ROB_printer(&rob1);
//	ROB_printer(&rob2);
//	ROB_printer(&rob3);
//
//	system("PAUSE");
//
//	FILE* fid=fopen("test.out", "w");
//	RS_fprinter(&rs1, fid);
//	RS_fprinter(&rs2, fid);
//	ROB_fprinter(&rob1, fid);
//	ROB_fprinter(&rob3, fid);
//	fclose(fid);
//}
//
