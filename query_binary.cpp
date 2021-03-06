#include "util/file_piece.hh"

#include "util/file.hh"
#include "util/scoped.hh"
#include "util/string_piece.hh"
#include "util/tokenize_piece.hh"
#include "util/murmur_hash.hh"
#include "util/probing_hash_table.hh"
#include "util/usage.hh"

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
#include <ctime> //for timing.
#include <chrono>

char * readTable(char * filename, size_t size);

uint64_t getHash(StringPiece text);

uint64_t getHash(StringPiece text) {
	std::size_t len = text.size();
	uint64_t key = util::MurmurHashNative(text.data(), len);
	return key;
}

//Read table from disk, return memory map location
char * readTable(char * filename, size_t size) {
	//Initial position of the file is the end of the file, thus we know the size
	int fd;

	//Find the size
	struct stat filestatus;
	stat(filename, &filestatus);
	unsigned long filesize = filestatus.st_size;
	int array_length = filesize/sizeof(char);


	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		perror("Error opening file for reading");
		exit(EXIT_FAILURE);
	}

	return (char *)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
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

	uint64_t GetValue() const {
		return value;
	}

	uint64_t value;
};

//Define table
typedef util::ProbingHashTable<Entry, boost::hash<uint64_t> > Table;

int main(int argc, char* argv[]) {
	if (argc != 4) {
		// Tell the user how to run the program
		std::cerr << "Usage: " << argv[0] << " path_to_hashtable path_to_data_bin tablesize" << std::endl;
		return 1;
	}

	int i;
	int fd;
	char *map;  /* mmapped array of int's */

	//Find the size
	struct stat filestatus;
	stat(argv[2], &filestatus);
	unsigned long filesize = filestatus.st_size;
	int array_length = filesize/sizeof(char) - 1; //The end of file has \0, which we don't want to count.
	std::cout << "array_length is " << array_length << std::endl;

	fd = open(argv[2], O_RDONLY);
	if (fd == -1) {
		perror("Error opening file for reading");
		exit(EXIT_FAILURE);
	}

	map = (char *)mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}

	//End of memory mapping code

	//Init the table
	int tablesize = atoi(argv[3]);
	size_t size = Table::Size(tablesize, 1.2);

	//Read it from file
	char * mem = readTable(argv[1], size);
	Table table(mem, size);

	//For searching
	const Entry * tmp;
	uint64_t key;

	//Interactive search
	std::cout << "Please enter a string to be searched, or exit to exit." << std::endl;
	while (true){
		bool found;
		std::string cinstr = "";
		getline(std::cin, cinstr);
		if (cinstr == "exit"){
			break;
		}else{
			//Time lookup
			std::clock_t c_start = std::clock();
			auto t_start = std::chrono::high_resolution_clock::now();

			//Actual lookup
			StringPiece tofind = StringPiece(cinstr);
			key = getHash(cinstr);
			found = table.Find(key, tmp);
			if (found) {
				std::string found(&map[tmp -> GetValue()] , &map[tmp -> GetValue()] + 100);
				std::cout << "Phrase corresponding to " << cinstr << " is:"  << std::endl << found << std::endl;
			} else {
				std::cout << "Key not found!" << std::endl;
			}

			//End timing
			std::clock_t c_end = std::clock();
			auto t_end = std::chrono::high_resolution_clock::now();

			//Print timing results
			std::cout << "CPU time used: "<< 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC<< " ms\n";
			std::cout << "Real time passed: "<< std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count()<< " ms\n";
		}
	}
	//clean up
	munmap(mem, size);
	munmap(map, filesize);
	close(fd);
	util::PrintUsage(std::cout);

	return 0;
}