/**
 * Project 2
 * LC-2K Linker
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAXSIZE 500
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct inone{
	int number;
	int file;
	int line;
};
struct inone scroll[MAXLINELENGTH];
struct offset{
	int offset;
};
struct offset offset[MAXLINELENGTH];

struct SymbolTableEntry {
	char label[7];
	char location;
	unsigned int offset;
	int file;
};

struct RelocationTableEntry {
    unsigned int file;
	unsigned int offset;
	char inst[6];
	char label[7];
};

struct FileData {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	unsigned int textStartingLine; // in final executable
	unsigned int dataStartingLine; // in final executable
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	unsigned int textSize;
	unsigned int dataSize;
	unsigned int symbolTableSize;
	unsigned int relocationTableSize;
	int text[MAXSIZE * MAXFILES];
	int data[MAXSIZE * MAXFILES];
	SymbolTableEntry symbolTable[MAXSIZE * MAXFILES];
	RelocationTableEntry relocTable[MAXSIZE * MAXFILES];
};

int main(int argc, char *argv[]) {
	char *inFileStr, *outFileStr;
	FILE *inFilePtr, *outFilePtr; 
	unsigned int i, j;

    if (argc <= 2 || argc > 8 ) {
        printf("error: usage: %s <MAIN-object-file> ... <object-file> ... <output-exe-file>, with at most 5 object files\n",
				argv[0]);
		exit(1);
	}

	outFileStr = argv[argc - 1];

	outFilePtr = fopen(outFileStr, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileStr);
		exit(1);
	}

	FileData files[MAXFILES];

  // read in all files and combine into a "master" file
	for (i = 0; i < argc - 2; ++i) {
		inFileStr = argv[i+1];

		inFilePtr = fopen(inFileStr, "r");
		printf("opening %s\n", inFileStr);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileStr);
			exit(1);
		}

		char line[MAXLINELENGTH];
		unsigned int textSize, dataSize, symbolTableSize, relocationTableSize;

		// parse first line of file
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&textSize, &dataSize, &symbolTableSize, &relocationTableSize);

		files[i].textSize = textSize;
		files[i].dataSize = dataSize;
		files[i].symbolTableSize = symbolTableSize;
		files[i].relocationTableSize = relocationTableSize;

		// read in text section
		int instr;
		for (j = 0; j < textSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = strtol(line, NULL, 0);
			files[i].text[j] = instr;
		}

		// read in data section
		int data;
		for (j = 0; j < dataSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = strtol(line, NULL, 0);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		unsigned int addr;
		for (j = 0; j < symbolTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
			files[i].symbolTable[j].file = i;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < relocationTableSize; ++j) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files
	

	//file[0].data will give you the array for all the data (5 for the .fill)
	CombinedFiles combined;
	int num_files = argc - 2;
	int tableSize = 0;
	/*for(int i = 0; i < num_files; i++){ // for putting all Globals in the same struck
		for(int j = 0; j < files[i].symbolTableSize; j++){
			if(files[i].symbolTable[j].location != 'U'){
				if(strcmp(combined.symbolTable[j].label, files[i].symbolTable[j].label)){
					strcpy(combined.symbolTable[tableSize].label, files[i].symbolTable[j].label);
					combined.symbolTable[tableSize].location = files[i].symbolTable[j].location;
					combined.symbolTable[tableSize].offset = files[i].symbolTable[j].offset;
					combined.symbolTable[tableSize].file = files[i].symbolTable[j].file;
					tableSize++;
				}
			}
		}
	}*/
	//int symbolSIZE = tableSize;
	// reloction table
	tableSize = 0;
	for(int i = 0; i < num_files; i++){ // for putting all relocation in the same struck
		for(int j = 0; j < files[i].relocationTableSize; j++){
			strcpy(combined.relocTable[tableSize].inst, files[i].relocTable[j].inst);
			strcpy(combined.relocTable[tableSize].label, files[i].relocTable[j].label);
			combined.relocTable[tableSize].offset = files[i].relocTable[j].offset;
			combined.relocTable[tableSize].file = files[i].relocTable[j].file;
			tableSize++;
		}
	}
	int relocSIZE = tableSize;
	int list_count = 0;
	int file_tracker = 0;
	int text_tracker = 0;
	int data_tracker = 0;
	for(int i = 0; i < num_files; i++){ //putting the text in order
		file_tracker = 0;
		text_tracker = 0;
		while(file_tracker < files[i].textSize){
			scroll[list_count].number = files[i].text[text_tracker];
			scroll[list_count].line = file_tracker;
			scroll[list_count].file = i;
			list_count++;
			file_tracker++;
			text_tracker++;
		}
	}
	for(int j = 0; j < num_files; j++){ // putting the data in order
		file_tracker = 0;
		data_tracker = 0;
		while(file_tracker < files[j].dataSize){
			scroll[list_count].number = files[j].data[data_tracker];
			scroll[list_count].line = file_tracker;
			scroll[list_count].file = j;
			list_count++;
			file_tracker++;
			data_tracker++;
		}
	}
	//int dataSIZE = symbolSIZE + relocSIZE;
	int sumTextFiles = 0;
	for(int x = 0; x < num_files; x++){
		sumTextFiles = sumTextFiles + files[x].textSize;
	}
	int sumDataFiles = 0;
	for(int x = 0; x < num_files; x++){
		sumDataFiles = sumDataFiles + files[x].dataSize;
	}
	int totalsize = sumDataFiles + sumTextFiles;
	// updating the symbol table
	bool breaks = false;
	/*int textsize = 0;
	int textsizeD = 0;
	int right_offset = 0;
	int saved_line2;
	int offset_next = 0;
	for(int s = 0; s < symbolSIZE; s++){
		int current_line = combined.symbolTable[s].offset;
		int current_file = combined.symbolTable[s].file;
		char letter = combined.symbolTable[s].location;
		int line_counter = 0;
		breaks = false;
		if(letter == 'D'){
			line_counter = sumTextFiles + 1;
			textsizeD = sumTextFiles;
			for(int a = 0; a < num_files; a++){
				for(int b = 0; b < files[a].dataSize; b++){
					if(scroll[line_counter].line == current_line && scroll[line_counter].file == current_file){
						offset[s].offset = line_counter + 1; // fix!!!!
						saved_line2 = line_counter;
						breaks = true;
					}
					line_counter++;
					if(breaks){ break; }
				}
				if(breaks){ break; }
			}
			right_offset = (sumTextFiles + textsizeD) - offset[s].offset;
			textsize++;
			textsizeD++;
			scroll[saved_line2].number = scroll[saved_line2].number + right_offset;
		}else{ // letter == 'T'
			for(int i = 0; i < num_files; i++){
				for(int j = 0; j < files[i].symbolTableSize; j++){
					if(scroll[j].line == current_line){
						offset[s].offset = scroll[line_counter].number & 0XFFFF;
						saved_line2 = line_counter;
						breaks = true;
					}
					line_counter++;
					textsize++;
					if(breaks){ break; }
				}
				if(breaks){ break; }
			}
			right_offset = (sumTextFiles + textsize) - offset[s].offset;
			textsize++;
			scroll[saved_line2].number = scroll[saved_line2].number + right_offset;
		}
		offset_next = s;
	}*/
	//updating the relocation table
	int datasize = 0;
	int right_offsetr = 0;
	int saved_line;
	for(int r = 0; r < relocSIZE; r++){
		breaks = false;
		int current_line = combined.relocTable[r].offset;
		int current_file = combined.relocTable[r].file;
		int line_counter = 0;
		int fill_line = 0;
		if(isupper(combined.relocTable[r].label[0])){
			for(int j = 0; j < totalsize; j++){
				if(scroll[line_counter].line == current_line && scroll[line_counter].file == current_file){
					offset[r].offset = scroll[line_counter].number & 0XFFFF;
					saved_line = line_counter;
					breaks = true; 
				}
				line_counter++;
				if(breaks){ break; }
			}
			right_offsetr = (sumTextFiles + line_counter) - offset[r].offset;
			scroll[saved_line].number = scroll[saved_line].number + right_offsetr;
		}else{
			if(!strcmp(combined.relocTable[r].inst, ".fill")){
				line_counter = sumTextFiles;
				for(int i = 0; i < num_files; i++){
					for(int j = 0; j < files[i].dataSize; j++){
						if(scroll[line_counter].line == current_line && scroll[line_counter].file == current_file){
							saved_line = line_counter;
							for(int x = 0; x < num_files; x++){
								for(int y = 0; y < files[x].textSize; y++){
									if(scroll[fill_line].line == current_line && scroll[fill_line].file == current_file){
										offset[r].offset = fill_line - 1;
										breaks = true;
									}
									fill_line++;
									if(breaks){ break; }
								}
								if(breaks){ break; }
							}
						}
						line_counter++;	
						if(breaks){ break; }
					}
					if(breaks){ break; }
				}
				right_offsetr = (sumTextFiles + r) - offset[r].offset;
				datasize++;
				scroll[saved_line].number = scroll[saved_line].number + right_offsetr;
			}else{
				for(int i = 0; i < num_files; i++){ // for finding the right spot in scroll[].line
					for(int j = 0; j < files[i].textSize; j++){
						if(scroll[line_counter].line == current_line && scroll[line_counter].file == current_file){
							offset[r].offset = scroll[line_counter].number & 0XFFFF;
							saved_line = line_counter;
							breaks = true;
						}
						line_counter++;
						if(breaks){ break; }
					}
					if(breaks){ break; }
				}
				// this is how to find offset
				// add the text file size to the data soze that the lable is at to find the offset
				// than 9 - 6 = 3. add the 3 to the value given in the obj (for the first one)
				right_offsetr = (sumTextFiles + datasize) - offset[r].offset;
				datasize++;
				scroll[saved_line].number = scroll[saved_line].number + right_offsetr;
			}
		}
	}
	
	for(int m = 0; m < totalsize; m++){
		fprintf(outFilePtr, "%d\n", scroll[m].number);
	}
} // main
