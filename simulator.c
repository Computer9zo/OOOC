#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "data_structures.h"
#include "cache.h"
//파일 내 글로벌로 선언




int core_simulator(struct CONFIG *config, struct INST **arr_inst, int* arr_inst_len, int num_of_inst, struct REPORT *out_report);
struct SIMUL_INFO;

void fetch(struct CONFIG *config, struct FQ_ARR *fetch_queue, struct THREAD *inst, struct SIMUL_INFO* info);
void decode(struct CONFIG *config, struct FQ_ARR *fetch_queue, struct RS *rs_ele, int rs_idx, struct RAT_ARR *rat, struct ROB_ARR *rob, struct ROB_ARR *lsq, int* decoded, struct SIMUL_INFO *info);
void value_payback(struct RS *rs_ele, struct ROB_ARR *rob);
void decode_and_value_payback(struct CONFIG * config, struct ROB_ARR *rob, struct ROB_ARR *lsq, struct RS_ARR * rs, struct FQ_ARR * fetch_queue, struct RAT_ARR * rat, struct SIMUL_INFO *info);
void issue(struct CONFIG *config, struct RS *rs_ele, int* issued);
void execute(struct RS *rs_ele, struct ROB* rob_ele, struct ROB_ARR *lsq, void** cache_object, struct SIMUL_INFO* info);
void rs_retire(struct RS *rs_ele, struct ROB *rob_ele);
//void lsq_issue_ex_retire(struct RS *rs_ele, struct ROB* rob_ele, struct LSQ *lsq, void** cache_object, struct SIMUL_INFO* info);
void ex_and_issue(struct CONFIG *config, struct ROB_ARR *rob, struct RS_ARR *rs, struct ROB_ARR *lsq, void** cache_object, struct SIMUL_INFO* info);
void commit(struct CONFIG *config, struct ROB_ARR *rob, struct RAT_ARR* rat, struct SIMUL_INFO* info);

