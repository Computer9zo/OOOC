#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "data_structures.h"
#include "cache.h"


int core_simulator(struct CONFIG *config, struct THREAD* threads, int thread_num, struct REPORT *out_report);

struct cons_remaining
{
	int remain;//현재 남은 빈 횟수 
	int width;//한 사이클에 최대한 수행할 수 있는 양
};

struct simulator_data
{
	struct con_core
	{
		//core
		struct FQ_ARR* fq;//패치큐
		struct RS_ARR* rs;//레서브 스테이션
		struct LSQ_ARR* lsq;//로드 스토어 큐
		struct ROB_ARR* rob;//리오더 버퍼
		struct RAT_ARR* rat;//레지스터 아키텍쳐 테이블
		
		struct core_info
		{
			// 남은 횟수 width나 빈공간중 작은거. 남은 횟수가 쓰인 만큼 ->빈공간이 준다., 작업이 완료된 만큼 빈공간이 는다
			struct cons_remaining fq, rob, lsq, load, write;
			int current_fetch_inst;
		} info;
	} core;
	struct con_cache
	{
		struct cons_cache_controller *cont;
		struct cons_cache *cache;
		struct statistics *stat;
		bool is_perfect_cache;
	} cache;
	struct const_simul_info
	{
		int num_of_inst;

		int Total_Insts;
		int*Inst_per_thread;

		int cycle;

		int cnt_Insts;//sum of under 3
		int cnt_IntAlu;
		int cnt_MemRead;
		int cnt_MemWrite;
	}info;
};

void lsq_issue(struct simulator_data *simul);

void lsq_exe(struct simulator_data *simul);

int simulator_initialize(struct CONFIG *config, struct THREAD* threads, int thread_num, struct simulator_data* out_simulator);

void simul_free(struct simulator_data* out_simulator) 
{
	
	FQ_delete(*out_simulator->core.fq);
	RS_delete(*out_simulator->core.rs);
	ROB_delete(*out_simulator->core.rob);
	LSQ_delete(*out_simulator->core.lsq);
	for (int i = 0; i < out_simulator->info.num_of_inst; ++i) {
		RAT_delete(out_simulator->core.rat[i]);
	}
	free(out_simulator->core.rat);
}

void write_report(struct simulator_data* out_simulator, struct REPORT* out_report)
{
	out_report->Cycles = out_simulator->info.cycle;
	out_report->Total_Insts += out_simulator->info.Total_Insts;
	out_report->Inst_per_thread = (int*)calloc(out_simulator->info.num_of_inst, sizeof(int));
	for (int i = 0; i <  out_simulator->info.num_of_inst; ++i) {
		
		out_report->Inst_per_thread[i] = out_simulator->info.Inst_per_thread[i];
	}
	out_report->IntAlu = out_simulator->info.cnt_IntAlu;
	out_report->MemRead = out_simulator->info.cnt_MemRead;
	out_report->MemWrite = out_simulator->info.cnt_MemWrite;
	out_report->IPC = ((double)(out_report->Total_Insts) / (double)out_report->Cycles);
}

bool is_work_left(struct THREAD* threads, struct simulator_data* simulator);

void remains_update(struct simulator_data* simulator);

void commit(struct simulator_data* simul);
void ex_and_issue(struct simulator_data* simulator);
void decode_and_value_payback(struct simulator_data* simul);
void fetch(struct THREAD *inst, struct simulator_data* simul);
void issue(struct RS *rs_ele, int* issue_remain);
void execute(struct RS *rs_ele, struct ROB* rob_ele, struct LSQ_ARR *lsq_arr, struct simulator_data* simul);
void rs_retire(struct RS *rs_ele, struct ROB *rob_ele);
void decode(struct RS *rs_ele, int rs_idx, struct simulator_data* simul, int *decoded_remain);
void value_payback(struct RS *rs_ele, struct ROB_ARR *rob);
void disp_debug(struct simulator_data* simul,int dump);

