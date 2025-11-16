#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// --- Configuration and Constants (Constraints) ---
#define MAX_WORD_LEN    255
#define MAX_DICT_SIZE   1000000
#define HASH_TABLE_SIZE 1000003  // Prime number for optimal hash distribution
#define MAX_NEXT_STEPS  100      // Max successors to store for longest paths

// Data structure for each word in the dictionary
typedef struct {
    char word[MAX_WORD_LEN + 1];
    char canonical[MAX_WORD_LEN + 1];
    int id;               // Unique array index
    int length;
    
    // Dynamic Programming Memoization State
    int max_chain_length;
    int num_next_steps;
    int next_in_chain[MAX_NEXT_STEPS]; // Indices of words that form the next step of the longest chain
} DictWord;

// Data structure for the canonical form Hash Map
typedef struct {
    char canonical_form[MAX_WORD_LEN + 1];
    int word_index;     // Start index in the global dictionary array
    int group_size;     // Number of anagrams in this group
    bool occupied;
} AnagramGroup;

AnagramGroup g_map[HASH_TABLE_SIZE];
DictWord g_dict[MAX_DICT_SIZE];
int g_word_count = 0;


// --- Helper Functions ---

// Used by qsort to order characters
int compare_chars(const void *a, const void *b) {
    return (*(char*)a - *(char*)b);
}

// Sorts characters to get the canonical form (O(L log L))
void canonicalize(const char* word, char* canonical) {
    strcpy(canonical, word);
    qsort(canonical, strlen(canonical), sizeof(char), compare_chars);
}

// DJB2 Hash Function
unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    char c;
    while ((c = *str++)) {
        hash = (hash * 33) + c; 
    }
    return hash;
}

// --- Hash Map Operations ---

void insert_group(const char* canonical, int start_index, int size) {
    unsigned long hash_val = hash_string(canonical) % HASH_TABLE_SIZE;
    int i = (int)hash_val;

    // Linear probing
    while (g_map[i].occupied) {
        i = (i + 1) % HASH_TABLE_SIZE;
    }

    // Insert the new entry
    strcpy(g_map[i].canonical_form, canonical);
    g_map[i].word_index = start_index;
    g_map[i].group_size = size;
    g_map[i].occupied = true;
}

AnagramGroup* find_group(const char* canonical) {
    unsigned long hash_val = hash_string(canonical) % HASH_TABLE_SIZE;
    int i = (int)hash_val;

    // Linear probing
    while (g_map[i].occupied) {
        if (strcmp(g_map[i].canonical_form, canonical) == 0) {
            return &g_map[i];
        }
        i = (i + 1) % HASH_TABLE_SIZE;
    }
    return NULL;
}

// --- Dynamic Programming Search ---

// Recursive DFS with memoization to find the longest chain starting at word_id
int find_longest_chain(int word_id) {
    DictWord* current_word = &g_dict[word_id];

    // Check Memoization result
    if (current_word->max_chain_length > 0) {
        return current_word->max_chain_length;
    }

    int max_length = 1;
    current_word->num_next_steps = 0;

    char next_canonical[MAX_WORD_LEN + 2];
    char current_canonical_copy[MAX_WORD_LEN + 1];
    strcpy(current_canonical_copy, current_word->canonical);
    int current_len = current_word->length;

    // Optimized Successor Search (O(K) where K=94 printable chars)
    // We only check for derived anagrams by adding one valid character.
    for (int c = 33; c <= 126; c++) {
        // Construct potential canonical form of the next word (Length + 1)
        current_canonical_copy[current_len] = (char)c;
        current_canonical_copy[current_len + 1] = '\0';
        
        canonicalize(current_canonical_copy, next_canonical);
        
        // O(1) average hash lookup
        AnagramGroup* group = find_group(next_canonical);
        
        if (group) {
            // Iterate over all words in the found anagram group
            for (int i = 0; i < group->group_size; i++) {
                int next_word_id = group->word_index + i;
                
                int current_chain = 1 + find_longest_chain(next_word_id);
                
                if (current_chain > max_length) {
                    max_length = current_chain;
                    current_word->num_next_steps = 0;
                    current_word->next_in_chain[current_word->num_next_steps++] = next_word_id;
                } else if (current_chain == max_length) {
                    if (current_word->num_next_steps < MAX_NEXT_STEPS) {
                        current_word->next_in_chain[current_word->num_next_steps++] = next_word_id;
                    }
                }
            }
        }
        // Restore terminator for next iteration
        current_canonical_copy[current_len] = '\0'; 
    }

    current_word->max_chain_length = max_length;
    return max_length;
}

// --- Result Printing ---

/*
 * Backtracks through the memoized 'next_in_chain' to print all longest paths.
 * Uses HEAP allocation (malloc) to safely handle recursion path strings.
 */
void print_all_chains(int word_id, char* current_path, int* path_count) {
    DictWord* current_word = &g_dict[word_id];
    
    int current_path_len = strlen(current_path);
    int new_word_len = current_word->length;
    int separator_len = (current_path_len > 0) ? 4 : 0; // 4 for " -> "
    size_t next_path_size = current_path_len + separator_len + new_word_len + 1;

    char* next_path = (char*)malloc(next_path_size);
    if (!next_path) {
        fprintf(stderr, "Error: Memory allocation failed for path buffer.\n");
        return;
    }
    
    // Construct the new path
    strcpy(next_path, current_path);
    if (current_path_len > 0) {
        strcat(next_path, " -> ");
    }
    strcat(next_path, current_word->word);

    if (current_word->num_next_steps == 0) {
        printf("Chain %d: %s\n", ++(*path_count), next_path);
    } else {
        // Traverse all longest successor paths
        for (int i = 0; i < current_word->num_next_steps; i++) {
            print_all_chains(current_word->next_in_chain[i], next_path, path_count);
        }
    }
    
    // CRITICAL: Free the path buffer to prevent memory leaks in recursion
    free(next_path);
}