void wait(void);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int core_simulator(struct CONFIG *config, struct INST **arr_inst, int* arr_inst_len, int num_of_inst, struct REPORT *out_report)
{
	//출력을 위한 공백 확보
	printf("\n");

	//파일 내 글로벌 초기화
	struct SIMUL_INFO info;
	info.num_of_inst = num_of_inst;
	info.cnt_IntAlu = 0;
	info.cnt_MemRead = 0;
	info.cnt_MemWrite = 0;
	info.cycle = 0;
	info.currnt_fetch = 0;

	// Initializing ...
	int i;
	int j;

	//편하게 사용하기 위해 Thread로 인스트럭션을 패키징함
	struct THREAD threads[] = (struct THREAD*)calloc(num_of_inst, sizeof(struct THREAD));
	for (i = 0; i < num_of_inst; ++i) {
		threads[i] = THREAD_create(arr_inst[i], arr_inst_len[i]);
	}

	// RAT
	struct RAT_ARR rat[] = (struct RAT_ARR*)calloc(num_of_inst,sizeof(struct RAT_ARR));
	for (i = 0; i < num_of_inst; ++i) {
		rat[i]=RAT_create(17); // 0 means no. rat[1] ~ rat[16] are Arch Register entries
	}

	// Fetch Queue
	struct FQ_ARR fq = FQ_create(2 * (*config).Width);

	// Reservation Station
	struct RS_ARR rs = RS_create((*config).RS_size);

	// Load store queue
	struct LSQ_ARR lsq = LSQ_create((*config).RS_size);

	// ReOrdering Buffer
	struct ROB_ARR rob = ROB_create((*config).ROB_size);

	// Cache
	struct cache_config cache_config;
	cache_config.block_size = (config->Width);//need edit
	cache_config.capacity;
	cache_config.set_numbers;
	cache_config.way_numbers;
	void** cache_object = (void**)cache_initializer(&cache_config);

	if (config->Width > 4)
	{
		info.load_blank = info.load_add_blank = 3;//메모리 패스 초기값 설정
		info.save_blank = info.save_add_blank = 2;
	}
	else if (config->Width > 1)
	{
		info.load_blank = info.load_add_blank = 2;//메모리 패스 초기값 설정
		info.save_blank = info.save_add_blank = 1;
	}
	else
	{
		info.load_blank = info.load_add_blank = 1;//메모리 패스 초기값 설정
		info.save_blank = info.save_add_blank = 1;
	}
	//struct cons_cache_controller *cache_cont = cache_object[0];
	//struct cons_cache *cache = cache_object[1];
	//struct statistics *stat = cache_object[2];

	//check all is accesable
	if ((rat == NULL) || (rob.ll.size == 0) || (fq.ca.size == 0) || (rs.size == 0) || (lsq.ll.size = 0 )) {
		return 1;
	}
	for (i = 0; i < num_of_inst; j++) {
		if (rat[j].size == 0){
			return 1;
		}
	}
	
	//check cycle state
	bool still_run = false;
	for (i = 0; i < num_of_inst; i++) {
		//pc가 끝에 도달하지 않았거나 ROB가 아직 남아있으면,
		still_run |= (threads[i].length != threads[i].pc) || (rob.ll.tail == rob.ll.head);
	}
	
	while (still_run);
	{	
		//cycle plus
		info.cycle++;

		//각 명령의 실행 횟수를 초기화한다.
		info.fetch_blank = (fq.ca.size - fq.ca.occupied);//패치큐가 얼마나 비었는지 확인
		info.ROB_blank = (rob.ll.size - rob.ll.occupied);//ROB가 얼마나 비었는지 확인
		info.load_blank = info.load_add_blank;//메모리 패스 빈 공간 확인
		info.save_blank = info.save_add_blank;
		//Loop1
		//ROB를 rob_status.occupied만큼 돌면서 commit/ (ex/issue) 실행
		//다수의 ROB의 최상위 원소중에서 C이고, time이 가장 적은것부터 commit하여 width나 모두 다 닳을때까지 실행 

		commit(config, &rob, rat, &info);
		ex_and_issue(config, &rob, &rs, &lsq, cache_object, &info);
		//issue();

		//Loop2
		//RS를 전부 돌면서 decode / value_feeding , is_completed_this_cycle array 도 초기화.
		decode_and_value_payback(config, &rob, &lsq, &rs,&fq,&rat,&info);

		// Fetch instructions
		fetch(config, &fq, threads, &info);

		// Dump
		int total_len = 0;
		int total_pc = 0;
		switch ((*config).Dump)
		{
		case 0://display current percentage to do;
			for (i = 0; i<info.num_of_inst; ++i){
				total_len += threads[i].length;
				total_pc += threads[i].pc;
			}
			if (info.cycle % 1000 == 0) {
				if (info.cycle == 1) {
					printf("Simulating =");
				}
				printf("%3d%%\b\b\b\b", total_pc *100 / total_len);
			}
			break;
		case 1://rob print
			printf("= Cycle %-5d\n", info.cycle);
			ROB_arr_reporter(&rob);
			break;
		case 2://rob and rs print
			printf("= Cycle %-5d\n", info.cycle);
			RS_arr_reporter(&rs, &rob);
			ROB_arr_reporter(&rob);
			break;
		default://debug mode
			printf("= Cycle %-5d\n", info.cycle);
			FQ_arr_printer(&fq);
			RAT_arr_printer(&rat);
			RS_arr_printer(&rs,&rob);
			ROB_arr_printer(&rob);
			wait();
			break;
		}

		for (i = 0; i < num_of_inst; i++) {
			//pc가 끝에 도달하지 않았거나 ROB가 아직 남아있으면,
			still_run |= (threads[i].length != threads[i].pc) || (rob.ll.tail == rob.ll.head);
		}
	}

	// Simulation finished
	
	//free array
	FQ_delete(fq);
	RS_delete(rs);
	ROB_delete(rob);
	LSQ_delete(lsq);
	for (i = 0; i < info.num_of_inst; ++i) {
		RAT_delete(rat[i]);
	}
	free(rat);

	// Write a report
	out_report->Cycles = info.cycle;
	out_report->Total_Insts = 0;//inst_length;
	out_report->Inst_per_thread = (int*)calloc(info.num_of_inst, sizeof(int));
	for (i = 0; i<info.num_of_inst; ++i) {
		out_report->Total_Insts += threads[i].length;
		out_report->Inst_per_thread[i] = threads[i].length;
	}
	out_report->IntAlu = info.cnt_IntAlu;
	out_report->MemRead = info.cnt_MemRead;
	out_report->MemWrite = info.cnt_MemWrite;
	out_report->IPC = ((double)(out_report->Total_Insts) / (double)info.cycle);

	if (config->Dump == 0) {printf("100%%");}

	// disp Final status
	printf("\n= Final State\n");
	REPORT_reporter(out_report);	// display a report
	printf("\n");
	
	return 0;
}

