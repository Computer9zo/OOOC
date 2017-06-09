#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "data_structures.h"

//파일 내 글로벌로 선언
static int inst_length;
static int pc = 0;
static int cycle = 1;

static int fetch_blank = 0;
static int decoded = 0;
static int issued = 0;
static int ROB_blank = 0;
static int cnt_IntAlu = 0;
static int cnt_MemRead = 0;
static int cnt_MemWrite = 0;

struct REPORT *core_simulator(struct CONFIG *config, struct INST *arr_inst, int arr_inst_len);

void fetch(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct INST *arr_inst);
void decode(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct RS *rs_ele, int rs_idx, struct RAT *rat, struct ROB *rob, struct CA_status *rob_status);
void value_payback(struct RS *rs_ele, struct ROB *rob);
void decode_and_value_payback(struct CONFIG * config, struct ROB *rob, struct CA_status *rob_status, struct RS * rs, bool *is_completed_this_cycle, struct FQ * fetch_queue, struct CA_status * fq_status, struct RAT * rat);

void issue(struct CONFIG *config, struct RS *rs_ele);
void execute(struct RS *rs_ele, struct ROB* rob_ele, bool *is_completed_this_cycle);
void rs_retire(struct RS *rs_ele, struct ROB *rob_ele);
void ex_and_issue(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RS *rs, bool *is_completed_this_cycle);

void wait(void);

void commit(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RAT *rat);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct REPORT *core_simulator(struct CONFIG *config, struct INST *arr_inst, int arr_inst_len)
{
	//출력을 위한 공백 확보
	printf("\n");

	//파일 내 글로벌 초기화
	pc = 0;
	cycle = 0;
	inst_length = arr_inst_len;

	decoded = 0;
	issued = 0;
	cnt_IntAlu = 0;
	cnt_MemRead = 0;
	cnt_MemWrite = 0;

	// Initializing ...

	int i;

	// RAT
	struct RAT rat[17]; // 0 means no. rat[1] ~ rat[16] are Arch Register entries
	for (i = 0; i < 17; i++) { rat[i].Q = 0; rat[i].RF_valid = true; }//initialize

	// Fetch Queue
	struct FQ* fetch_queue = calloc(2 * (*config).Width, sizeof(struct FQ) );//struct FQ fetch_queue[2 * (*config).Width]
	struct CA_status fq_status;
	fq_status.size = 2 * (*config).Width;
	fq_status.head = 0;
	fq_status.occupied = 0;

	// Reservation Station
	struct RS* rs = calloc((*config).RS_size, sizeof(struct RS)); //struct RS rs[(*config).RS_size];
	// is_completed_this_cycle? to prevent issue of insturction in RS which has V value promoted from Q in current cycle.
	bool* is_completed_this_cycle = calloc((*config).RS_size, sizeof(bool));//bool is_completed_this_cycle[(*config).ROB_size];
	for (i = 0; i < (*config).RS_size; i++) { rs[i].is_valid = false; is_completed_this_cycle[i] = false;}//initialize

	// ReOrdering Budder
	struct ROB* rob = calloc((*config).ROB_size, sizeof(struct ROB));//struct ROB rob[(*config).ROB_size];
	struct CA_status rob_status;
	rob_status.size = (*config).ROB_size;
	rob_status.head = 0;
	rob_status.occupied = 0;
	
	//check all is accesable
	if (fetch_queue==NULL|| rs==NULL || rob==NULL|| is_completed_this_cycle==NULL)
	{//error
		return NULL;
	}

	do
	{	
		//cycle pluse
		cycle++;

		//각 명령의 실행 횟수를 초기화한다.
		fetch_blank = (fq_status.size - fq_status.occupied);//패치큐가 얼마나 비었는지 확인
		ROB_blank = (rob_status.size - rob_status.occupied);//ROB가 얼마나 비었는지 확인
		decoded = 0;
		issued = 0;

		//Loop1
		//ROB를 rob_status.occupied만큼 돌면서 commit/ (ex/issue) 실행

		commit(config, rob, &rob_status, rat);

		ex_and_issue(config, rob, &rob_status, rs, is_completed_this_cycle);

		//Loop2
		//RS를 전부 돌면서 decode / value_feeding , is_completed_this_cycle array 도 초기화.
		decode_and_value_payback(config, rob, &rob_status, rs,is_completed_this_cycle, fetch_queue,&fq_status,rat);


		// Fetch instructions
		fetch(config, fetch_queue, &fq_status, arr_inst);


		// Dump
		switch ((*config).Dump)
		{
		case 0:
			//지겨움 방지
			//if (pc % (inst_length / 100) == 0)
			if (cycle % 1000 == 1)
			{
				if (cycle == 1)
				{
					printf("Simulating =");
				}
				printf("%3d%%\b\b\b\b", pc*100 / inst_length);
			}
			break;
		case 1:
			printf("= Cycle %-5d\n", cycle);
			ROB_arr_reporter(rob, rob_status);
			break;
		case 2:
			printf("= Cycle %-5d\n", cycle);
			RS_arr_reporter(rs, (*config).RS_size, &rob_status);
			ROB_arr_reporter(rob, rob_status);
			break;
		default:
			//debug mode
			printf("= Cycle %-5d\n", cycle);
			FQ_arr_printer(fetch_queue, fq_status);
			RAT_arr_printer(rat, 17);
			RS_arr_printer(rs, (*config).RS_size, &rob_status);
			ROB_arr_printer(rob, rob_status);
			wait();
			break;
		}
		


	} while (fq_status.occupied>0 || rob_status.occupied > 0);

	// Simulation finished
	
	//free array
	free(fetch_queue);
	free(rs);
	free(rob);

	free(is_completed_this_cycle);
	

	// Write a report
	struct REPORT *ptr_report = malloc(sizeof(struct REPORT));
	(*ptr_report).Cycles = cycle;
	(*ptr_report).Total_Insts = inst_length;
    (*ptr_report).IntAlu = cnt_IntAlu;
	(*ptr_report).MemRead = cnt_MemRead;
	(*ptr_report).MemWrite = cnt_MemWrite;
	(*ptr_report).IPC = ((double)inst_length / (double) cycle);


	if (config->Dump == 0) {printf("100%%");}
	printf("\n= Final State\n");
	REPORT_reporter(ptr_report);	// display a report
	printf("\n");
	return ptr_report;	
}

