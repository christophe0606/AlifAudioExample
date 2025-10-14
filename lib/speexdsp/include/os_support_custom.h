#ifndef _OS_SUPPORT_CUSTOM_H_
#define _OS_SUPPORT_CUSTOM_H_

#ifdef   __cplusplus
extern "C"
{
#endif

extern void reset_speex_memory_stats();
extern int get_speex_allocated_memory();
extern void record_new_speex_allocation(int nb);

static inline void *speex_alloc (int size)
{
   /* WARNING: this is not equivalent to malloc(). If you want to use malloc()
      or your own allocator, YOU NEED TO CLEAR THE MEMORY ALLOCATED. Otherwise
      you will experience strange bugs */
   //record_new_speex_allocation(size);
   return calloc(size,1);
}

#ifdef   __cplusplus
}
#endif



#endif