/*data structure used by the compressor, is basically a tree where the nodes have a variable number of children
 We implement it as an hash table (faster search) with a field that is the label of the various children that is
 incremented meanwhile the children are added, so the temporary sequence of children insertion is kept
 */

//has to be opaque too?
//node structure
typedef struct ent{
	unsigned int father_label;
	unsigned int symbol;
	unsigned int child_label;
}node;

typedef struct dict{
	int next_free;		//next free entry for collision
	node* tree;
}dictionary;

//structure to define the genreal trie
typedef struct not_tree{
	int next_label;		//next label for a child
	dictionary* dict;	//dictionary
}trie;

/*search if a given father has as a son that symbol, if the search is successful returns 0 and the father label is updated with the child label
 otherwise returns the position in which insert the couple, father label remains still in that case*/
int search(unsigned int* father, unsigned int symbol, trie tr);

//insertion in the trie
int insertion(int position, unsigned int father_label, unsigned int symbol, trie tr);

//generate a new label, in consecutive way
int generate_label(trie* tr);

//find the next_free_entry in case of collision
int find_next_free(trie* tr);

//suppress the structurre in case it is filled or at the end of the program
void suppress(trie* tr);