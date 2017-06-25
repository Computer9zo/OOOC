#ifndef  __ITERATOR__
#define  __ITERATOR__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//�迭 �� ������ �ƴ� ����� Iterator ������ �ʿ��� �Լ��� ������ ���� ���
//

struct CA_status  // Status of cyclic array
{
	int size;
	int head;
	int occupied;
};

struct CA_status CA_create(int size);
void CA_delete(struct CA_status* del_ca);

int CA_cnt_push(struct CA_status *status);
void CA_cnt_pop(struct CA_status *status);

int CA_get_cidx(int idx, struct CA_status *status);//��¥ ��� �ε����� head������ �Ÿ��� �ٲ�
int CA_next_pos(struct CA_status *status);

struct LL_status
{
	int size;
	int head;
	int occupied;
	int tail;

	int*next;
	int*prev;
};

struct LL_status LL_create(int size);
void LL_delete(struct LL_status *del_ll);

void LL_cnt_pop(struct LL_status *status, int pop_num);
int LL_cnt_push(struct LL_status *status);

int LL_next_pos(const struct LL_status *status, const int origin_pos);
int LL_get_cidx(const struct LL_status *status, const int target_idx);

#endif // ! __ITERATOR__//