void fetch(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct INST *arr_inst)
{
	int fetch_num = ((*config).Width > fetch_blank) ? fetch_blank : (*config).Width;
	int i;
	for (i = 0; i < fetch_num && pc < inst_length; i++)
	{
		fetch_queue[ca_next_pos(fq_status)].opcode = arr_inst[pc].opcode;
		fetch_queue[ca_next_pos(fq_status)].dest = arr_inst[pc].dest;
		fetch_queue[ca_next_pos(fq_status)].oprd_1 = arr_inst[pc].oprd_1;
		fetch_queue[ca_next_pos(fq_status)].oprd_2 = arr_inst[pc].oprd_2;
		ca_cnt_push(fq_status);
		pc++;
	}
}

void decode(struct CONFIG *config, struct FQ *fetch_queue, struct CA_status *fq_status, struct RS *rs_ele, int rs_idx, struct RAT *rat, struct ROB *rob, struct CA_status *rob_status)
{
	// Decode only when: 1) fq is not empty. 2) Upto N instructions. 3) ROB has empty place
	if ((*fq_status).occupied > 0 && decoded < (*config).Width && decoded < ROB_blank)
	{

		struct FQ * fq_ele;
		fq_ele = fetch_queue + ((*fq_status).head); //디코드해올 fq의 inst


		// Count Instruction number
		switch (fetch_queue->opcode)
		{
		case 0:
			cnt_IntAlu++;
			break;
		case 1:
			cnt_MemRead++;
			break;
		case 2:
			cnt_MemWrite++;
			break;
		}

		// Putting first element of Fetch Queue to Reservation Station	
		(*rs_ele).is_valid = true;
		(*rs_ele).opcode = fq_ele->opcode;

		int oprd_1 = fq_ele->oprd_1;
		int oprd_2 = fq_ele->oprd_2;
		if (oprd_1 == 0 || rat[oprd_1].RF_valid)
		{
			(*rs_ele).oprd_1.state = V;
		}
		else
		{
			(*rs_ele).oprd_1.state = Q;
			(*rs_ele).oprd_1.data.q = rat[oprd_1].Q;
		}

		if (oprd_2 == 0 || rat[oprd_2].RF_valid)
		{
			(*rs_ele).oprd_2.state = V;
		}
		else
		{
			(*rs_ele).oprd_2.state = Q;
			(*rs_ele).oprd_2.data.q = rat[oprd_2].Q;
		}

		(*rs_ele).time_left = -1; // Waiting to be issued

		// Putting first element of Fetch Queue to ROB
		rob[ca_next_pos(rob_status)].opcode = fq_ele->opcode;
		rob[ca_next_pos(rob_status)].dest = fq_ele->dest;
		rob[ca_next_pos(rob_status)].rs_dest = rs_idx;
		rob[ca_next_pos(rob_status)].status = P;
		(*rs_ele).rob_dest = ca_next_pos(rob_status);

		// Modify RAT status
		if (fetch_queue[(*fq_status).head].dest != 0)
		{
			rat[fetch_queue[(*fq_status).head].dest].RF_valid = false;		// Now data isn't inside the Arch Register file anymore
			rat[fetch_queue[(*fq_status).head].dest].Q = ca_next_pos(rob_status);   // So leave reference to ROB entry in RAT
		}

		ca_cnt_push(rob_status); // Element has been pushed to ROB
		ca_cnt_pop(fq_status);   // Element has been poped from Fetch Queue

		decoded++;
	}
}

