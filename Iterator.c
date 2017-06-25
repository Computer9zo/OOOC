#include "Iterator.h"

//CA 초기화
struct CA_status CA_create(int size)
{
	struct CA_status result;
	result.head = 0;
	result.occupied = 0;
	result.size = size;
	return result;
}

void CA_delete(struct CA_status* del_ca)
{
	//do nothing;
}

//Occupied를 1 늘리고, 현재 push해야할 idx를 반환
int CA_cnt_push(struct CA_status *status)
{
	return CA_get_cidx((*status).occupied++, status);
}

//Occupied를 1 빼고, Head를 1 앞으로 옮김(Head만 pop가능)
void CA_cnt_pop(struct CA_status *status)
{
	(*status).head = ((*status).head + 1) % (*status).size;
	(*status).occupied--;
}

//다음에 넣을 원소 위치를 반환
int CA_next_pos(struct CA_status *status)
{
	return ((*status).head + (*status).occupied) % (*status).size;
}

//Head에서 idx째 원소 위치의 실 idx를 반환
int CA_get_cidx(int idx, struct CA_status *status)
{
	return (idx - (*status).head + (*status).size) % (*status).size;
}

struct LL_status LL_create(int size)
{
	struct LL_status result;
	result.head = 0;
	result.tail = 0;
	result.occupied = 0;
	result.size = size;
	result.next = (int*)calloc(size, sizeof(int));
	result.prev = (int*)calloc(size, sizeof(int));
	
	//메모리 할당이 제대로 되었는지 확인하고, 실패했다면 메모리 할당을 모두 해제하고 size 0인 LL을 반환.
	if (result.next == NULL) 
	{
		result.size = 0; return result; 
	}
	if (result.prev == NULL)
	{
		free(result.next);
		result.size = 0; return result;
	}

	//각 LL관 관계 설정.
	for (int i = 0; i<result.size; ++i) {
		result.next[i] = i + 1;
		result.prev[i] = i - 1;
	}
	result.next[result.size - 1] = 0;
	result.prev[0] = result.size - 1;

	return result;
}

void LL_delete(struct LL_status *del_ll)
{
	free(del_ll->next);
	free(del_ll->prev);
}

void LL_cnt_pop(struct LL_status *status, int pop_num)
{
	//만약 머리가 빠졌다면, 다음 머리는 현재 머리 바로 뒤의 원소
	if (status->head == pop_num) {
		status->head = status->next[pop_num];
	}
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

int LL_cnt_push(struct LL_status *status)
{
	int result = status->tail;

	++(status->occupied);
	status->tail = status->next[status->tail];

	return result;
}

int LL_next_pos(const struct LL_status *status, const int origin_pos)
{
	return status->next[origin_pos];
}

int LL_get_cidx(const struct LL_status *status, const int target_idx)
{
	int idx;
	int ptr = status->head;
	for (idx = 0; idx < status->size; ++idx) {
		if (ptr == target_idx) {
			break;
		}
		ptr = status->next[ptr];
	}
	return idx;
}