void fetch(struct CONFIG *config, struct FQ_ARR *fetch_queue, struct THREAD *inst, struct SIMUL_INFO* info)
{

	if (info->currnt_fetch >= 0)
	{//아직 fetch하지 않은 inst가 남아있다면,
		int fetch_num = ((*config).Width > info->fetch_blank) ? info->fetch_blank : (*config).Width;
		int i;
		struct THREAD * current_thread = inst + (info->currnt_fetch);
		struct INST* current_inst;
		struct FQ* fq_ele;
	

		for (i = 0; i < fetch_num; i++)
		{
			//대입
			fq_ele = (fetch_queue->fq) + (ca_next_pos(&(fetch_queue->ca)));
			current_inst = (current_thread->instruction) + (current_thread->pc);

			fq_ele->opcode = current_inst->opcode;
			fq_ele->dest = current_inst->dest;
			fq_ele->oprd_1 = current_inst->oprd_1;
			fq_ele->oprd_2 = current_inst->oprd_2;
			fq_ele->inst_num = info->currnt_fetch;

			//수량 갱신
			ca_cnt_push(&(fetch_queue->ca));
			++(current_thread->pc);

			//다음 current_thread 정하기
			for (int j = 1; j <= info->num_of_inst; ++j) {
				current_thread = inst + (((info->currnt_fetch)+j)%(info->num_of_inst));
				if (current_thread->length != current_thread->pc) {
					break;
				}
			}
			if (current_thread->length == current_thread->pc) {
				info->currnt_fetch = -1;
				break;
			}

		}
	
	}
}

