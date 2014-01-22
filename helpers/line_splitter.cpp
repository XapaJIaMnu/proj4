#include "line_splitter.hh"

line_text splitLine(StringPiece textin) {
	const char delim[] = " ||| ";
	line_text output;

	//Tokenize
	util::TokenIter<util::MultiCharacter> it(textin, util::MultiCharacter(delim));
	//Get source phrase
	output.source_phrase = *it;
	it++;
	//Get target_phrase
	output.target_phrase = *it;
	it++;
	//Get probabilities
	output.prob = *it;
	it++;
	//Get WordAllignment 1
	output.word_all1 = *it;
	it++;
	//Get WordAllignment 2
	output.word_all2 = *it;

	return output;
}


unsigned long getUniqLines(char * filepath){
	//Read the file
	util::FilePiece filein(filepath);

	unsigned long uniq_lines = 0;
	line_text prev_line;

	while (true){
		line_text new_line;
		try {
			//Process line read
			new_line = splitLine(filein.ReadLine());
		} catch (util::EndOfFileException e){
			std::cout << "End of file" << std::endl;
			break;
		}
		if (new_line.source_phrase == prev_line.source_phrase){
			continue;
		} else {
			uniq_lines++;
			prev_line = new_line;
		}
	}

	return uniq_lines;
} 

std::pair<std::vector<char>::iterator, int> vector_append(line_text* input, std::vector<char>* outputvec, std::vector<char>::iterator it, bool new_entry){
	//Append everything to one string
	std::string temp = "";
	int vec_size = 0;
	int new_string_size = 0;

	if (new_entry){
		//If we have a new entry, add an empty line.
		temp += '\n';
	}

	temp += input->target_phrase.as_string() + "\t" + input->prob.as_string() + "\t" + input->word_all1.as_string() + "\t" + input->word_all2.as_string() + "\n";
	
	//Put into vector
	outputvec->insert(it, temp.begin(), temp.end());

	//Return new iterator updated iterator
	//Return iterator + length
	std::pair<std::vector<char>::iterator, int> retvalues (it+temp.length(), temp.length());
	return retvalues;
}

std::vector<double> splitProbabilities(StringPiece textin){
	const char delim[] = " ";
	std::vector<double> output;

	//Split on space
	util::TokenIter<util::MultiCharacter> it(textin, util::MultiCharacter(delim));

	//For each double
	while (it) {
		output.push_back(atof(it->data()));
		it++;
	}

	return output;
}

std::vector<int> splitWordAll1(StringPiece textin){
	const char delim[] = " ";
	const char delim2[] = "-";
	std::vector<int> output;

	//Split on space
	util::TokenIter<util::MultiCharacter> it(textin, util::MultiCharacter(delim));

	//For each int
	while (it) {
		//Split on dash (-)
		util::TokenIter<util::MultiCharacter> itInner(*it, util::MultiCharacter(delim2));

		//Insert the two entries in the vector. User will read entry 0 and 1 to get the first,
		//2 and 3 for second etc.
		output.push_back(atoi(itInner->data()));
		itInner++;
		output.push_back(atoi(itInner->data()));
		it++;
	}

	return output;

}

std::vector<int> splitWordAll2(StringPiece textin){
	const char delim[] = " ";
	std::vector<int> output;

	//Split on space
	util::TokenIter<util::MultiCharacter> it(textin, util::MultiCharacter(delim));

	//For each int
	while (it) {
		output.push_back(atoi(it->data()));
		it++;
	}

	return output;

}

target_text splitSingleTargetLine(StringPiece textin){
	const char delim[] = "\t";
	target_text output;

	//Split on elements:
	util::TokenIter<util::MultiCharacter> it(textin, util::MultiCharacter(delim));

	//Get target phrase
	output.target_phrase = getVocabIDs(*it);
	it++;

	//Get probabilities
	output.prob = splitProbabilities(*it);
	it++;

	//Get wordall1
	output.word_all1 = splitWordAll1(*it);
	it++;

	//Get wordall2
	output.word_all2 = splitWordAll2(*it);

	return output;

}

std::vector<target_text> splitTargetLine(StringPiece textin){
	const char delim[] = "\n\n";
	const char delim2[] = "\n";
	std::vector<target_text> output;
	StringPiece all_entries;

	//Split on two new lines, get the whole entry
	util::TokenIter<util::MultiCharacter> it(textin, util::MultiCharacter(delim));
	all_entries = *it;

	//Split on new line, split multiple target lines
	util::TokenIter<util::MultiCharacter> it2(*it, util::MultiCharacter(delim2));
	while (it2){
		if (it2->size() < 3){
			//Skip empty lines. Necessary for last line.
			it2++;
			continue;
		}
		output.push_back(splitSingleTargetLine(*it2));
		it2++;
	}

	return output;
}