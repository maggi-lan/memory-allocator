#include <iostream>
#include <cstdint>
#define HEAP_CAP 65536  // 2^16

using namespace std;




class DLL {
	protected:
		char bytes[HEAP_CAP] = {0};

		// __attribute__ disables padding and might not work on windows
		struct __attribute__((packed)) chunkData {
			uint16_t size;      // 2 bytes --> size of chunk
			uint16_t leftSize;  // 2 bytes --> size of chunk on the left of current chunk
			bool vacancy;       // 1 byte  --> true if chunk is free
			uint16_t next;      // 2 bytes --> index of next free chunk
			uint16_t prev;      // 2 bytes --> index of previous free chunk
			// NOTE: index '0' will be treated like nullptr to mark the ends of DLL
			// NOTE: hence no chunk will be stored there
		};

		// Converts Memory Address to Memory Index
		uint16_t toIndex(chunkData *chunk) {
			if ((char *) chunk < &bytes[0] || (char *) chunk > &bytes[HEAP_CAP - 1]) {
				cout << "Invalid Memory Address\n";
				return (uint16_t) 0;
			}

			return (uint16_t) ((char *) chunk - &bytes[0]);
		}

		// Converts Array Index to Memory Address
		chunkData *toAddress(uint16_t index) {
			if (index < 0 || index > HEAP_CAP - 1) { 
			// not rejecting index = 0 since this function can be helpful to replace "&bytes[0]"
				cout << "Invalid Array Index\n";
				return nullptr;
			}

			return (chunkData *) &bytes[index];
		}


		// Deletes a Node & Returns the Address of the Deleted Node
		chunkData *deleteNode(chunkData *&chunk, chunkData *&head) {
			uint16_t nextIndex = chunk->next;
			uint16_t prevIndex = chunk->prev;

			chunk->vacancy = false;
			chunk->next = 0;
			chunk->prev = 0;

			if (nextIndex != 0)
				toAddress(nextIndex)->prev = prevIndex;	
			if (prevIndex != 0)
				toAddress(prevIndex)->next = nextIndex;	

			if (head == chunk)
				head = toAddress(nextIndex);
			return chunk;
		}

		// Adds a Node
		void addNode(chunkData *&chunk, chunkData *&head) {
			chunk->vacancy = true;
			chunk->prev = 0;
			if (head != toAddress(0))
				chunk->next = toIndex(head);
			else
				chunk->next = 0;

			if (head != nullptr && head != toAddress(0))
				head->prev = toIndex(chunk);	
			head = chunk;
		}
};


class Memory : private DLL {
	private:
		chunkData *avail;  // Head of a DLL which contains Free Chunks

		// Merges Chunks (which are adjacent in the byte array) Together
		void coalesce(chunkData *&chunk, chunkData *&head) {
			// Merging with Left Chunk
			chunkData *leftChunk =  toAddress(toIndex(chunk) - chunk->leftSize);
			if (chunk->leftSize == 0)
				leftChunk = toAddress(0);
			if (leftChunk > toAddress(0) && leftChunk->vacancy) {
				leftChunk->size += chunk->size;
				deleteNode(chunk, avail);       // remove right
				deleteNode(leftChunk, avail);   // remove left
				addNode(leftChunk, avail);      // re-add merged so that it would be on the front of the DLL
				chunk = leftChunk;
        	}

			// Merging with Right Chunk
			chunkData *rightChunk;
			if ((toIndex(chunk) + chunk->size) < HEAP_CAP)
				rightChunk = toAddress(toIndex(chunk) + chunk->size);
			else
				rightChunk = toAddress(0);	
			if (rightChunk > toAddress(0) && rightChunk <= toAddress(HEAP_CAP - 1) && rightChunk->vacancy == true) {
				chunk->size += rightChunk->size;
				deleteNode(rightChunk, avail);
				deleteNode(chunk, avail);
				addNode(chunk, avail);
			}

			// Updating the leftSize of the Chunk on the Right of the Merged Chunk
			int rightIndex = toIndex(chunk) + chunk->size;
			if (rightIndex < HEAP_CAP) 
				toAddress(rightIndex)->leftSize = chunk->size;
		}

