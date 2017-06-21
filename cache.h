#include <stdbool.h>
/* 캐시 모듈 사용법 
 * 
 *
 * 1. 초기화
 * 모듈에는 세 가지 핵심적인 클래스가 있다.
 * 바로 cache_controller (이하 cache_cont)
 * cache (이하 cache) 그리고
 * statistics (이하 stat) 이다.
 * 이것들을 쓰려면 우선 초기화부터 해야 한다.
 *
 * 준비된 cache_config 자료구조의 포인터를 넣어
 * cache_initializer(struct cache_config *config) 함수를 호출하면
 * 이 함수는 void형 포인터를 가리키는 포인터를 반환할 것이다.
 * 사실 이 함수는 void **objects 를 리턴 값으로 가지는데
 * objects[0] 는 함수 내부에서 malloc으로 생성한 cache_cont의 포인터,
 * objects[1] 은 함수 내부에서 생성한 cache의 포인터,
 * objects[2] 는 함수 내부에서 생성한 stat의 포인터를 가지고 있다.
 *
 * 그러니 main에서는 cache_initializer를 호출한 뒤에, 어떤 보이드형 포인터로 받고
 * 이 보이드 형 포인터들을 다시 알맞는 자료형으로 캐스팅하면 생성자가 만든
 * 객체들에 대한 접근을 할 수 있다.
 * 
 * void **objects = cache_initializer(struct cache_config *config);
 * struct cons_cache_controller *cache_cont = objects[0];
 * struct cons_cache *cache = objects[1];
 * struct statistics *stat = objects[2];
 *
 *
 * 2. 캐시 엑세스 (뤼드 / 롸이트)
 * cache_query 함수를 1번에서 얻은 객체들을 인자로 전달하여 호출하면 
 * 캐시 접근이 가능하다.
 * bool cache_query(struct cons_cache controller *cache_cont, struct cons_cache *cache, struct statistics *stat, int access_type, int addr);
 * 앞의 세 객체는 내부적으로 동작하는데 필요하고 뒤의 두 개 중
 * access_type은 읽기/쓰기 정보이다. 여기 cache.h에 enum 타입으로 나와 있다.
 * addr은 접근할 곳의 물리 주소이다.
 *
 * 뤼드 혹은 롸이트 접근을 시도하여
 * 성공하면 true, 실패하면 false의 boolean 값을 반환할 것이다.
 *
 *
 * 3. cache_filler
 * 이 모듈을 시뮬레이터에 붙이기 위해
 * 캐시 미스시 캐시 컨트롤러가 핸들링 하는 부분을 없앴다.
 * 캐시 미스가 나면, Reservation station에 카운트가 50카운트가 되고
 * 미스가 났던 인스트럭션이 Reservation station에서 타임 카운트가 다 되면
 * 이 cache_filler 함수를 호출해야 한다.
 * 이 함수는 메인 메모리에 있는 데이터 블락을 캐시에 써 놓는 역활을 한다.
 * clock update 관련 사항을 캐시 모듈로 끌어들이기 복잡하니 이 함수를 따로 떼었다.
 *
 *
 */

struct cache_config
{
	int capacity;
	int block_size;
	int way_numbers;
	int set_numbers;
};

struct cons_cache
{
	int data[0]; // For struct hack
};

struct statistics
{
	int Read_accesses;
	int Write_accesses;
	int Read_misses;
	int Write_misses;
	int Clean_evictions;
	int Dirty_evictions;
};

struct cons_cache_controller
{
	// Methods for cache_controller
	void (*update_LRU)(struct cons_cache_controller *self, int index, int target_way);
	int (*find_victim)(struct cons_cache_controller *self, int index);
	void (*cache_fill)(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr);
	void (*cache_read)(struct cons_cache_controller *self, struct cons_cache *cache, int addr);
	void (*cache_write)(struct cons_cache_controller *self, struct cons_cache *cache, int addr);
	int (*search)(struct cons_cache_controller *self, struct cons_cache *cache, int addr);
	bool (*read_try)(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr);
	bool (*write_try)(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr);

	// Variables for cache_controller
	// About cache
	int capacity;
	int block_size;
	int way_numbers;
	int set_numbers;
	
	int tag_filter_bits;
	int index_filter_bits;

	// LRU list for True LRU
	int LRU[0]; // For struct hat
};

struct dummy // For struct hat of cons_cache_controller
{
	// Methods for cache_controller
	void (*update_LRU)(struct cons_cache_controller *self, int index, int target_way);
	int (*find_victim)(struct cons_cache_controller *self, int index);
	void (*cache_fill)(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr);
	int (*search)(struct cons_cache_controller *self, struct cons_cache *cache, int addr);
	bool (*read_try)(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr);
	bool (*write_try)(struct cons_cache_controller *self, struct cons_cache *cache, struct statistics *stat, int addr);

	// Variables for cache_controller
	// About cache
	int capacity;
	int block_size;
	int way_numbers;
	int set_numbers;
	
	int tag_filter_bits;
	int index_filter_bits;
};

void *cache_initializer(struct cache_config *config);

bool cache_query(struct cons_cache_controller *cache_cont, struct cons_cache *cache, struct statistics *stat, int access_type, int addr);

void cache_filler(struct cons_cache_controller *cache_cont, struct cons_cache *cache, struct statistics *stat, int addr);

void cache_reader(struct cons_cache_controller *cache_cont, struct cons_cache *cache, int addr);

void cache_writer(struct cons_cache_controller *cache_cont, struct cons_cache *cache, int addr);