void wait(void);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int core_simulator(struct CONFIG *config, struct THREAD* threads, int thread_num, struct REPORT *out_report)
{
	
	//simulator init
	struct simulator_data simul_data;

	if (1 == simulator_initialize(config, threads, thread_num, &simul_data))
	{//init failed!
		return 1;
	}

	
	//check cycle state

	
	while (is_work_left(threads, &simul_data))
	{	
		//TODO: Check here ! Core logic
		//cycle plus
		++(simul_data.info.cycle);

		//각 명령의 실행 횟수를 초기화한다.
		remains_update(&simul_data);
		printf("A");
		//Loop1
		//ROB를 rob_status.occupied만큼 돌면서 commit/ (ex/issue) 실행
		//다수의 ROB의 최상위 원소중에서 C이고, time이 가장 적은것부터 commit하여 width나 모두 다 닳을때까지 실행 

		commit(&simul_data);
		ex_and_issue(&simul_data);
		printf("t");
		lsq_exe(&simul_data);
		lsq_issue(&simul_data);
		printf("t");
		
		

		//Loop2
		//RS를 전부 돌면서 decode / value_feeding , is_completed_this_cycle array 도 초기화.
		decode_and_value_payback(&simul_data);
		// Fetch instructions
		fetch(threads,&simul_data);
		printf("t");
		// Dump
		disp_debug(&simul_data,(*config).Dump);
		printf("t");
	}

	// Simulation finished
	
	//free array
	simul_free(&simul_data);

	// Write a report
	write_report(&simul_data, out_report);

	if (config->Dump == 0) {printf("100%%");}

	// disp Final status
	printf("\n= Final State\n");
	REPORT_reporter(out_report);	// display a report
	printf("\n");
	
	return 0;
}

int simulator_initialize(struct CONFIG *config, struct THREAD* threads, int thread_num, struct simulator_data* out_simulator)
{
	//simulator_data.info inits
	out_simulator->info.num_of_inst = thread_num;

	out_simulator->info.Total_Insts = 0;
	out_simulator->info.Inst_per_thread = (int*)calloc(out_simulator->info.num_of_inst,sizeof(int));
	if (out_simulator->info.Inst_per_thread == NULL) 
	{//malloc error
		return 1;
	}
	for (int i = 0; i < out_simulator->info.num_of_inst; ++i)
	{
		out_simulator->info.Total_Insts       += threads[i].length;
		out_simulator->info.Inst_per_thread[i] = threads[i].length;
	}
	
	out_simulator->info.cycle = 0;

	out_simulator->info.cnt_Insts = 0;
	out_simulator->info.cnt_IntAlu = 0;
	out_simulator->info.cnt_MemRead = 0;
	out_simulator->info.cnt_MemWrite = 0;
	
	//simulator_data.core init
	out_simulator->core.fq = (struct FQ_ARR*)calloc(1, sizeof(struct FQ_ARR));
	if (out_simulator->core.fq == NULL)
	{//malloc error
		return 1;
	}
	(*out_simulator->core.fq) = FQ_create(2 * (*config).Width);
	if (out_simulator->core.fq->ca.size == 0)
	{//malloc error
		return 1;
	}

	out_simulator->core.rs = (struct RS_ARR*)calloc(1, sizeof(struct RS_ARR));
	if (out_simulator->core.rs == NULL)
	{//malloc error
		return 1;
	}
	(*out_simulator->core.rs) = RS_create((*config).RS_size);
	if (out_simulator->core.rs->size == 0)
	{//malloc error
		return 1;
	}

	out_simulator->core.lsq = (struct LSQ_ARR*)calloc(1, sizeof(struct LSQ_ARR));
	if (out_simulator->core.lsq == NULL)
	{//malloc error
		return 1;
	}
	(*out_simulator->core.lsq) = LSQ_create((*config).LSQ_size);
	if (out_simulator->core.lsq->ll.size == 0)
	{//malloc error
		return 1;
	}

	out_simulator->core.rob = (struct ROB_ARR*)calloc(1, sizeof(struct ROB_ARR));
	if (out_simulator->core.rob == NULL)
	{//malloc error
		return 1;
	}
	(*out_simulator->core.rob) = ROB_create((*config).ROB_size);
	if (out_simulator->core.rob->ll.size == 0)
	{//malloc error
		return 1;
	}

	out_simulator->core.rat = (struct RAT_ARR*)calloc(out_simulator->info.num_of_inst, sizeof(struct RAT_ARR));
	if (out_simulator->core.rat == NULL)
	{//malloc error
		return 1;
	}
	for (int i = 0; i < out_simulator->info.num_of_inst; ++i) 
	{
		out_simulator->core.rat[i] = RAT_create(17); // 0 means no. rat[1] ~ rat[16] are Arch Register entries
		if (out_simulator->core.rat->size == 0)
		{//malloc error
			return 1;
		}
	}

	out_simulator->core.info.fq.width = (*config).Width;
	out_simulator->core.info.fq.remain = -1;//it not init in this time. it will init start of every cycle

	out_simulator->core.info.rob.width = (*config).Width;
	out_simulator->core.info.rob.remain = -1;//it not init in this time. it will init start of every cycle

	out_simulator->core.info.lsq.width = (*config).Width;
	out_simulator->core.info.lsq.remain = -1;//it not init in this time. it will init start of every cycle

	if (config->Width > 4)
	{
		out_simulator->core.info.load.width = 3;//메모리 패스 초기값 설정
		out_simulator->core.info.write.width = 2;
	}
	else if (config->Width > 1)
	{
		out_simulator->core.info.load.width = 2;//메모리 패스 초기값 설정
		out_simulator->core.info.write.width = 1;
	}
	else
	{
		out_simulator->core.info.load.width = 1;//메모리 패스 초기값 설정
		out_simulator->core.info.write.width = 1;
	}
	out_simulator->core.info.load.remain = -1;//it not init in this time. it will init start of every cycle
	out_simulator->core.info.write.remain = -1;//it not init in this time. it will init start of every cycle

	out_simulator->core.info.current_fetch_inst = 0;

	// Cache
	if (config->Cache_size <= 0)
	{//it mean perfect cache mode.
		out_simulator->cache.is_perfect_cache = true;
		out_simulator->cache.cache = NULL;
		out_simulator->cache.cont = NULL;
		out_simulator->cache.stat = NULL;
	}
	else
	{
		out_simulator->cache.is_perfect_cache = false;

		struct cache_config cache_config;
		cache_config.block_size = 32;//need edit
		cache_config.capacity = (config->Cache_size)*1024;
		cache_config.way_numbers = 8;
		cache_config.set_numbers = cache_config.capacity/cache_config.block_size/cache_config.way_numbers;
		
		void** cache_object = (void**)cache_initializer(&cache_config);
		if (cache_object == NULL)
		{//malloc error
			return 1;
		}
		out_simulator->cache.cache = cache_object[1];
		out_simulator->cache.cont = cache_object[0];
		out_simulator->cache.stat = cache_object[2];
		if (out_simulator->cache.cache == NULL || out_simulator->cache.cont == NULL || out_simulator->cache.stat == NULL)
		{//malloc error
			return 1;
		}
		free(cache_object);
	}
	return 0;
}

