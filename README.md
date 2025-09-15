# Custom Memory Allocator in C++

A simple **64 KB memory allocator** implemented in C++ to simulate how `malloc` and `free` work under the hood. It uses a **doubly linked list (DLL)** to manage free memory chunks and supports splitting and coalescing of blocks.

## Key Features
- Implements custom `myalloc(size)` and `myfree(ptr)` functions.
- Simulates heap space in the stack using a **DLL of free chunks**.
- Supports:
  - **Splitting** chunks during allocation (if larger than required).
  - **Coalescing** adjacent free chunks to reduce fragmentation.
- Fixed-size heap (`64 KB`) represented as a byte array.

## Logic
- **MEMORY**
  - `bytes` is an array of 2<sup>16</sup> bytes and this will simulate the heap space.
  - This array will be divided into **memory chunks** with each chunk having a header (of size 9 bytes).
  - The header acts as a DLL node and the free chunks are kept track of as a DLL of headers.
  - **NOTE**: `avail` = head node of the DLL.
- **ALLOCATION**
  - When `myalloc` is called, linear search is implemented on the DLL to find a chunk whose size is equal to or more than the required size.
  - If a chunk roughly fits the required size, the node is removed from the DLL and the address of the chunk is returned.
  - If a chunk is too big, the chunk is split into two.
  - The address of the perfectly sized chunk is returned while the DLL is updated with the other chunk.
  - **NOTE**: allocated chunks aren't tracked explicitly since a chunk is not free if it isn't in the list.
- **FREEING**
  - When `myfree` is called, the chunk header is added back to the DLL.
  - Merging of free chunks which are contiguous in `bytes` might occur with the help of `coalesce()`.
  - Coalescing helps to deal with fragmentation of memory.

## Time Complexity
- **Allocation (`myalloc`)** → `O(n)` (linear search through available chunks).
- **Freeing (`myfree`)** → `O(1)` (direct insertion into free list + possible coalescing).

## Design Choices
- **Array indices instead of raw pointers**
  - Each node in the DLL stores indices (`uint16_t`) instead of pointers (`void*`).
  - This saves **12 bytes per node**
  - For a large number of chunks, this significantly reduces overhead.

- **Packed struct for metadata**
  - `__attribute__((packed))` removes compiler padding, keeping chunk metadata compact.
  - Metadata size = **9 bytes per chunk** (`size`, `leftSize`, `vacancy`, `next`, `prev`).

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