void decode(struct CONFIG *config, struct FQ_ARR *fetch_queue, struct RS *rs_ele, int rs_idx, struct RAT_ARR *rat, struct ROB_ARR *rob, struct ROB_ARR *lsq, int* decoded, struct SIMUL_INFO *info)
{
	// Decode only when: 1) fq is not empty. 2) Upto N instructions. 3) ROB has empty place
	if (fetch_queue->ca.occupied > 0 && *decoded < (*config).Width && *decoded < info->ROB_blank)
	{

		struct FQ * fq_ele;
		fq_ele = (fetch_queue->fq) + (fetch_queue->ca.head); //디코드해올 fq의 inst

		// Count Instruction number
		switch (fq_ele->opcode)
		{
		case 0:
			info->cnt_IntAlu++;
			break;
		case 1:
			info->cnt_MemRead++;
			break;
		case 2:
			info->cnt_MemWrite++;
			break;
		}
		// Putting first element of Fetch Queue to Reservation Station	
		(*rs_ele).is_valid = true;
		(*rs_ele).opcode = fq_ele->opcode;

		int oprd_1 = fq_ele->oprd_1;
		int oprd_2 = fq_ele->oprd_2;
		if (oprd_1 == 0 || rat->rat[oprd_1].RF_valid)
		{
			(*rs_ele).oprd_1.state = V;
		}
		else
		{
			(*rs_ele).oprd_1.state = Q;
			(*rs_ele).oprd_1.data.q = rat->rat[oprd_1].Q;
		}

		if (oprd_2 == 0 || rat->rat[oprd_2].RF_valid)
		{
			(*rs_ele).oprd_2.state = V;
		}
		else
		{
			(*rs_ele).oprd_2.state = Q;
			(*rs_ele).oprd_2.data.q = rat->rat[oprd_2].Q;
		}

		(*rs_ele).time_left = -1; // Waiting to be issued
		
		// Putting first element of Fetch Queue to LSQ
		switch (fq_ele->opcode)
		{
		case 0:
			break;
		case 1:
		case 2:
			lsq->rob[lsq->ll.tail].opcode = fq_ele->opcode;
			rob->rob[rob->ll.tail].rs_dest = rs_idx;
			break;
		}

		// Putting first element of Fetch Queue to ROB
		rob->rob[rob->ll.tail].opcode = fq_ele->opcode;
		rob->rob[rob->ll.tail].dest = fq_ele->dest;
		rob->rob[rob->ll.tail].rs_dest = rs_idx;
		rob->rob[rob->ll.tail].status = P;
		rob->rob[rob->ll.tail].inst_num = fq_ele->inst_num;
		(*rs_ele).rob_dest = rob->ll.tail;

		// Modify RAT status
		if (fq_ele->dest != 0)
		{
			rat->rat[fq_ele->dest].RF_valid = false;		// Now data isn't inside the Arch Register file anymore
			rat->rat[fq_ele->dest].Q = rob->ll.tail;   // So leave reference to ROB entry in RAT
		}

		ll_cnt_push(&(rob->ll)); // Element has been pushed to ROB
		ca_cnt_pop(&(fetch_queue->ca));   // Element has been poped from Fetch Queue

		decoded++;
	}
}

void value_payback(struct RS *rs_ele, struct ROB_ARR *rob)
{
	if (rs_ele->time_left<0)//if it isn't issued
	{
		// Q ->V Promotion
		if ((*rs_ele).oprd_1.state == Q && rob->rob[((*rs_ele).oprd_1.data.q)].status)
		{
			(*rs_ele).oprd_1.state = V;
		}

		if ((*rs_ele).oprd_2.state == Q && rob->rob[((*rs_ele).oprd_2.data.q)].status)
		{
			(*rs_ele).oprd_2.state = V;
		}
	}
}

void decode_and_value_payback(struct CONFIG * config, struct ROB_ARR *rob, struct ROB_ARR *lsq, struct RS_ARR * rs, struct FQ_ARR * fetch_queue, struct RAT_ARR * rat, struct SIMUL_INFO *info)
{
	int decoded = 0;
	// For every entries in Reservation Station,
	for (int i = 0; i < rs->size; i++)
	{
		if (rs->rs[i].is_valid) // Instruction is inside this entry of RS
		{
			value_payback((rs->rs) + i, rob);//check args is ready.
		}
		else// This entry of RS is empty now
		{
			if (false== rs->rs[i].is_completed_this_cycle)//if not this entry flushed this cycle,
			{
				decode(config, fetch_queue, (rs->rs)+i, i, rat, rob, lsq, &decoded, info);// Try to decode instruction into empty place
			}
			else
			{
				rs->rs[i].is_completed_this_cycle = false;
			}
		}
	}

}


void issue(struct CONFIG *config, struct RS *rs_ele, int* issued)
{
	if (*issued < (*config).Width)
	{//아직 width만큼 issue가 되지 않았다면
		if ((*rs_ele).oprd_1.state == V &&
			(*rs_ele).oprd_2.state == V)
		{//issue 조건을 검사하고 만족한다면, issue를 한다.
			(*rs_ele).time_left = 0;
			++(*issued);
		}			
	}
}

