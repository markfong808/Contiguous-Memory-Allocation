#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#define MEMSIZE 80
#define COMMAND_LEN_LIMIT 10

typedef struct Mem
{
    char process_name;
    int size;
    int reamin_size;
} MemoryBlock;

MemoryBlock *mem_pool;

void create_block()
{
    mem_pool = malloc(MEMSIZE * sizeof(MemoryBlock));
    if (mem_pool == NULL)
    {
        printf("memory allocation failed!\n");
    }
    for (int i = 0; i < MEMSIZE; i++)
    {
        mem_pool[i].process_name = '.'; // Initialize process name to '\0'
        mem_pool[i].size = 0;           // Initialize size to 0
    }
}
void Request_memory(const char name, int size, char algo)
{
    int allocated_size = 0;
    bool enough_space = false;
    switch (algo)
    {
    case 'F': // first fit
    {
        int i = 0;
        while (i < MEMSIZE && allocated_size < size)
        {
            // Find the start of the next available hole
            while (i < MEMSIZE && mem_pool[i].process_name != '.')
            {
                i++;
            }
            if (i >= MEMSIZE)
            {
                break; // No more available holes
            }
            int hole_start = i;
            int hole_size = 0;
            // Calculate the size of the hole
            while (i < MEMSIZE && mem_pool[i].process_name == '.' && hole_size < size - allocated_size)
            {
                hole_size++;
                i++;
            }
            if (hole_size >= size - allocated_size)
            {
                for (int j = hole_start; j < hole_start + size - allocated_size; j++)
                {
                    mem_pool[j].process_name = name;
                }
                allocated_size += hole_size;
                enough_space = true;
            }
            
        }
        if (!enough_space)
        {
            printf("Not enough space to allocate!\n");
        }
        break;
    }
    case 'B':
    { // best fit
        int best_fit_index = -1;
        int min_hole_size = MEMSIZE + 1;
        // find the smallest hole first
        for (int i = 0; i < MEMSIZE; i++)
        {
            if (mem_pool[i].process_name == '.')
            {
                int hole_size = 0;

                int j;
                for (j = i; j < MEMSIZE && mem_pool[j].process_name == '.'; j++)
                {
                    hole_size++;
                }

                if (hole_size < min_hole_size)
                {
                    best_fit_index = i;
                    min_hole_size = hole_size;
                }

                i = j - 1;
            }
        }
        // check and see if the smallest hole can fit the requested mem size
        if (size <= min_hole_size)
        {
            for (int k = best_fit_index; k < best_fit_index + size; k++)
            {
                mem_pool[k].process_name = name;
            }
            allocated_size = size;
        }
        break;
    }
    case 'W': // worst-fit
    {
        int best_fit_index = -1;
        int max_hole_size = 0;
        // find the biggest hole first
        for (int i = 0; i < MEMSIZE; i++)
        {
            if (mem_pool[i].process_name == '.')
            {
                int hole_size = 0;
                int j;
                for (j = i; j < MEMSIZE && mem_pool[j].process_name == '.'; j++)
                {
                    hole_size++;
                }

                if (hole_size > max_hole_size)
                {
                    best_fit_index = i;
                    max_hole_size = hole_size;
                }

                i = j - 1;
            }
        }

        if (size <= max_hole_size)
        {
            for (int k = best_fit_index; k < best_fit_index + size; k++)
            {
                mem_pool[k].process_name = name;
            }
            allocated_size = size;
        }
        break;
    } // worst fit end

    } // switch end
}

void Free_memory(const char name, int size, char algo)
{
    for (int i = 0; i < MEMSIZE; i++)
    {
        if (mem_pool[i].process_name == name)
        {
            mem_pool[i].process_name = '.';
        }
    }
}

void compact_memory_pool(MemoryBlock *mem_pool)
{
    int next_free = 0;
    printf("Compacting memory... \n");

    for (int i = 0; i < MEMSIZE; i++)
    {
        if (mem_pool[i].process_name != '.')
        {
            mem_pool[next_free] = mem_pool[i];
            next_free++;
        }
    }
    // Fill the remaining space with free blocks
    for (int i = next_free; i < MEMSIZE; i++)
    {
        mem_pool[i].process_name = '.'; // Mark as free
        mem_pool[i].size = 0;
    }
}

void show_memory_pool(const MemoryBlock *mem_pool)
{
    printf("Memory Pool Contents:\n");
    for (int i = 0; i < MEMSIZE; i++)
    {

        printf("%c", mem_pool[i].process_name);
    }
    printf("\n");
}

void read_script(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Failed to open file: %s\n", filename);
        return;
    }
    char line[1024];
    fgets(line, sizeof(line), file);
    while (fgets(line, sizeof(line), file) != NULL)
    {
        printf("%s", line); // Print each line
    }

    fclose(file);
}

// A: Allocate N bytes for a process using one of the 3 allocation algorithms
// F: Free all allocations for a given process
// S: Read a script – a sequence of commands from a file, and execute them
// C: Compact the allocations, making them into one contiguous block.  (This somewhat resembles the operation of a mark-sweep garbage collector in C#)
// R: readscript
// E: Exit
int main(int argc, char **argv)
{

    create_block();

    char command[COMMAND_LEN_LIMIT];
    char action;
    char process_name;
    int size;
    char algo = '\0';
    MemoryBlock *curr = mem_pool;

    while (1)
    {

        printf("Enter command (A-allocate,F-free,S-show,R-readScript,C-compact,E-exit)> \n");
        fgets(command, COMMAND_LEN_LIMIT, stdin);
        sscanf(command, "%c %c %d %c", &action, &process_name, &size, &algo);
        if (action == 'A')
        {
            printf("Action: %c , Process Name: %c , Size: %d , Algorithm: %c  \n", action, process_name, size, algo);
            Request_memory(process_name, size, algo);
            continue;
        }
        if (action == 'F')
        {
            printf("Freeing allocated memory for process %c \n",process_name);
            Free_memory(process_name, size, algo);
            continue;
        }
        if (*command == 'C')
        {
            compact_memory_pool(mem_pool);
            show_memory_pool(mem_pool);
        }
        if (*command == 'S')
        {
            show_memory_pool(mem_pool);
        }
        if (*command == 'R')
        {
            printf("Reading script... \n");
            read_script("output.txt");
            continue;
        }

        if (*command == 'E')
        {
            printf("Exit terminal!\n");
            exit(0);
        }
    }

    return 0;
}