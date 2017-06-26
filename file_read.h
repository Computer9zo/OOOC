/*----------------------------------------------------*
/* file_read.h
/* 17-06-26 22:00
/*----------------------------------------------------*
/* What is file_read do?
/* - Read Config and Instruction files. 
/* - And print it's percentage.
/* - Give Util function about file name.
/*----------------------------------------------------*/

#ifndef _FILE_READ_
#define _FILE_READ_
//for prevent multifle include

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structure.h"



//inst file reading
bool read_instruction(FILE *in_filename, struct INST * out_inst);// bool is_readed, read one line and convert INST form
bool make_inst_array(char* filename, struct INST *** out_inst_arr, int **out_len);// bool is_readed read whole file and convert INST array

//config file reading
bool config_reader(char* filename, struct CONFIG *out_config);// bool is_readed, read whole file and convert Config file

#endif // !_FILE_READ_