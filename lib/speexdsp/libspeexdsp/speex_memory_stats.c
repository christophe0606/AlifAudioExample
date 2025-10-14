static int allocated_bytes = 0;

void reset_speex_memory_stats()
{
  allocated_bytes = 0;
}

int get_speex_allocated_memory()
{
    return(allocated_bytes);
}

void record_new_speex_allocation(int nb)
{
    allocated_bytes += nb;
}