#include "line_splitter.hh"
#include "probing_hash_utils.hh"
#include "vocabid.hh"
#include <sys/stat.h> //For finding size of file
#include <sys/mman.h>

char * read_binary_file(char * filename);

class QueryEngine {
	char * binary_mmaped;
	std::map<uint64_t, std::string> vocabids;
	Table table;
	int largest_entry; //Size of largest entry, for reading from binary_mmaped when quering
	public:
		QueryEngine (char *, char *, char *, char *, char *);
		~QueryEngine();
		std::pair<bool, std::vector<target_text>> query(StringPiece source_phrase);
		std::pair<bool, std::vector<target_text>> query(std::vector<uint64_t> source_phrase);

};