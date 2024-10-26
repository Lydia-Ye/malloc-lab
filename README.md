# BiBop Memory Allocator

## Project Overview
This project is a custom memory allocator implemented in C/C++, designed to replace the standard `malloc` and `free` functions in linux system with a "Big Bag of Pages" (BiBoP) allocator. This allocator efficiently handles dynamic memory allocation requests by organizing memory into size-based blocks, allowing quick access, allocation, and deallocation. It also includes handling for both small and large memory requests to optimize memory usage and improve performance.

### 1. **Efficient Allocation with Size Classes**
   - Memory is allocated in predefined size classes ranging from 16 to 2048 bytes. Each allocation request is rounded up to the nearest power of two, ensuring that memory is aligned and that allocations are organized for efficient reuse.
   - For requests above 2048 bytes, the allocator uses `mmap` to request memory directly from the operating system.

### 2. **Freelist Management**
   - The allocator maintains separate freelists for each size class, allowing efficient access to available memory blocks of the requested size.
   - Freed memory is immediately placed at the front of the appropriate freelist, enabling quick reuse and reducing memory fragmentation.

### 3. **Embedded Metadata**
   - Each memory page includes a header with essential metadata, such as a magic number and the size of allocated objects in that page. This design provides quick access to allocation details and enables safe deallocation, preventing undefined behavior when invalid pointers are passed to `free`.

### 4. **16-Byte Alignment**
   - All allocated memory blocks are aligned to a 16-byte boundary, making the allocator portable across different system architectures that require specific alignment constraints.

### 5. **Fallback for Large Allocations**
   - The allocator supports larger allocations by mapping memory directly via `mmap` for requests exceeding 2048 bytes. This approach optimizes memory usage by bypassing the freelist structure for oversized objects, thus preventing inefficiencies and potential fragmentation in smaller size classes.

### 6. **Memory-Safe Deallocation**
   - `free` operations utilize the allocator’s metadata to validate memory blocks before deallocation, reducing errors from double-free or invalid-free operations. This ensures that only memory allocated by this allocator is returned to the freelist.

### 7. **Custom Logging for Debugging**
   - A custom `log_message` function is provided to safely log messages without triggering additional memory allocations, making it compatible with low-level debugging requirements in the allocator’s constrained environment.

## Benefits
This custom allocator provides predictable, efficient memory management and avoids issues like memory leaks or fragmentation by reusing freed memory blocks effectively. It also offers greater control over alignment and size constraints, making it an ideal choice for memory-constrained environments or applications with specific memory usage requirements.