// --- Dictionary Loading and Preprocessing ---

bool load_and_preprocess_dictionary(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open dictionary file '%s'.\n", filename);
        return false;
    }

    char line_buffer[MAX_WORD_LEN + 2]; 
    int word_counter = 0;

    // Read words, check constraints, and canonicalize
    while (fgets(line_buffer, sizeof(line_buffer), file) != NULL) {
        
        if (word_counter >= MAX_DICT_SIZE) {
            fprintf(stderr, "Warning: Dictionary size exceeds maximum limit of %d. Truncating.\n", MAX_DICT_SIZE);
            break;
        }

        // Clean up: remove trailing newline and carriage return
        size_t len = strlen(line_buffer);
        if (len > 0 && line_buffer[len - 1] == '\n') line_buffer[--len] = '\0';
        if (len > 0 && line_buffer[len - 1] == '\r') line_buffer[--len] = '\0';

        // Skip invalid words
        if (len == 0 || len > MAX_WORD_LEN) continue;
        
        DictWord* entry = &g_dict[word_counter];
        strcpy(entry->word, line_buffer);
        entry->id = word_counter;
        entry->length = (int)len;
        entry->max_chain_length = 0;
        
        canonicalize(entry->word, entry->canonical);
        word_counter++;
    }
    
    fclose(file);
    g_word_count = word_counter;
    
    if (g_word_count == 0) {
        fprintf(stderr, "Error: Dictionary is empty or contains no valid words.\n");
        return false;
    }

    // Sort the main dictionary array by canonical form to group anagrams
    qsort(g_dict, g_word_count, sizeof(DictWord), 
          (int (*)(const void *, const void *))strcmp); 

    // Populate the Hash Map for O(1) lookups
    int group_start_index = 0;
    while (group_start_index < g_word_count) {
        const char* current_canonical = g_dict[group_start_index].canonical;
        int group_size = 0;
        
        // Find the end of the current anagram group
        for (int i = group_start_index; i < g_word_count; i++) {
            if (strcmp(g_dict[i].canonical, current_canonical) == 0) {
                group_size++;
            } else {
                break;
            }
        }

        insert_group(current_canonical, group_start_index, group_size);
        
        group_start_index += group_size;
    }

    return true;
}


// --- Main Program ---

int main() {
    
    char dictionary_path[MAX_WORD_LEN + 1]; 
    char starting_word_input[MAX_WORD_LEN + 1]; 

    // --- Interactive Input ---
    
    printf("----------------------------------\n");
    
    printf("Enter Dictionary File Path: ");
    if (scanf("%s", dictionary_path) != 1) {
        fprintf(stderr, "Error: Could not read file path.\n");
        return EXIT_FAILURE;
    }
    
    printf("Enter Starting Word: ");
    if (scanf("%s", starting_word_input) != 1) {
        fprintf(stderr, "Error: Could not read starting word.\n");
        return EXIT_FAILURE;
    }


    // 2. Dictionary Loading and Preprocessing
    printf("\nLoading and preprocessing dictionary...\n");
    if (!load_and_preprocess_dictionary(dictionary_path)) {
        return EXIT_FAILURE;
    }
    printf("Loaded %d unique word entries.\n", g_word_count);
    
    // 3. Find the starting word's entry in the dictionary
    char starting_word_canonical[MAX_WORD_LEN + 1];
    canonicalize(starting_word_input, starting_word_canonical);

    AnagramGroup* start_group = find_group(starting_word_canonical);
    if (!start_group) {
        printf("\nResult: Starting word '%s' is not found in the dictionary.\n", starting_word_input);
        return EXIT_SUCCESS;
    }

    // 4. Initiate the Dynamic Programming search
    int overall_max_length = 0;
    
    // Check all anagrams of the input word (to ensure we find the longest path regardless of starting word variant)
    for (int i = 0; i < start_group->group_size; i++) {
        int start_id = start_group->word_index + i;
        int chain_len = find_longest_chain(start_id);
        
        if (chain_len > overall_max_length) {
            overall_max_length = chain_len;
        }
    }
    
    // 5. Output Results
    if (overall_max_length <= 1) {
        printf("\nResult: No derived anagram chain found starting from '%s'.\n", starting_word_input);
        return EXIT_SUCCESS;
    }
    
    printf("\n--- Longest Derived Anagram Chains ---\n");
    printf("Max Chain Length: %d words.\n", overall_max_length);

    int total_chains_printed = 0;
    
    char empty_path[] = ""; 

    for (int i = 0; i < start_group->group_size; i++) {
        int start_id = start_group->word_index + i;
        if (g_dict[start_id].max_chain_length == overall_max_length) {
            print_all_chains(start_id, empty_path, &total_chains_printed);
        }
    }
    
    printf("Total longest chains found: %d\n", total_chains_printed);
    printf("------------------------------------\n");

    return EXIT_SUCCESS;
}
