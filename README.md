# asAlloc
custom allocators in c++
The projects aims at creating a custom memory manager to be used in performance critical applications which can reduce overheads due to
malloc/new by alloacting large chunck of memory up front from heap and using it in a restricted pattern

It will contain 3 components - 

1) A Master class responsible for allocating memory from heap and distributing it to different allocators in form of pages.
2) A set of specialized allocators - 
  a) StackAllocator that can be used when a set of allocations are done in one time frame (without any deallocation) and deallocated at        the end of time frame.
  b) PoolAllocator that can be used in situations when frequent allocation and deallocation of equi-size chucks is required.
3) A general purpose allocator.