bool is_work_left(struct THREAD* threads, struct simulator_data* simulator)
{
	bool is_work_left = false;
	for (int i = 0; i < simulator->info.num_of_inst; i++) {
		//pc가 끝에 도달하지 않았거나 ROB가 아직 남아있으면,
		if ((threads[i].length != threads[i].pc) || ((simulator->core.rob)->ll.tail != (simulator->core.rob)->ll.head))
		{
			is_work_left = true;
			break;
		}
	}
	return is_work_left;
}

void remains_update(struct simulator_data* simulator)
{
	simulator->core.info.fq.remain =
		(simulator->core.fq->ca.size - simulator->core.fq->ca.occupied) < simulator->core.info.fq.width ?
		(simulator->core.fq->ca.size - simulator->core.fq->ca.occupied) : simulator->core.info.fq.width;
	simulator->core.info.rob.remain =
		(simulator->core.rob->ll.size - simulator->core.rob->ll.occupied) < simulator->core.info.rob.width ?
		(simulator->core.rob->ll.size - simulator->core.rob->ll.occupied) : simulator->core.info.rob.width;
	simulator->core.info.lsq.remain =
		(simulator->core.lsq->ll.size - simulator->core.lsq->ll.occupied) < simulator->core.info.lsq.width ?
		(simulator->core.lsq->ll.size - simulator->core.lsq->ll.occupied) : simulator->core.info.lsq.width;

}

void fetch(struct THREAD *inst, struct simulator_data* simul)
{

	if (simul->core.info.current_fetch_inst >= 0)
	{//아직 fetch하지 않은 inst가 남아있다면,
		
		struct THREAD * current_thread = inst + (simul->core.info.current_fetch_inst);
		struct INST* current_inst;
		struct FQ* fq_ele;
	

		while ( simul->core.info.fq.remain>0 )
		{
			//대입
			fq_ele = (simul->core.fq->fq) + (CA_next_pos(&(simul->core.fq->ca)));
			current_inst = (current_thread->instruction) + (current_thread->pc);

			fq_ele->opcode = current_inst->opcode;
			fq_ele->dest = current_inst->dest;
			fq_ele->oprd_1 = current_inst->oprd_1;
			fq_ele->oprd_2 = current_inst->oprd_2;
			fq_ele->inst_num = (simul->core.info.current_fetch_inst);

			//수량 갱신
			CA_cnt_push(&simul->core.fq->ca);
			--(simul->core.info.fq.remain);
			++(current_thread->pc);

			//다음 current_thread 정하기
			for (int j = 1; j <= simul->info.num_of_inst; ++j) {
				current_thread = inst + (((simul->core.info.current_fetch_inst)+j)%(simul->info.num_of_inst));
				if (current_thread->length != current_thread->pc) {
					break;
				}
			}
			if (current_thread->length == current_thread->pc) {
				(simul->core.info.current_fetch_inst) = -1;
				break;
			}

		}
	
	}
}

