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
struct dup{
	int offset;
	int file;
};
struct dup dup[MAXLINELENGTH];

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
	for(int i = 0; i < num_files; i++){ // for putting all Globals in the same struck
		for(int j = 0; j < files[i].symbolTableSize; j++){
			strcpy(combined.symbolTable[tableSize].label, files[i].symbolTable[j].label);
			combined.symbolTable[tableSize].location = files[i].symbolTable[j].location;
			combined.symbolTable[tableSize].offset = files[i].symbolTable[j].offset;
			combined.symbolTable[tableSize].file = files[i].symbolTable[j].file;
			tableSize++;
		}
	}
	int og_symbolSIZE = tableSize;
	for(int z = 0; z < og_symbolSIZE; z++){
		for(int q = 0; q < tableSize; q++){
			for(int u = q + 1; u < tableSize + 1; u++){
				if(!strcmp(combined.symbolTable[q].label, combined.symbolTable[u].label)){
					if(combined.symbolTable[q].location == 'U'){
						strcpy(combined.symbolTable[q].label, combined.symbolTable[u].label);
						combined.symbolTable[q].location = combined.symbolTable[u].location;
						combined.symbolTable[q].offset = combined.symbolTable[u].offset;
						combined.symbolTable[q].file = combined.symbolTable[q].file;
					}
					for(int i = u; i < tableSize; i++){
						strcpy(combined.symbolTable[i].label, combined.symbolTable[i + 1].label);
						combined.symbolTable[i].location = combined.symbolTable[i + 1].location;
						combined.symbolTable[i].offset = combined.symbolTable[i + 1].offset;
						combined.symbolTable[i].file = combined.symbolTable[i + 1].file;
					}
					tableSize--;
				}
			}
			
		}
	}
	//checking to see if anything is 'U'
	for(int n = 0; n < tableSize; n++){
		if(combined.symbolTable[n].location == 'U'){
			exit(1);
		}
	}
	int symbolsize = tableSize;
	int current = 0;
	tableSize = 0;
	for(int i = 0; i < num_files; i++){ // for putting all relocation in the same struck
		for(int j = 0; j < files[i].relocationTableSize; j++){
			strcpy(combined.relocTable[tableSize].inst, files[i].relocTable[j].inst);
			strcpy(combined.relocTable[tableSize].label, files[i].relocTable[j].label);
			combined.relocTable[tableSize].offset = files[i].relocTable[j].offset;
			combined.relocTable[tableSize].file = files[i].relocTable[j].file;
			tableSize++;
			current++;
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
	//updating the relocation table
	bool breaks = false;
	int offset;
	int datasize = 0;
	int right_offsetr = 0;
	int saved_line;
	int rightline = 0;
	int image = 0;
	for(int r = 0; r < relocSIZE; r++){
		breaks = false;
		int current_line = combined.relocTable[r].offset;
		int current_file = combined.relocTable[r].file;
		int line_counter = 0;
		int fill_line = 0;
		if(isupper(combined.relocTable[r].label[0])){
			int globalline = 0;
			bool symbolB = false;
			image = 0;
			for(int a = 0; a < symbolsize; a++){
				if(!strcmp(combined.relocTable[r].label, combined.symbolTable[image].label)){
					// just need the number from image, nothing else to do here
					symbolB = true;
				}
				if(symbolB){ break; }
				image++;
			}
			if(!strcmp(combined.relocTable[r].inst, ".fill")){
				breaks = false;
				line_counter = sumTextFiles;
				for(int i = 0; i < num_files; i++){
					for(int j = 0; j < files[i].dataSize; j++){
						if(scroll[line_counter].line == current_line && scroll[line_counter].file == current_file){
							saved_line = line_counter;
							line_counter = sumTextFiles;
							for(int d = 0; d < num_files; d++){
								for(int e = 0; e < files[d].dataSize; e++){
									if(scroll[line_counter].line == combined.symbolTable[image].offset && scroll[line_counter].file == combined.symbolTable[image].file){
										// this if statment will tell us the line the global is define on
										rightline = globalline;
										breaks = true;
									}
								if(breaks){ break; }
									line_counter++;
									globalline++;
								}
							  if(breaks){ break; }
							}
						}
					  line_counter++;	
					  if(breaks){ break; }
					}
					if(breaks){ break; }
				}
				right_offsetr = (sumTextFiles + rightline) - offset;
				scroll[saved_line].number = scroll[saved_line].number + right_offsetr;
			}else{
				if(combined.symbolTable[image].location == 'T'){
					breaks = false;
					for(int b = 0; b < sumTextFiles; b++){
						if(scroll[line_counter].line == combined.symbolTable[image].offset && scroll[line_counter].file == combined.symbolTable[image].file){
							rightline = line_counter;
							breaks = true;
						}
						if(breaks){ break; }
						line_counter++;
					}
				}else{ // == 'D'
					line_counter = sumTextFiles;
					breaks = false;
					for(int d = 0; d < num_files; d++){
						for(int e = 0; e < files[d].dataSize; e++){
							if(scroll[line_counter].line == combined.symbolTable[image].offset && scroll[line_counter].file == combined.symbolTable[image].file){
								// this if statment will tell us the line the global is define on
								rightline = globalline;
								breaks = true;
							}
							if(breaks){ break; }
							line_counter++;
							globalline++;
						}
						if(breaks){ break; }
					}
				}
				breaks = false;
				line_counter = 0;
				for(int j = 0; j < totalsize; j++){
					if(scroll[line_counter].line == current_line && scroll[line_counter].file == current_file){
						offset = scroll[line_counter].number & 0XFFFF;
						saved_line = line_counter;
						fill_line = sumTextFiles;
						breaks = true;
					}	
					line_counter++;
					if(breaks){ break; }
				}
				right_offsetr = (sumTextFiles + rightline) - offset;//check this line
				//datasize++;
				scroll[saved_line].number = scroll[saved_line].number + right_offsetr;
			}
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
										offset = fill_line - 1;
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
				right_offsetr = (sumTextFiles + r) - offset;
				datasize++;
				scroll[saved_line].number = scroll[saved_line].number + right_offsetr;
			}else{
				for(int i = 0; i < num_files; i++){ // for finding the right spot in scroll[].line
					for(int j = 0; j < files[i].textSize; j++){
						if(scroll[line_counter].line == current_line && scroll[line_counter].file == current_file){
							offset = scroll[line_counter].number & 0XFFFF;
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
				right_offsetr = (sumTextFiles + datasize) - offset;
				datasize++;
				scroll[saved_line].number = scroll[saved_line].number + right_offsetr;
			}
		}
	}
	
	for(int m = 0; m < totalsize; m++){
		fprintf(outFilePtr, "%d\n", scroll[m].number);
	}
} // main