void execute(struct RS *rs_ele, struct ROB* rob_ele, struct ROB_ARR *lsq, void** cache_object, struct SIMUL_INFO* info)
{
	//이미 이슈가 최대 N개까지 가능하기 때문에, ex도 최대 N개까지만 수행된다. 검사필요 없음
	//ROB 순서로 호출하므로 자동으로 오래된 것 부터 수행한다.

	if (rs_ele->time_left == 0)
	{//만약 실행 대기중이라면, 
		
		if (rs_ele->opcode = IntAlu) {//alu 펑션이라면 실행하고 RS를 비운 다음 ROB를 C 상태로 바꾼다.
			rs_retire(rs_ele, rob_ele);
		}
		else {//alu 펑션이 아니라면 메모리 주소가 끝났으니 lsq의 작업을 한다.
			lsq_issue(rs_ele, rob_ele, lsq, cache_object, info);
		}
		rs_ele->is_completed_this_cycle = true;

	}
	else
	{//만약 issue가 되어있다면,
		--(rs_ele->time_left);//ALU에 넣는다 ( 실행 대기 단계 ).
	}

}

void ex_and_issue(struct CONFIG *config, struct ROB_ARR *rob, struct RS_ARR *rs, struct ROB_ARR *lsq, void** cache_object, struct SIMUL_INFO* info)
{
	int i;

	struct ROB* rob_ptr;
	int rob_ptr_idx;
	struct RS  * rs_ele;

	int issued = 0;

	//start from head
	rob_ptr_idx = rob->ll.head;
	rob_ptr = (rob->rob) + (rob_ptr_idx);
	
	for (i = 0; i < (rob->ll.occupied); ++i)
	{//모든 원소에 대해, 검사한다

		if (rob_ptr->status == P)
		{//만약 상태가 P라면, 이는 issue의 대상과 ex의 대상을 포함한다.
			
			rs_ele = (rs->rs) + (rob_ptr->rs_dest);

			if (rs_ele->time_left >= 0)
			{//만약 Issue 되었다면
				execute(rs_ele, rob_ptr, lsq, cache_object, info);
				//rs와 lsq의 실행 및 실행 완료한다.
			}
			else
			{//만약 Issue 안되었다면
				issue(config, rs_ele, &issued);
				//이슈 조건을 검사하고 부합할 경우 이슈한다.
			}

		}

		//다음 원소로 포인터를 이동한다
		rob_ptr_idx = ll_next_pos(&(rob->ll), rob_ptr_idx);
		rob_ptr = (rob->rob) + (rob_ptr_idx);

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
/*
void lsq_issue_ex_retire(struct RS *rs_ele, struct ROB* rob_ele, struct LSQ_ARR *lsq, void** cache_object, struct SIMUL_INFO* info)//info를 이용해 통신하도록 해보자
{
	struct cons_cache_controller *cache_cont = cache_object[0];
	struct cons_cache *cache = cache_object[1];
	struct statistics *stat = cache_object[2];
	struct LSQ* lsq_ele = (lsq->lsq)+(rs_ele->lsq_dest);

	if (lsq_ele->time == 0)//실행이 완료되었으면, 커밋 과정을 수행한다.
	{
		rs_retire(rs_ele, rob_ele);
		ll_cnt_pop(&(lsq->ll), rs_ele->lsq_dest);

		//캐시 업데이트도 수행한다
		cache_filler(cache_cont, cache, stat, lsq_ele->address)

		//커밋 시 빈공간이 났음을 저장한다
		if (rs_ele->opcode == MemRead)
		{
			++(info->load_add_blank);
		}
		else
		{
			++(info->save_add_blank);
		}
	}
	else if (lsq_ele->time>0)//실행중이면 대기시간을 1사이클 줄인다(불러오기)
	{
		--(lsq_ele->time);
	}
	else //아직 이슈가 안되었으면
	{
		if (rs_ele->opcode == MemRead && 0 < info->load_blank) {
			//Load시 캐시 hit이 났을 때 2사이클, miss시 52사이클
			//위의 준비 안된 SAVE가 있을 시 작업을 수행하지 않는다.
			struct LSQ* lsq_ptr = (lsq->lsq) + (rs_ele->lsq_dest);
			int lsq_ptr_idx = (rs_ele->lsq_dest);
			while (true)
			{//한단계씩 전 명령을 검사한다
				if (lsq_ptr->opcode == MemWrite)
				{//write에 대해서만 검사한다.
					if (lsq_ptr->address < 0)
					{//알수 없는 주소의 save이므로 load를 하지 않는다.
						break;
					}
					else if (lsq_ptr->address == rs_ele->oprd_2.data.v)
					{//같은 주소의 세이브가 있으므로 feed-forward.
						lsq_ele->address = rs_ele->oprd_2.data.v;
						lsq_ele->time = 0;

						--(info->load_add_blank);
						--(info->load_blank);
					}
				}
				if (lsq_ptr_idx == lsq->ll.head)
				{//위의 조건이 모두 해당되지 않을 시, 그냥 로드하면 된다.
					lsq_ele->address = rs_ele->oprd_2.data.v;
					lsq_ele->time = cache_query(cache_cont, cache, stat, READ, lsq_ele->address) ? 0 : 51;

					--(info->load_add_blank);
					--(info->load_blank);
				}

				lsq_ptr_idx = lsq->ll.prev[lsq_ptr_idx];
				lsq_ptr = (lsq->lsq) + (lsq_ptr_idx);
			}

		}
		else if (rs_ele->opcode == MemRead && 0 <info->save_blank) {
			//write시 캐시 hit이면 즉시 commit, miss시 51사이클
			--(info->save_blank);
			lsq_ele->address = rs_ele->oprd_2.data.v;
			if (cache_query(cache_cont, cache, stat, WRITE, lsq_ele->address))
			{
				rs_retire(rs_ele, rob_ele);
				ll_cnt_pop(&(lsq->ll), rs_ele->lsq_dest);
			}
			else
			{
				--(info->load_add_blank);
				lsq_ele->time = 50;
			}
		}
	}
}
*/
void commit(struct CONFIG *config, struct ROB_ARR *rob, struct RAT_ARR* rat, struct SIMUL_INFO* info)
{
	int i;//iter
	struct ROB* rob_ptr;//for easier code
	int rob_ptr_idx;
	int remain_of_search = (*config).Width;// Only permits upto N commits

	//그 스레드의 커밋이 끝났는지 아직 계속할 수 있는 지 체크
	bool commit_done = false;
	bool* thread_commit = (bool*)calloc(info->num_of_inst,sizeof(bool));
	for (i = 1; i < info->num_of_inst; ++i) { thread_commit[i] = true; }

	//start from head
	rob_ptr_idx = rob->ll.head;
	rob_ptr = (rob->rob)+(rob_ptr_idx);
	do{

		if (thread_commit[rob_ptr->inst_num])
		{//아직 해당 inst의 커밋이 끝나지 않았다면,
			switch (rob_ptr->status)
			{
			case C:
				rat[rob_ptr->inst_num].rat[rob_ptr->dest].RF_valid = true;
				ll_cnt_pop(&(rob->ll), rob_ptr_idx);
				--remain_of_search;
				break;
			case P:
				thread_commit[rob_ptr->inst_num] = false;
				
				//inst 커밋이 하나 끝날 때마다 전체 inst 커밋이 끝났는지 검사
				commit_done = true;
				for (i = 1; i < info->num_of_inst; ++i) {
					commit_done &= (!thread_commit[i]);
				}
			}
		}

		//다음 ROB로 포인터를 옮긴다
		rob_ptr_idx = ll_next_pos(&(rob->ll), rob_ptr_idx);
		rob_ptr = (rob->rob) + (rob_ptr_idx);

		//종료 조건 검사
		if (commit_done || //커밋이 끝났거나
			remain_of_search == 0 || //이미 width만큼 커밋했거나
			rob_ptr_idx == rob->ll.head) //이미 전부 검사했다면
		{
			break;
		}
			
	} while (true);
}

struct SIMUL_INFO
{
	int num_of_inst;

	int cycle;
	int cnt_IntAlu;
	int cnt_MemRead;
	int cnt_MemWrite;

	int fetch_blank;
	int ROB_blank;
	int load_blank;
	int save_blank;
	int load_add_blank;
	int save_add_blank;
	int currnt_fetch;
};