void decode(struct RS *rs_ele, int rs_idx, struct simulator_data* simul, int *decoded_remain)
{
	// Decode only when: 1) fq is not empty. 2) Upto N instructions. 3) rob h
	if (simul->core.fq->ca.occupied > 0 && *decoded_remain > 0 && simul->core.info.rob.remain>0)
	{
		struct FQ * fq_ele;
		fq_ele = (simul->core.fq->fq) + (simul->core.fq->ca.head); //디코드해올 fq의 inst

		if (fq_ele->opcode == IntAlu || simul->core.info.lsq.remain > 0)
		{//int alu가 아니라면, lsq에 여유가 있는지를 검사한다.

		 // Element has been poped from Fetch Queue
			CA_cnt_pop(&simul->core.fq->ca);

			// Count Instruction number
			++(simul->info.cnt_Insts);
			switch (fq_ele->opcode)
			{
			case 0:
				++(simul->info.cnt_IntAlu);
				break;
			case 1:
				++(simul->info.cnt_MemRead);
				break;
			case 2:
				++(simul->info.cnt_MemWrite);
				break;
			}

			//init;
			struct RAT_ARR* rat = simul->core.rat;
			struct ROB_ARR* rob = simul->core.rob;
			// struct RS_ARR* rs = simul->core.rs;  UNUSED
			// struct FQ_ARR* fq = simul->core.fq;  UNUSED
			struct LSQ_ARR*lsq = simul->core.lsq;

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

			if (oprd_2 == 0 || fq_ele->opcode != IntAlu|| rat->rat[oprd_2].RF_valid)
			{
				(*rs_ele).oprd_2.state = V;
				(*rs_ele).oprd_2.data.v = oprd_2;
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
				lsq->lsq[lsq->ll.tail].opcode = fq_ele->opcode;
				lsq->lsq[lsq->ll.tail].address = -1;
				lsq->lsq[lsq->ll.tail].rob_dest = rob->ll.tail;
				lsq->lsq[lsq->ll.tail].time = -1;
				lsq->lsq[lsq->ll.tail].status = P;
				rs_ele->lsq_dest = lsq->ll.tail;
				LL_cnt_push(&simul->core.lsq->ll);
				--(simul->core.info.lsq.remain);
				break;
			}

			// Putting first element of Fetch Queue to ROB
			rob->rob[rob->ll.tail].opcode = fq_ele->opcode;
			rob->rob[rob->ll.tail].dest = fq_ele->dest;
			rob->rob[rob->ll.tail].rs_dest = rs_idx;
			rob->rob[rob->ll.tail].status = P;
			rob->rob[rob->ll.tail].inst_num = fq_ele->inst_num;

			//TODO: Check is this right?
			rob->rob[rob->ll.tail].lsq_source = lsq->ll.tail;

			(*rs_ele).rob_dest = rob->ll.tail;
			LL_cnt_push(&simul->core.rob->ll);
			--(simul->core.info.rob.remain);

			// Modify RAT status
			if (fq_ele->dest != 0)
			{
				rat->rat[fq_ele->dest].RF_valid = false;		// Now data isn't inside the Arch Register file anymore
				rat->rat[fq_ele->dest].Q = rob->ll.tail;   // So leave reference to ROB entry in RAT
			}


			--(*decoded_remain);
		}
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

void decode_and_value_payback(struct simulator_data* simul)
{
	struct RS_ARR* rs = simul->core.rs;
	struct ROB_ARR* rob = simul->core.rob;

	//struct CONFIG * config, struct ROB_ARR *rob, struct ROB_ARR *lsq, struct RS_ARR * rs, struct FQ_ARR * fetch_queue, struct RAT_ARR * rat, struct SIMUL_INFO *info
	int decoded = simul->core.info.rob.width;
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
				decode( (rs->rs)+i, i, simul, &decoded);// Try to decode instruction into empty place
			}
			else
			{
				rs->rs[i].is_completed_this_cycle = false;
			}
		}
	}

}


