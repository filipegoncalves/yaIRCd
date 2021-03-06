#ifndef TRIE_GUARD
#define TRIE_GUARD

/** @file
	@brief Flexible trie with some neat options.

	This file describes every trie operations available to the various list managers (nicknames list, commands list, channels list, and any other list of strings)
	It allows client code (by client we mean "the code that uses this") to define which characters are allowed inside a word.
	Client code is required to provide functions that convert a letter into a position (ID) and a position back into a letter. IDs must be unique, consecutive, and start at 0; there can be no gaps
	in the sequence, because IDs are used to index an array. So, for example, to allow an alphabet which consists of the characters `[a-z]` and `[0-9]`, client code must find a mapping which converts 
	any of these characters into an integer `i` such that `i >= 0 && i <= 35` (26 letters for the alphabet and 10 digits).
	One possible mapping would be to map any letter `c` in `[a-z]` to `c - &lsquo;a&rsquo;` and any number `i` in `[0-9]` to `&lsquo;z&rsquo; + i - &lsquo;0&rsquo;`.
	
	Please refer to http://en.wikipedia.org/wiki/Trie if you are not sure how a trie works. It always guarantees `O(n)` insertion, deletion and search time, where `n` is the size of the word. When compared to hash tables,
	it is a good alternative, since hash tables provide `O(1)` access, but normally take about `O(n)` time to compute the hash function, and there can be collisions.
	
	@author Filipe Goncalves
	@date November 2013
	@see trie.c
	@warning This implementation is reentrant, but it is not thread safe. The same trie instance cannot be fed into this implementation from different threads concurrently. 
	Upper caller needs to make the necessary use of mutexes or other synchronization primitives.
*/

/** Return code for invalid words */
#define TRIE_INVALID_WORD 1

/** Return code for out of memory error */
#define TRIE_NO_MEM 2

/** Flag that indicates that a node's data is to be freed when it is about to be destroyed. 
	@see destroy_trie()
*/
#define TRIE_FREE_DATA 1

/** Flag that indicates that a node's data shall not be freed when it is about to be destroyed. 
	@see destroy_trie()
*/
#define TRIE_NO_FREE_DATA 0

/** A node in a trie. */
struct trie_node {
	char is_word; /**<Indicates if the path from root down to this node denotes a word */
	int children; /**<Says how many children are present in `edges[]` */
	struct trie_node **edges; /**<Edges pointing to this node's children. This is a dynamically allocated array. Each `edges[i]` represents the character returned by `(*trie->pos_to_char)(i)`. */
	void *data; /**<Pointer to arbitrary data associated with this node. This is valid only if `is_word` is true, and it is used by the client code to associate data with words. */
};

/** A trie */
struct trie_t {
	struct trie_node *root; /**<Root node */
	void (*free_f)(void *, void *); /**<A pointer to a function that is responsible for free'ing a node's `data` when it is about to be destroyed. See `destroy_trie()` and `delete_word_trie()` for further info. */									     
	int (*is_valid)(char s); /**<A pointer to a function that returns `1` if `s` is a valid character (considered part of a word), and `0` otherwise. */
	char (*pos_to_char)(int p); /**<A pointer to a function that converts an index position from `edges` back to its character representation. */
	int (*char_to_pos)(char s); /**<A pointer to a function that converts a character `s` into a valid, unique position that is used to index `edges`. */
	int edges_no; /**<Number of edges. It is valid to reference any position in `edges[0 .. edges_no-1]`. */
};

/** A stack element describing a node in a path of a prefix search. */
struct trie_node_stack_elm {
	struct trie_node *el; /**<Pointer to the node that this element describes */
	struct trie_node_stack_elm *next; /**<Pointer to the next stack element */
	char letter; /**<Letter that was last used to reach this node, namely, the result of calling `(*trie->pos_to_char)(i)` for an edge `i` such that `edges[i] == `el`. */
	int depth; /**<Depth of this element on the stack. Starts counting from 1. */
};

/** A stack used to maintain state between different calls to `find_by_prefix_next_trie()`. */
struct trie_node_stack {
	char *path; /**<A characters sequence describing the path from the root node down to the current node. Each `trie_node_stack_elm` in the stack writes to `path[element->depth-1]`. */
	char *prefix; /**<The prefix originally passed to `find_by_prefix_next_trie()` in the first call that started this search. */
	int depth; /**<Max. depth allowed. Only character sequences of at most `depth-1` will be reported and written to `path`. When a match is found, `path` is null terminated; it must hold enough space for at least `depth` characters. */
	struct trie_node_stack_elm *top; /**<The top of the stack */
};

/* These functions are documented in the C file that implements them */
struct trie_t *init_trie(void (*free_function)(void *, void *), int (*is_valid)(char), char (*pos_to_char)(int), int (*char_to_pos)(char), int edges);
void destroy_trie(struct trie_t *trie, int free_data, void *args);
int add_word_trie(struct trie_t *trie, char *word, void *data);
void *delete_word_trie(struct trie_t *trie, char *word);
void *find_word_trie(struct trie_t *trie, char *word);
void trie_for_each(struct trie_t *trie, void (*f)(void *, void *), void *args);
struct trie_node_stack *find_by_prefix_next_trie(struct trie_t *trie, struct trie_node_stack *st, const char *prefix, int depth, char *result, int *err_code, void **data);
void free_trie_stack(struct trie_node_stack *st);
#endif /* TRIE_GUARD */
