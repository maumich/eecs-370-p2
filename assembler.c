/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000
struct mylabel{
    char name[7];
    int addy;
};
struct mylabel labels[MAXLINELENGTH];

struct WorldWide{
    char ID[7];
    char home;
    char op[7];
};
struct WorldWide Global[MAXLINELENGTH];

int pc = 0;
int Text = 0; // used in the header. The number of instruction above .fill
int Data = 0; // used in the header. The number of .fill
int Symbol_table = 0; // useed in the header. Mapping of symbols
int Relocation_Table = 0; // in the header. Any address that may shift
int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
int R_type_instruction(int opcode, int arg0, int arg1, int arg2, FILE*outfileptr){
    opcode = opcode << 22; //shifting bit 
    arg0 = arg0 << 19;
    arg1 = arg1 << 16;
    //arg2 is destR
    int mc = opcode | arg0 | arg1 | arg2;
    //this "|" is called bitwise or
    fprintf(outfileptr, "%d\n", mc);
    return 0;
   }

int I_type_instruction(int opcode, int arg0, int arg1, int arg2, FILE*outfileptr){
    opcode = opcode << 22;
    arg0 = arg0 << 19; //regA
    arg1 = arg1 << 16; //regB
    //arg2 = arg2 >> 15; // 32 bit by deflaut needs to be 16
    if(arg2 > 32767 || arg2 < -32768){ // if the offset is bigger than 16 bits (2^16)
        exit(1);
    }
    arg2 = arg2 & 0XFFFF;
    int mc = opcode | arg0 | arg1 | arg2;
    fprintf(outfileptr, "%d\n", mc);
    return 0;
}

int J_type_instruction(int opcode, int arg0, int arg1, FILE*outfileptr){
    opcode = opcode << 22;
    arg0 = arg0 << 19; //regA
    arg1 = arg1 << 16;
    int mc = opcode | arg0 | arg1;
    fprintf(outfileptr, "%d\n", mc);
    return 0;
}

int O_type_instruction(int opcode, FILE*outfileptr){
    opcode = opcode << 22;
    fprintf(outfileptr, "%d\n", opcode);
    return 0;
}

int reg_check(char *arg){
    int arg_num;
    if(isNumber(arg)){
            arg_num = atoi(arg);
        }else{
            exit(1);
        }
    if(arg_num < 0 || arg_num > 7){
            exit (1);
        }
    return arg_num;
}