void issue(struct RS *rs_ele, int* issue_remain)
{
	if (*issue_remain > 0)
	{
		if ((*rs_ele).oprd_1.state == V &&
			(*rs_ele).oprd_2.state == V)
		{//issue 조건을 검사하고 만족한다면, issue를 한다.
			(*rs_ele).time_left = 0;
			--(*issue_remain);
		}
	}
}

// TODO: LSQ 관련 함수들 만들
void lsq_issue(struct simulator_data *simul)
{
	struct LSQ_ARR *lsq_arr = simul->core.lsq;
	struct ROB_ARR *rob_arr = simul->core.rob;

	int read_port = simul->core.info.load.width;
	int write_port = simul->core.info.write.width;

	struct cons_cache_controller *cache_cont = (*simul).cache.cont;
	struct cons_cache *cache = (*simul).cache.cache;
	struct statistics *stat = (*simul).cache.stat;
	bool is_perfect_cache = (*simul).cache.is_perfect_cache;

	// LSQ_ARR를 돌면서 issue를 한다
	// Time count를 -1에서 초기화시킴.
	int i = 0;
	int addr;
	int op;
	int time;

	bool are_there_dangerous_stores = false;

	int* older_stores = NULL;
	if (lsq_arr->ll.occupied > 0)
	{
		older_stores = calloc(lsq_arr->ll.occupied, sizeof(int));
	}
		//[lsq_arr[0].ll.occupied];	
	int older_stores_num = 0;
	//printf("te");
	
	struct LSQ *lsq_ptr;
	int lsq_ptr_idx;

	lsq_ptr_idx = (*lsq_arr).ll.head;
	lsq_ptr = (*lsq_arr).lsq + lsq_ptr_idx;
	if (is_perfect_cache)
	{
		while ((read_port > 0 || write_port > 0) && (i < (*lsq_arr).ll.occupied))
		{
			// op = (*lsq_arr[i].lsq).opcode;
			op = (*lsq_ptr).opcode;
			// addr = (*lsq_arr[i].lsq).address;
			addr = (*lsq_ptr).address;
			// time = (*lsq_arr[i].lsq).time;
			time = (*lsq_ptr).time;
			
			if (op == MemRead) // MemRead case
			{
				if (time == -1 && addr != -1 && !are_there_dangerous_stores) // Address has been calculated and there are no older stores with unknown addresses
				{
					cache_query(cache_cont, cache, stat, op, addr);
					// (*lsq_arr[i].lsq).time = 2;
					(*lsq_ptr).time = 2;
					// (*lsq_arr[i].lsq).was_hit = HIT;
					(*lsq_ptr).was_hit = HIT;
					read_port--;
					i++;
				}
				else // No address yet or there are dangerous stores
				{
					i++;
				}
			}
			else // MemWrite case
			{
				if (addr != -1) // Address has been calculated
				{
					cache_query(cache_cont, cache, stat, op, addr);
					// (*lsq_arr[i].lsq).time = 2;
					(*lsq_ptr).time = 2;
					// (*lsq_arr[i].lsq).was_hit = HIT;
					(*lsq_ptr).was_hit = HIT;
					// (*rob_arr[(*lsq_arr[i].lsq).rob_dest].rob).status = C;
					(*rob_arr).rob[(*lsq_ptr).rob_dest].status = C;
					write_port--;
					i++;
				}
				else if (time != -1) // Already issued
				{
					i++;
				}
				else // No address yet
				{
					are_there_dangerous_stores = true;
					i++;
				}
			}

			lsq_ptr_idx = LL_next_pos(&(*lsq_arr).ll, lsq_ptr_idx);
			lsq_ptr = (*lsq_arr).lsq + (lsq_ptr_idx);
					
		}
	}
	else // Not perfect cache
	{
		bool is_hit;
		while ((read_port > 0 || write_port > 0) && (i < (*lsq_arr).ll.occupied))
		{
			// op = (*lsq_arr[i].lsq).opcode;
			op = (*lsq_ptr).opcode;
			// addr = (*lsq_arr[i].lsq).address;
			addr = (*lsq_ptr).address;
			// time = (*lsq_arr[i].lsq).time;
			time = (*lsq_ptr).time;

			if (op == MemRead) // MemRead case
			{
				if (time == -1 && addr != -1 && !are_there_dangerous_stores) // Address has been calculated and there are no older stores with unknown addresses
				{
					is_hit = cache_query(cache_cont, cache, stat, op, addr);
					if (is_hit)
					{
						// (*lsq_arr[i].lsq).time = 2;
						(*lsq_ptr).time = 2;
						// (*lsq_arr[i].lsq).was_hit = HIT;
						(*lsq_ptr).was_hit = HIT;
					}
					else
					{
						// (*lsq_arr[i].lsq).time = 52;
						(*lsq_ptr).time = 52;
						// (*lsq_arr[i].lsq).was_hit = MISS;
						(*lsq_ptr).was_hit = MISS;
						int j;
						for (j = 0; j < older_stores_num; j++)
						{
							if ((*lsq_ptr).address == older_stores[j])
							{
								(*lsq_ptr).time = 2;
								(*lsq_ptr).was_hit = FORWARD;
								break;
							}
						}
					}
					
					read_port--;
					i++;
				}
				else // No address yet or there are dangerous stores
				{
					i++;
				}
			}
			else // MemWrite case
			{
				if (time == -1 && addr != -1) // Address has been calculated
				{
					is_hit = cache_query(cache_cont, cache, stat, op, addr);
					if (is_hit)
					{
						// (*lsq_arr[i].lsq).time = 2;
						(*lsq_ptr).time = 2;
						// (*lsq_arr[i].lsq).was_hit = HIT;
						(*lsq_ptr).was_hit = HIT;
					}
					else
					{
						 // (*lsq_arr[i].lsq).time = 52;
						 (*lsq_ptr).time = 52;
						// (*lsq_arr[i].lsq).was_hit = MISS;
						(*lsq_ptr).was_hit = MISS;
					}

					(*rob_arr).rob[(*lsq_ptr).rob_dest].status = C;

					older_stores[older_stores_num] = (*lsq_ptr).address;
					older_stores_num++;

					write_port--;
					i++;
				}
				else if (time != -1) // Already issued
				{
					older_stores[older_stores_num] = (*lsq_ptr).address;
					older_stores_num++;
					i++;
				}
				else // No address yet
				{
					are_there_dangerous_stores = true;
					i++;
				}
			}

			lsq_ptr_idx = LL_next_pos(&(*lsq_arr).ll, lsq_ptr_idx);
			lsq_ptr = (*lsq_arr).lsq + (lsq_ptr_idx);
					
		}
	}	
	free(older_stores);
	//printf("te");
}

