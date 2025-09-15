# Custom Memory Allocator in C++

A simple **64 KB memory allocator** implemented in C++ to simulate how `malloc` and `free` work under the hood. It uses a **doubly linked list (DLL)** to manage free memory chunks and supports splitting and coalescing of blocks.

## Key Features
- Implements custom `myalloc(size)` and `myfree(ptr)` functions.
- Simulates heap space in the stack using a **DLL of free chunks**.
- Supports:
  - **Splitting** chunks during allocation (if larger than required).
  - **Coalescing** adjacent free chunks to reduce fragmentation.
- Fixed-size heap (`64 KB`) represented as a byte array.

## Time Complexity
- **Allocation (`myalloc`)** → `O(n)` (linear search through available chunks).  
- **Freeing (`myfree`)** → `O(1)` (direct insertion into free list + possible coalescing).

## Design Choices
- **Array indices instead of raw pointers**  
  - Each node in the DLL stores indices (`uint16_t`) instead of pointers (`void*`).  
  - This saves **6 bytes per node** (on 64-bit systems: pointer = 8 bytes vs index = 2 bytes).  
  - For a large number of chunks, this significantly reduces overhead.

- **Packed struct for metadata**  
  - `__attribute__((packed))` removes compiler padding, keeping chunk metadata compact.  
  - Metadata size = **9 bytes per chunk** (`size`, `leftSize`, `vacancy`, `next`, `prev`).

- **DLL Head = Available List**  
  - Free chunks are tracked using a DLL, with head pointing to the first free block.

## Limitations
- Limited memory space - `64 KB`.
- Minimal error handling.
- For educational purposes only (not production ready).

## Future Scope
- Replace linear search with a **hashmap or balanced tree** for `O(1)` or `O(log n)` allocation.   
- Expand heap beyond 64 KB.  

## How to Run
```bash
g++ memory.cpp -o allocator
./allocator
```

## Sample Usage
```cpp
Memory heap;
int *p = (int *) heap.myalloc(sizeof(int));
heap.myfree(p);
```
