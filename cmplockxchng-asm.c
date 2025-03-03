__attribute__((always_inline)) static uint64_t cmpxchg(uint64_t new, uint64_t old, volatile uint64_t* lock) {
         uint64_t result = 0;
         asm volatile(
             "lock cmpxchg %2,%1\n"
             "sete (%3)\n"
             : : "a"(old),"m"(*lock),"r"(new),"r"(&result)
             : "memory","cc"
         );
         return result;
}
