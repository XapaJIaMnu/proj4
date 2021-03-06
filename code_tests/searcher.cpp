#include "util/file_piece.hh"

#include "util/file.hh"
#include "util/scoped.hh"
#include "util/string_piece.hh"
#include "util/tokenize_piece.hh"
#include "util/murmur_hash.hh"
#include "util/probing_hash_table.hh"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h> //For finding size of file
#include <boost/functional/hash.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void readTable(char * filename, char *mem, size_t size);

uint64_t getHash(StringPiece text);

uint64_t getHash(StringPiece text) {
	std::size_t len = text.size();
	uint64_t key = util::MurmurHashNative(text.data(), len);
	return key;
}

void readTable(char * filename, char *mem, size_t size) {
	//Initial position of the file is the end of the file, thus we know the size
	std::ifstream file (filename, std::ios::in|std::ios::binary);
	file.read ((char *)mem, size); // read
	file.close();
}

//Hash table entry
struct Entry {
	uint64_t key;
	typedef unsigned char Key;

	uint64_t GetKey() const {
		return key;
	}

	void SetKey(uint64_t to) {
		key = to;
	}

	unsigned int GetValue() const {
		return value;
	}

	unsigned int value;
};

//Define table
typedef util::ProbingHashTable<Entry, boost::hash<uint64_t> > Table;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		// Tell the user how to run the program
		std::cerr << "Usage: " << argv[0] << " path_to_hashtable path_to_data_bin" << std::endl;
		return 1;
	}

	int i;
	int fd;
	int *map;  /* mmapped array of int's */

	//Find the size
	struct stat filestatus;
	stat(argv[2], &filestatus);
	unsigned long filesize = filestatus.st_size;
	int array_length = filesize/sizeof(int) - 1; //The end of file has \0, which we don't want to count.


	fd = open(argv[2], O_RDONLY);
	if (fd == -1) {
		perror("Error opening file for reading");
		exit(EXIT_FAILURE);
	}

	map = (int*)mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);

	//End of memory mapping code

	//Init the table
	int tablesize = 23339836;
	size_t size = Table::Size(tablesize, 1.2);
	char * mem = new char[size];
	memset(mem, 0, size);
	readTable(argv[1], mem, size);
	Table table(mem, size);
	table.CheckConsistency();
	std::cout << "Table is consistent!" << std::endl;

	//Get a key;
	StringPiece str = StringPiece("! &apos; - and what is more");
	uint64_t key = getHash(str);

	const Entry * tmp;
	table.Find(key, tmp);
	std::cout <<"Key found" << std::endl;
	unsigned int idx = tmp -> GetValue();

	std::cout << "Key is " << tmp -> GetKey() << ", expected " << key << std::endl;

	std::cout << "Value is " << map[idx] << " expected 425598091" << std::endl;

	//Interactive search
	std::cout << "Please enter a string to be searched, or exit to exit." << std::endl;
	while (true){
		bool found;
		std::string cinstr = "";
		getline(std::cin, cinstr);
		if (cinstr == "exit"){
			break;
		}else{
			StringPiece tofind = StringPiece(cinstr);
			key = getHash(cinstr);
			found = table.Find(key, tmp);
			if (found) {
				std::cout << "Integer corresponding to " << cinstr << " is " << map[tmp -> GetValue()] << std::endl;
			} else {
				std::cout << "Key not found!" << std::endl;
			}
			
		}
	}
	//clean up
	delete[] mem;
	close(fd);

	return 0;
}