void lsq_exe(struct simulator_data *simul)
{
	struct LSQ_ARR *lsq_arr = simul->core.lsq;
	struct ROB_ARR *rob_arr = simul->core.rob;

	// If time == 0, change the status of corresponding entry in ROB to C
	struct cons_cache_controller *cache_cont = (*simul).cache.cont;
	struct cons_cache *cache = (*simul).cache.cache;
	struct statistics *stat = (*simul).cache.stat;

	int i;
	int lsq_occupied = lsq_arr[0].ll.occupied;

	int* older_stores = NULL;
	if (lsq_arr->ll.occupied > 0)
	{
		older_stores = calloc(lsq_arr->ll.occupied, sizeof(int));
	}
	int older_stores_num = 0;

	int lsq_ptr_idx = (*lsq_arr).ll.head;
	struct LSQ *lsq_ptr = (*lsq_arr).lsq + (lsq_ptr_idx);
	for (i = 0; i < lsq_occupied; i++)
	{	
		if (((*lsq_ptr).opcode == MemWrite) && (*lsq_ptr).address != -1)
		{
			older_stores[older_stores_num] = (*lsq_ptr).address;
			older_stores_num++;
		}

		if (((*lsq_ptr).time == 0) && ((*lsq_ptr).status == P)) // Cache access completed and never been commited to ROB
		{
			if ((*lsq_ptr).opcode == MemRead) // MemRead case
			{
				if ((*lsq_ptr).was_hit == HIT)
				{
					cache_reader(cache_cont, cache, (*lsq_ptr).address);
				}
				else if ((*lsq_ptr).was_hit == MISS)
				{
					cache_filler(cache_cont, cache, stat, (*lsq_ptr).address);
					cache_reader(cache_cont, cache, (*lsq_ptr).address);
				}
				
				(*lsq_ptr).status = C;
				(*rob_arr).rob[(*lsq_ptr).rob_dest].status = C; // Completed
			}
			else if ((*lsq_ptr).opcode == MemWrite)
			{
				// Actual cache / memory access of MemWrite takes place when this instruction is vaporized from LSQ. !!	
				(*lsq_ptr).status = C;
				(*rob_arr).rob[(*lsq_ptr).rob_dest].status = C;
			}
		}
		else if ((*lsq_ptr).time > 0) // Still executing
		{
			if (((*lsq_ptr).opcode == MemRead) && ((*lsq_ptr).time > 2)) // Forwarding
			{
				int j;
				for (j = 0; j < older_stores_num; j++)
				{
					if ((*lsq_ptr).address == older_stores[j])
					{
						(*lsq_ptr).time = 2;
						break;
					}
				}
			}

			(*lsq_ptr).time--;
		}

		lsq_ptr_idx = LL_next_pos(&(*lsq_arr).ll, lsq_ptr_idx);
		lsq_ptr = (*lsq_arr).lsq + (lsq_ptr_idx);
	}

	free(older_stores);

}