	public:
		// Initializing the 'avail' List with a Chunk
		Memory() {
			chunkData *head = toAddress(1);
			head->size = HEAP_CAP - 1;
			head->leftSize = 0;
			head->vacancy = true;
			head->next = 0;
			head->prev = 0;

			avail = head;
		}

		// Find a Free Chunk from 'avail', Remove it from 'avail' & Return the Address of the Chunk
		void *myalloc(int allocSize) {
			for (chunkData *ptr = avail; ptr != toAddress(0); ptr = toAddress(ptr->next)){
				chunkData* chunk = nullptr;
				int needed = sizeof(chunkData) + allocSize;

				if (ptr->size >= needed) {

					// CASE 1: Splitting Existing Chunk --> Existing Chunk is Big 
					if (ptr->size >= needed + sizeof(chunkData) + 1) {
						chunk = ptr;
						uint16_t tempSize = ptr->size - (allocSize + sizeof(chunkData));
						chunk->size = allocSize + sizeof(chunkData);
						chunk->vacancy = false;

						ptr = toAddress(toIndex(chunk) + chunk->size);
						ptr->vacancy = true;
						ptr->size = tempSize;
						ptr->leftSize = chunk->size;
						
						// Replacing chunk with the ptr in avail
						chunkData *chunkNext = toAddress(chunk->next);
						ptr->next = toIndex(chunkNext);
						if (chunkNext > toAddress(0))
							chunkNext->prev = toIndex(ptr);	
						chunkData *chunkPrev = toAddress(chunk->prev);
						ptr->prev = toIndex(chunkPrev);
						if (chunkPrev > toAddress(0))
							chunkPrev->next = toIndex(ptr);
						if (avail == chunk)
							avail = ptr;

						// Updating the leftSize of the Chunk on the Right of the Right Splitted Chunk
						int rightIndex = toIndex(ptr) + ptr->size;
						if (rightIndex < HEAP_CAP) 
							toAddress(rightIndex)->leftSize = ptr->size;
					}

					// CASE 2: No Splitting of Existing Chunk Required --> Existing Chunk is of Perfect Size
					else {
						chunk = deleteNode(ptr, avail);
					}
				}

				// Returning Address of the Chunk (if chunk is valid)
				if (chunk)
					return toAddress(toIndex(chunk) + sizeof(chunkData));
			}

			// Program reaches here if "NO Free Space is Available"
			cout << "Memory Allocation Failed\n";
			return nullptr;
		}

		// Add an Occupied Chunk to 'avail' & Coalesce it with other Chunks (if possible)
		// DON'T CALL THIS FUNCTION WITH INVALID ADDRESSES, MIGHT BREAK THE PROGRAM
		void myfree(void *ptr) {
			chunkData *chunk = toAddress(toIndex((chunkData *) ptr) - sizeof(chunkData));

			if (chunk <= toAddress(0) || chunk > toAddress(HEAP_CAP - 1)) {
				cout << "Invalid Input\n";
				return;
			}
			
			if (chunk->vacancy == true) {
				cout << "This chunk is already free\n";
				return;
			}

			chunk->vacancy = true;
			addNode(chunk, avail);
			coalesce(chunk, avail);
		}
};


// TEST CODE
int main(void) {
	Memory heap;

	int *p1 = (int *) heap.myalloc(sizeof(int));
	cout << "(4 bytes) p1 = " << p1 << '\n';

	int *p2 = (int *) heap.myalloc(sizeof(int));
	cout << "(4 bytes) p2 = " << p2 << '\n';

	int *p3 = (int *) heap.myalloc(sizeof(int));
	cout << "(4 bytes) p3 = " << p3 << '\n';

	heap.myfree(p1);
	cout << "p1 freed\n";

	heap.myfree(p2);
	cout << "p2 freed\n";

	int **p4 = (int **) heap.myalloc(sizeof(int *));
	cout << "(8 bytes) p4 = " << p4 << '\n';

	int **p5 = (int **) heap.myalloc(sizeof(int *));
	cout << "(8 bytes) p5 = " << p5 << '\n';

	int *p6 = (int *) heap.myalloc(HEAP_CAP);
	cout << "p6 = " << p6 << '\n';


	return 0;
}
