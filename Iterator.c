#include "Iterator.h"

//CA �ʱ�ȭ
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

//Occupied�� 1 �ø���, ���� push�ؾ��� idx�� ��ȯ
int CA_cnt_push(struct CA_status *status)
{
	return CA_get_cidx((*status).occupied++, status);
}

//Occupied�� 1 ����, Head�� 1 ������ �ű�(Head�� pop����)
void CA_cnt_pop(struct CA_status *status)
{
	(*status).head = ((*status).head + 1) % (*status).size;
	(*status).occupied--;
}

//������ ���� ���� ��ġ�� ��ȯ
int CA_next_pos(struct CA_status *status)
{
	return ((*status).head + (*status).occupied) % (*status).size;
}

//Head���� idx° ���� ��ġ�� �� idx�� ��ȯ
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
	
	//�޸� �Ҵ��� ����� �Ǿ����� Ȯ���ϰ�, �����ߴٸ� �޸� �Ҵ��� ��� �����ϰ� size 0�� LL�� ��ȯ.
	if (result.next == NULL) 
	{
		result.size = 0; return result; 
	}
	if (result.prev == NULL)
	{
		free(result.next);
		result.size = 0; return result;
	}

	//�� LL�� ���� ����.
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
	//���� �Ӹ��� �����ٸ�, ���� �Ӹ��� ���� �Ӹ� �ٷ� ���� ����
	if (status->head == pop_num) {
		status->head = status->next[pop_num];
	}
	//���ܰ��� ������ next�� ���� ������ next�� ���� �ܰ��� prev�� ���� ������ prev��
	status->next[status->prev[pop_num]] = status->next[pop_num];
	status->prev[status->next[pop_num]] = status->prev[pop_num];

	//���� ���Ҵ� �� ���� ���� �ڿ� ���̰�, 0�� ����Ű�� �Ѵ�.
	status->next[status->prev[status->head]] = pop_num;
	status->prev[pop_num] = status->prev[status->head];
	status->next[pop_num] = status->head;
	status->prev[status->head] = pop_num;

	//�׸��� ������ �ϳ� ����
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