void execute(struct RS *rs_ele, struct ROB* rob_ele, struct LSQ_ARR *lsq_arr, struct simulator_data* simul)
{
	//이미 이슈가 최대 N개까지 가능하기 때문에, ex도 최대 N개까지만 수행된다. 검사필요 없음
	//ROB 순서로 호출하므로 자동으로 오래된 것 부터 수행한다.

	//if (rs_ele->time_left == 0)
	//{//만약 실행 대기중이라면, 
		
		if ((rs_ele->opcode == IntAlu))
		{//load나 store가 아니라면 retire의 작업을 한다. RS를 비운 다음 ROB를 C 상태로 바꾼다.
			rs_retire(rs_ele, rob_ele);
		}
		// TODO: 아래 두 케이스들을 체크 바람. 
		// SOME MISS, i fix it.
		else if (((*rs_ele).opcode == MemRead)) // MemRead
		{//Load 라면 LSQ의 대항 entry에다 주소를 주고 retire 한다
			(*lsq_arr).lsq[(*rs_ele).lsq_dest].address = (*rs_ele).oprd_2.data.v;
			rs_retire(rs_ele, rob_ele);
			(*rob_ele).status = P;
		}
		else // MemWrite
		{//Store 라면 ROB의 해당 entry를 C로 만들고 LSQ에서 해당 entry에  주소를 할당한다
			(*lsq_arr).lsq[(*rs_ele).lsq_dest].address = (*rs_ele).oprd_2.data.v;
			rs_retire(rs_ele, rob_ele);

		}
		rs_ele->is_completed_this_cycle = true;

	//}
	//else
	//{//만약 issue가 되어있다면,
	//	--(rs_ele->time_left);//ALU에 넣는다 ( 실행 대기 단계 ).
	//}

}

void ex_and_issue(struct simulator_data* simul)
{
	int i;

	struct ROB_ARR* rob = simul->core.rob;
	struct RS_ARR* rs = simul->core.rs;
	struct LSQ_ARR* lsq = simul->core.lsq;
	struct ROB* rob_ptr;
	int rob_ptr_idx;
	struct RS  * rs_ele;

	int issue_remain = simul->core.info.rob.width;

	//start from head
	rob_ptr_idx = rob->ll.head;
	rob_ptr = (rob->rob) + (rob_ptr_idx);
	
	for (i = 0; i < (rob->ll.occupied); ++i)
	{//모든 원소에 대해, 검사한다

		if (rob_ptr->status == P)
		{//만약 상태가 P라면, 이는 issue의 대상과 ex의 대상을 포함한다.
			
			rs_ele = (rs->rs) + (rob_ptr->rs_dest);

			if (rs_ele->time_left == 0)
			{//만약 Issue 되었다면
				execute(rs_ele, rob_ptr, lsq, simul);
				//실행하고 완료시 mem_load는 lsq issue, 나머지는 retire한다.
			}
			else
			{//만약 Issue 안되었다면
				issue(rs_ele,&issue_remain);
				//이슈 조건을 검사하고 부합할 경우 이슈한다.
			}

		}

		//다음 원소로 포인터를 이동한다
		rob_ptr_idx = LL_next_pos(&(rob->ll), rob_ptr_idx);
		rob_ptr = (rob->rob) + (rob_ptr_idx);

	}

}

void rs_retire(struct RS *rs_ele, struct ROB *rob_ele)
{
	rob_ele->status = C;
	(*rs_ele).is_valid = false;
}

