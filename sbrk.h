

#if defined (XMC1)
extern "C" void *_sbrk(int incr);
    void dummy_sbrk_caller() __attribute__((__used__));
    void dummy_sbrk_caller()
    {
      _sbrk(0);
    }
#endif 