int arg2_as_int(char *arg2, int num_of_labels, int opcode){
   /*three conditions:
    1) arg2 is a number
    2) arg2 is a label and we are dealing with a branch
    3) arg2 is a label and we are not dealing with a branch*/
    int arg2_int = -2;
    bool flippy = true;
    bool GlobalFind = false;
    if(isNumber(arg2)){
           arg2_int = atoi(arg2); 
           flippy = false;
        } else{
            if(opcode == 4){
                arg2_int = 0;
                int x = 0;
                int offset = arg2_int -(pc + 1);
                int arg0_int = 0;
                for(; x < num_of_labels; x++){
                    if(!strcmp(labels[x].name, arg2)){
                        arg0_int = labels[x].addy;
                        break;
                    }  
                }
                if(x == num_of_labels){
                    exit(1);
                }
                flippy = false;
                return offset + arg0_int; 
            }
            if(isupper(arg2[0])){
                for(int i = 1; i < Symbol_table + 1; i++){
                    if(Global[i].home == 'U' && !strcmp(Global[i].ID, arg2)){
                        arg2_int = 0;
                        flippy = false;
                        GlobalFind = true;
                        break;
                    }
                }
            }
            if(!GlobalFind){
                for(int i = 0; i < num_of_labels; i++){
                    if(!strcmp(labels[i].name, arg2)){
                        arg2_int = labels[i].addy;
                        flippy = false;
                        break;
                    }
                
                }
            }
            
        }
        if(flippy == true || arg2_int < 0){ //undefine label
            exit(1);
        }
    return arg2_int;
}
int main(int argc, char **argv){
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    int linecounter = 0;
    int num_of_labels = 0;
    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) { //first pass
        if(isupper(label[0])){ //for finding and dealing with global
            if(Symbol_table == 0){//for the first Globla found
                Symbol_table++; 
                strcpy(Global[Symbol_table].ID, label);
                if(!strcmp(opcode, ".fill")){
                    Global[Symbol_table].home = 'D';
                    strcpy(Global[Symbol_table].op, opcode);
                }else{
                    Global[Symbol_table].home = 'T';
                    strcpy(Global[Symbol_table].op, opcode);
                }
            }else{//if not first
                if(strcmp(Global[Symbol_table].ID, label)){//if not first and not 'U'
                    Symbol_table++; 
                    strcpy(Global[Symbol_table].ID, label);
                    if(!strcmp(opcode, ".fill")){
                        Global[Symbol_table].home = 'D';
                        strcpy(Global[Symbol_table].op, opcode);
                    }else{
                        Global[Symbol_table].home = 'T';
                        strcpy(Global[Symbol_table].op, opcode);
                    } 
                }
            }
            
        }
        if(isupper(arg2[0])){ //checking to see if the Global is in arg2 ('U')
            for(int q = 0; q < Symbol_table + 1;){
                if(strcmp(arg2, Global[q].ID)){
                    Symbol_table++;
                    strcpy(Global[Symbol_table].ID, arg2);
                    Global[Symbol_table].home = 'U';
                    q++;
                    break; 
                }
            }
        }
        if(isupper(arg0[0])){ //checking to see if the Global is in arg2 ('U')
            for(int q = 0; q < Symbol_table + 1;){
                if(strcmp(arg0, Global[q].ID)){
                    Symbol_table++;
                    strcpy(Global[Symbol_table].ID, arg0);
                    Global[Symbol_table].home = 'U';
                    q++;
                    break; 
                }
            }
        }
        if(label[0] != '\0'){ //Normal label checking from p1. Globals also go here
            strcpy(labels[num_of_labels].name, label);
            labels[num_of_labels].addy = linecounter;
            num_of_labels++;
        }
      
       if(strcmp(opcode, ".fill")){
         Text++; // for header
        }
        if(!strcmp(opcode, "lw") && !isNumber(arg2)){
           Relocation_Table++;
        }else if(!strcmp(opcode, "sw") && !isNumber(arg2)){
            Relocation_Table++;
        }else if(!strcmp(opcode, ".fill") && !isNumber(arg0)){
            Relocation_Table++;
        }
        linecounter++;
    }
    for(int a = 0; a < num_of_labels - 1; a++){ //error checking for dup labels(p1)
        for(int b = a + 1; b < num_of_labels; b++){
            if(!strcmp(labels[a].name, labels[b].name)){
                exit(1);
            }
        }
    }
    int ogbol = Symbol_table;
    for(int z = 0; z < ogbol; z++){
        for(int q = 1; q < Symbol_table; q++){
            for(int u = q + 1; u < Symbol_table + 1; u++){
                if(!strcmp(Global[q].ID, Global[u].ID)){ //Fixing if the Global is 'U' to 'D' or 'T' 
                    if(Global[q].home == 'U'){
                        Global[q].home = Global[u].home;
                        strcpy(Global[q].ID, Global[u].ID);
                        strcpy(Global[q].op, Global[u].op);
                        if(!strcmp(Global[u].op, ".fill")){
                            Global[u].home = 'D';
                        }else{
                            Global[u].home = 'T';
                        }
                    }else{
                    if(!strcmp(Global[q].op, ".fill")){
                            Global[q].home = 'D';
                        }else{
                            Global[q].home = 'T';
                        } 
                    }
                    for(int i = u; i < Symbol_table; i++){
                        strcpy(Global[i].ID, Global[i + 1].ID);
                        strcpy(Global[i].op, Global[i + 1].op);
                        Global[i].home = Global[i + 1].home;
                    }
                    Symbol_table--;
                }
            }
        }
    }
    Data = linecounter - Text; //finding data for header
    //Symbol_table = Symbol_table - 2;
    fprintf(outFilePtr, "%d %d %d %d\n", Text, Data, Symbol_table, Relocation_Table); //printing the header
    rewind(inFilePtr);
    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)) { //second pass
      if(!strcmp(opcode, "add")){
        int opcode_int = 0;
        int arg0_int = reg_check(arg0);
        int arg1_int = reg_check(arg1);
        int arg2_int = arg2_as_int(arg2, num_of_labels, opcode_int); // for making arg2 a function
        R_type_instruction(opcode_int, arg0_int, arg1_int, arg2_int, outFilePtr);
        pc++;
      }
      else if(!strcmp(opcode, "nor")){
        int opcode_int = 1;
        int arg0_int = reg_check(arg0);
        int arg1_int = reg_check(arg1);
        int arg2_int = arg2_as_int(arg2, num_of_labels, opcode_int);
        R_type_instruction(opcode_int, arg0_int, arg1_int, arg2_int, outFilePtr);
        pc++;
      }
      else if(!strcmp(opcode, "lw")){
        int opcode_int = 2;
        int arg0_int = reg_check(arg0);
        int arg1_int = reg_check(arg1);
        int arg2_int = arg2_as_int(arg2, num_of_labels, opcode_int);
        I_type_instruction(opcode_int, arg0_int, arg1_int, arg2_int, outFilePtr);
        pc++;
      }
      else if(!strcmp(opcode, "sw")){
        int opcode_int = 3;
        int arg0_int = reg_check(arg0);
        int arg1_int = reg_check(arg1);
        int arg2_int = arg2_as_int(arg2, num_of_labels, opcode_int);
        I_type_instruction(opcode_int, arg0_int, arg1_int, arg2_int, outFilePtr);
        pc++;
      }
      else if(!strcmp(opcode, "beq")){
        int opcode_int = 4;
        int arg0_int = reg_check(arg0);
        int arg1_int = reg_check(arg1);
        int arg2_int = arg2_as_int(arg2, num_of_labels, opcode_int);
        I_type_instruction(opcode_int, arg0_int, arg1_int, arg2_int, outFilePtr);
        pc++;
      }
      else if(!strcmp(opcode, "jalr")){
        int opcode_int = 5;
        int arg0_int = reg_check(arg0);
        int arg1_int = reg_check(arg1);
        J_type_instruction(opcode_int, arg0_int, arg1_int, outFilePtr);
        pc++;
      }
      else if(!strcmp(opcode, "halt")){
        int opcode_int = 6;
        O_type_instruction(opcode_int, outFilePtr);
        pc++;
      }
      else if(!strcmp(opcode, "noop")){
       int opcode_int = 7;
       O_type_instruction(opcode_int, outFilePtr);
       pc++;
      }
      else if(!strcmp(opcode, ".fill")){
        if(isNumber(arg0)){
            int arg0_int = atoi(arg0);
            fprintf(outFilePtr, "%d\n", arg0_int);
        }else{
            int arg0_int = 0;
            for(int i = 0; i < num_of_labels; i++){
                if(!strcmp(labels[i].name, arg0)){
                    arg0_int = labels[i].addy;
                    break;
                }  
            }
            fprintf(outFilePtr, "%d\n", arg0_int);
        }
        pc++;
      }else{ // for error checking unrecognized opcodes
        exit(1);
      }

    }
    //printing the Symbol Table
    int fill_count = -1;
    int text_count = -1;
    for(int h = 1; h < Symbol_table + 1; h++){
       if(Global[h].home == 'D'){
        fill_count = -1;
        rewind(inFilePtr);
        while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
            if(!strcmp(opcode, ".fill")){
                fill_count++;
            }
            if(!strcmp(Global[h].ID, label)){
                break;
            }
        }
        fprintf(outFilePtr, "%s D %d\n", Global[h].ID, fill_count);
       }
       else if(Global[h].home == 'T'){
           text_count = -1;
           rewind(inFilePtr);
           while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
                if(strcmp(arg0, ".fill")){
                 text_count++;
                }
                if(!strcmp(Global[h].ID, label)){
                break;
            }
            }
            fprintf(outFilePtr, "%s T %d\n", Global[h].ID, text_count);
        }else{
            fprintf(outFilePtr, "%s U 0\n", Global[h].ID);
        }

    }
    int lineCNT = 0;
    int fillins = 0;
    rewind(inFilePtr);
    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){//third pass for setting relcation tabel
        //Relcoation table
        if(!strcmp(opcode, "lw") && !isNumber(arg2)){
            fprintf(outFilePtr, "%d lw %s\n", lineCNT, arg2);
        }else if(!strcmp(opcode, "sw") && !isNumber(arg2)){
            fprintf(outFilePtr, "%d sw %s\n", lineCNT, arg2);
        }else if(!strcmp(opcode, ".fill") && !isNumber(arg0)){
            fprintf(outFilePtr, "%d .fill %s\n", fillins, arg0);
        }
        if(!strcmp(opcode, ".fill")){
            fillins++;
        }
        lineCNT++;
    }
    
    //NOTE isnumber(0) for checking if the string is a just a number
    return(0);
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}


/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}