void commit(struct simulator_data* simul)
{

	struct ROB_ARR* rob = simul->core.rob;
	struct RAT_ARR* rat = simul->core.rat;
	struct LSQ_ARR *lsq_arr = simul->core.lsq;

	struct cons_cache_controller *cache_cont = simul->cache.cont;
	struct cons_cache *cache = simul->cache.cache;
	struct statistics *stat = simul->cache.stat;

	int remain_commit = (simul->core.info.rob.width);

	struct ROB* rob_ptr;//for easier code
	int rob_ptr_idx;
	
	//그 스레드의 커밋이 끝났는지 아직 계속할 수 있는 지 체크하기 위한 함수
	bool commit_done = false;
	//printf("b");
	bool* thread_commit = calloc(simul->info.num_of_inst,sizeof(bool));
	//printf("b");
	for (int i = 1; i < simul->info.num_of_inst; ++i) { thread_commit[i] = true; }

	//start from head
	rob_ptr_idx = rob->ll.head;
	rob_ptr = (rob->rob)+(rob_ptr_idx);
	for (int i = 0; i < simul->core.rob->ll.occupied && remain_commit > 0 && !commit_done; ++i)
	{//rob 내의 모든 원소에 대해서만 검사하면 됨. 또한 커밋 횟수가 남아있을때만 검사하면 됨. 또한 모든 커밋이 끝나지 않았을때 검사하면 됨.
		if (thread_commit[rob_ptr->inst_num])
		{//아직 해당 inst의 커밋이 끝나지 않았다면,
			switch (rob_ptr->status)
			{
			case C:
				rat[rob_ptr->inst_num].rat[rob_ptr->dest].RF_valid = true;
				LL_cnt_pop(&(rob->ll), rob_ptr_idx);
				--remain_commit;
				
				// Actual cache write happens
				if (rob_ptr->opcode == MemWrite)
				{
					if((*lsq_arr).lsq[rob_ptr->lsq_source].was_hit == HIT)
					{
						// If it was HIT, just write 
						cache_writer(cache_cont, cache, (*lsq_arr).lsq[rob_ptr->lsq_source].address);
					}
					else
					{
						// If it was MISS, fill cache first and the write
						cache_filler(cache_cont, cache, stat, (*lsq_arr).lsq[rob_ptr->lsq_source].address);
						cache_writer(cache_cont, cache, (*lsq_arr).lsq[rob_ptr->lsq_source].address);

					}
				}
				
				// Remove from LSQ
				LL_cnt_pop(&(*lsq_arr).ll, rob_ptr->lsq_source);

				//if (rob_ptr->opcode == MemWrite)
				//{//Mem Write의 경우, commit되면 엑세스가 시작되므로, 처리를 별도로 해주어야 한다.
				//	
				//	int lsq_dest = (simul->core.rs->rs[rob_ptr->rs_dest].lsq_dest);
				//	lsq_write_issue(simul, lsq_dest);
				//
				//}
				break;
			case P:
				thread_commit[rob_ptr->inst_num] = false;
				//inst 커밋이 하나 끝날 때마다 전체 inst 커밋이 끝났는지 검사
				commit_done = true;
				for (i = 1; i < simul->info.num_of_inst; ++i) {
					commit_done &= (!thread_commit[i]);
				}
			}
		}
		//다음 ROB로 포인터를 옮긴다
		rob_ptr_idx = LL_next_pos(&(rob->ll), rob_ptr_idx);
		rob_ptr = (rob->rob) + (rob_ptr_idx);
	}
	free(thread_commit);
}

void wait(void)
{
	printf("\nPress any key to continue :\n");
	getchar();
}

void disp_debug(struct simulator_data* simul, int debug)
{
	switch (debug)
	{
	case 0://display current percentage to do;
		if (simul->info.cycle % 1000 == 1) {
			if (simul->info.cycle == 1) {
				printf("Simulating =");
			}
			printf("%3d%%\b\b\b\b", simul->info.cnt_Insts * 100 / simul->info.Total_Insts);
		}
		break;
	case 1://rob print
		printf("= Cycle %-5d\n", simul->info.cycle);
		ROB_arr_reporter(simul->core.rob);
		break;
	case 2://rob and rs print
		printf("= Cycle %-5d\n", simul->info.cycle);
		RS_arr_reporter(simul->core.rs, simul->core.rob);
		ROB_arr_reporter(simul->core.rob);
		break;
	case 3://lsq print
		printf("= Cycle %-5d\n", simul->info.cycle);
		LSQ_arr_reporter(simul->core.lsq, simul->core.rob);
		break;
	case 4:// rs lsq, rob print
		printf("= Cycle %-5d\n", simul->info.cycle);
		RS_arr_reporter(simul->core.rs, simul->core.rob);
		LSQ_arr_reporter(simul->core.lsq, simul->core.rob);
		ROB_arr_reporter(simul->core.rob);
		break;
	default://debug mode
		printf("= Cycle %-5d\n", simul->info.cycle);
		FQ_arr_printer(simul->core.fq);
		RAT_arr_printer(simul->core.rat);
		RS_arr_printer(simul->core.rs, simul->core.rob);
		LSQ_arr_printer(simul->core.lsq, simul->core.rob);
		ROB_arr_printer(simul->core.rob);
		wait();
		break;
	}
}
