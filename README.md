# Anagram-Solver
This is a simple program to solve an anagram problem 

 -------------------- The Problem --------------------
We are looking for derived anagrams in a dictionary.
Derived anagram is a word that consists of all the letters from the base word plus one more letter, rearranged ex. sail→nails → aliens.
Write a code in C (working in terminal) that will accept two arguments:
- a dictionary file
- a starting word
The program should search for the longest derived anagram chain based on this starting word and the dictionary.
All words in the chain must be contained in the dictionary.
The words in the dictionary
- have random number of characters between 1 and 255
- consist of random (inclusive repeating) printable ASCII characters, represented by codes 33 to 126
- are unique, so not repeating
The dictionary may consist of up to one million words.
There may be more than one longest chain starting from a given word, the program should find all of them.

-------------------- Algorithms and thought proccess --------------------
1. Algorithms used:

  1 - Dynamic Programming (or Memoization): A technique for solving complex problems by breaking them down into smaller, overlapping sub-problems and saving the results.

  2 - Hashing and Indexing: A fast method of organizing the entire dictionary to     allow near-instant lookups, similar to using a phonebook index.
 

2. Thought Process: How the Task Was Approached (The Narrative)

My main goal was to solve the problem quickly, even with a massive dictionary of up to one million words.

  Phase 1: Structuring the Search and Data
  
  The first step was understanding the inherent structure of the chains:
  
  Level Structure: I realized that to get from one word to the next, the word length must increase by exactly one. This sets up the search structure: the words in the dictionary naturally fall into levels defined by their length. This means the overall search is a path that only ever moves forward one level at a time.
  
  Canonicalization: To handle anagrams (like "tea" and "eat"), I decided to standardize every word by sorting its letters alphabetically—the "standard form." This solved the problem of determining if two words are anagrams without tedious character counting.
  
  Creating a Phonebook (Hashing): Using the "standard form" as a key, I built a fast index—a massive "phonebook"—that maps every standard form to the actual location of its word group in the dictionary. This was the foundation for making lookups instant.
  
  Phase 2: Optimizing the Connections (The Fast Jump)
  
  The core performance challenge was avoiding a slow search. For any given word, we must find the next word quickly:
  
  The Optimization Goal: If we checked every single word in the dictionary for a match, it would be too slow. The goal was to check only the possible next steps. Since a derived anagram means adding just one character, there are only about 94 possible characters to add.
  
  The Instant Check: For any word, I loop through those 94 possible characters. For each character, I create the potential "standard form" of the next word. I then use the pre-built "phonebook" from Phase 1 to instantly see if that new standard form exists in the dictionary. This allows the search to jump straight to the valid next word group in near-constant time.
  
  Phase 3: Finding the Longest Chain
  
  With the structure and fast lookup in place, the pathfinding was implemented:
  
  Pathfinding (DFS): I used a Depth-First Search (DFS) approach to recursively explore all possible chains, always moving from one word level to the next.
  
  The "No Reruns" Rule (Memoization): I quickly realized many chains overlap. To prevent getting stuck recalculating the longest path from the same word hundreds of times, I implemented a "no reruns" rule (memoization). Once the longest chain starting at any word is calculated, the result is saved and instantly reused if needed again.
  
  Error Prevention: For safety and robustness, I chose to use dynamic memory allocation (malloc) for the path string during the recursive printing process. This prevents potential memory errors or stack overflow issues if the final chains turn out to be extremely long.
  
  Phase 4: Recording and Printing
  
  As the program finds the longest chains, it records not only the length but also the IDs of the words that lead to that maximum length. This allows the program to quickly backtrack at the very end and print all paths of that maximum length.