void value_payback(struct RS *rs_ele, struct ROB *rob)
{
	if (rs_ele->time_left<0)//if it isn't issued
	{
		// Q ->V Promotion
		if ((*rs_ele).oprd_1.state == Q && rob[((*rs_ele).oprd_1.data.q)].status)
		{
			(*rs_ele).oprd_1.state = V;
		}

		if ((*rs_ele).oprd_2.state == Q && rob[((*rs_ele).oprd_2.data.q)].status)
		{
			(*rs_ele).oprd_2.state = V;
		}
	}
}

void decode_and_value_payback(struct CONFIG * config, struct ROB *rob, struct CA_status *rob_status, struct RS * rs, bool *is_completed_this_cycle, struct FQ * fetch_queue, struct CA_status * fq_status, struct RAT * rat)
{
	// For every entries in Reservation Station,
	for (int i = 0; i < (*config).RS_size; i++)
	{
		if (rs[i].is_valid) // Instruction is inside this entry of RS
		{
			value_payback(rs + i, rob);//check args is ready.
		}
		else// This entry of RS is empty now
		{
			if (false==is_completed_this_cycle[i])//if not this entry flushed this cycle,
			{
				decode(config, fetch_queue, fq_status, rs+i, i, rat, rob, rob_status);// Try to decode instruction into empty place
			}
		}

		// Flush is_completed_this_cycle array
		is_completed_this_cycle[i] = false;

	}

}


void issue(struct CONFIG *config, struct RS *rs_ele)
{
	if (issued < (*config).Width)
	{//아직 width만큼 issue가 되지 않았다면
		if ((*rs_ele).oprd_1.state == V &&
			(*rs_ele).oprd_2.state == V)
		{//issue 조건을 검사하고 만족한다면, issue를 한다.

			(*rs_ele).time_left = ((*rs_ele).opcode == MemRead) ? 2 : 0;
			++issued;

		}
			
	}
}

void execute(struct RS *rs_ele, struct ROB* rob_ele, bool *is_completed_this_cycle)
{
	//if ( executed < (*config).Width)
	//이미 이슈가 최대 N개까지 가능하기 때문에, ex도 최대 N개까지만 수행된다. 검사필요 없음

	if (rs_ele->time_left == 0)
	{//만약 실행 대기중이라면, 실행하고 RS를 비운 다음 ROB를 C 상태로 바꾼다.
		
		rs_retire(rs_ele, rob_ele);
		(*is_completed_this_cycle) = true;

	}
	else
	{//만약 issue가 되어있다면,
		--(rs_ele->time_left);//ALU에 넣는다 ( 실행 대기 단계 ).
	}

}

void ex_and_issue(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RS *rs, bool *is_completed_this_cycle)
{
	int i;
	struct ROB * rob_ele;
	struct RS  * rs_ele;
	for (i = 0; i < (rob_status->occupied); ++i)
	{//모든 원소에 대해, 검사한다

		rob_ele = rob + ((rob_status->head + i) % rob_status->size);
		if (rob_ele->status == P)
		{//만약 상태가 P라면, 이는 issue의 대상과 ex의 대상을 포함한다.
			
			rs_ele = (rs + (rob_ele->rs_dest));

			if (rs_ele->time_left >= 0)
			{//만약 Issue 되었다면
				execute(rs + (rob_ele->rs_dest), rob_ele, is_completed_this_cycle + (rob_ele->rs_dest));
				//실행 및 실행 완료한다.
			}
			else
			{//만약 Issue 안되었다면
				issue(config, rs_ele);
				//이슈 조건을 검사하고 부합할 경우 이슈한다.
			}

		}

	}

}

void wait(void)
{
	printf("\nPress any key to continue :\n");
	getchar();
}

void rs_retire(struct RS *rs_ele, struct ROB *rob_ele)
{
	rob_ele->status = C;
	(*rs_ele).is_valid = false;
}

void commit(struct CONFIG *config, struct ROB *rob, struct CA_status *rob_status, struct RAT *rat)
{
	int i;
	int num_of_search;
	struct ROB* rob_head;
	// Only permits upto N commits
	num_of_search = (*config).Width > (*rob_status).occupied ? (*rob_status).occupied : (*config).Width;

	//현재 ROB의 원소 개수와 Width 중 작은 숫자까지만 확인하면 됨.
	//왜냐면 최대 커밋은 연속적으로 C인 경우에만 Width까지 가능하고(최대한 줄여도 Head - Width 사이)
	//원소 개수가 Width보다 작다면, 최대 커밋은 원소 개수까지만 가능하기 때문.

	for (i = 0; i < num_of_search; i++)
	{
		rob_head = rob + (rob_status->head);
		if ( (rob_head->status) == C)
		{
			rat[rob_head->dest].RF_valid = true;
			ca_cnt_pop(rob_status);
		}
		else
		{
			break;
		}
	}
}
