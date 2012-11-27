#define MAX_ELEMENTS MAX_ARRAY
#define USE_CACHE_ALIGN

#define MAGIC_CHARS_ROW {'R','O','W',' '}

typedef struct _row_comm {
	char magic[8];		// magic
	int elements;		// number of cpus
	int id;				// cpu identifier
	unsigned long lock;	// lock

#ifdef USE_CACHE_ALIGN
	char pad0[(CACHE_LINE -
			( (sizeof(char) *8) +
			(sizeof(int) *2) +
			(sizeof(unsigned long) *1) ) %
			CACHE_LINE)];
#endif	/* USE_CACHE_ALIGN */

	bitmask_t 	  status[MAX_BITMAP];   // receivers (cells) status
	bitmask_t 	  active[MAX_BITMAP];	// currently allocated buffers or initialized
	unsigned long offset[MAX_ARRAY];	// offset of the buffer from the beginning of the map
	int 	 	  csize;				// allocated space per buffer
	int 		  cnumber;				// number of buffers currently present

#ifdef USE_CACHE_ALIGN
	char pad1[(CACHE_LINE -
			( (sizeof(bitmask_t) *2 *MAX_BITMAP) +
			(sizeof(int) *2) +
			(sizeof(unsigned long) *MAX_ARRAY) ) %
			CACHE_LINE)];
#endif	/* USE_CACHE_ALIGN */
} row_comm;

//NOTA ricordati la differenza tra massimo numero di elementi e quelli correntemente allocati

// this is dependent by the type of memory allocator we are using
// see what we learn from the MPICH/Nemesis guys so define it
// simply void*
typedef struct _shm_desc {
	size_t	size;
	key_t	key;
	row_comm * addr; // used by the setup process
} shm_desc;
// TODO move in the allocator


#define MAGIC_CHARS_MATR {'M','A','T','R'}
//general communicator header
typedef struct _matrix_comm {
	char			magic[4];
	unsigned long	lock; // we have to spinlock so must maybe be cache aligned (the problem is this does not to be critical)

	int				elements; // the maximum number of elements, or the currently allocated elements
	bitmask_t 		present[MAX_BITMAP]; // which elements are currently really allocated
	void*			desc[MAX_ELEMENTS]; // available descriptors per element

} matrix_comm;


// NOTA the following two can be merged in one!!!
// they are both allocated locally/privately
// so no one cares
// what will be readed concurrently and is in the same cache line?
// does something like this exists? and if yes what?

//structure CACHE_ALIGNED !!!
typedef struct _comm_buffers {

	// THIS IS MY BITMAP: the copy is inshared memory
	bitmask_t * recv_bmp; //this first because must be cache aligned
	// THIS ARE THE ptr to MY BUFFERS hopefully they will fit the same cache line of the pointer before
	bbuffer_t * recv_buf[MAX_ARRAY];

#ifdef USE_CACHE_ALIGN
	char pad0[(CACHE_LINE -
			( (sizeof(bitmask_t *) *1) +
			(sizeof(bbuffer_t *) *MAX_ARRAY) ) %
			CACHE_LINE)];
#endif	/* USE_CACHE_ALIGN */

	// the following data is packed per item  because it will be accessed together
	struct {
		// remote bitmap pointer
		bitmask_t * bmp;
		// remote recv buffer pointer
		bbuffer_t * buf;
	} send[MAX_ARRAY];

	// the following is not aligned
	int elements, id; //information must be replicated on each local copy

} comm_buffers;

// this is the private/local summary of each area
typedef struct _comm_mapping {
	matrix_comm * matrix;
	row_comm    * row[MAX_ARRAY];

	// remove the followings?!?!
	// the following can be reconstructed from base
	bitmask_t* bmp[MAX_ARRAY];
	bbuffer_t* buf[MAX_ARRAY];
} comm_mapping;

// PROTOTYPES DECLARATION

comm_mapping * matrix_init_mapping (int size, int elements);
comm_buffers * matrix_init_buffers (comm_mapping * map, int id);

void matrix_finalize_buffers(comm_mapping * map);
void matrix_finalize_mapping(comm_mapping * map);

int matrix_send_to(comm_buffers * buffs, int dest, char* buff, int count);
int matrix_send_self(comm_buffers * buffs, char *buff, int count);

int matrix_recv_from(comm_buffers* buffs, int src, char* buff, int count);
int matrix_recv_self(comm_buffers* buffs, char* buff, int count);
