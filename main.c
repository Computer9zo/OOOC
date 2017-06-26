/*----------------------------------------------------*
/* main.c
/* 17-06-26 14:27
/*----------------------------------------------------*
/* What is main do?
/* - Show awesome main screen
/* - Get program argument (argc, argv) and load files.
/* - Just run simulation
/* - Print and save report
/*----------------------------------------------------*/


//display every message in program
#include "disp_message.h"
#include "data_structure.h"
int main(int argv, char* argc[])//Get argv and argc
{
	//show main display
	disp(_DN_title);

	//if there too few argument, disp manual and quit
	if (argv < 4 || argv > 5)
	{
		disp(_DN_manual);
		pause(_DP_done);
		return 1;
	}

	//print name of config ans insts files
	disp(_DN_read_file);
	disp(_DN_read_config_file(argc[1]));
	for (int inst_num = 2; inst_num < argv; ++inst_num)
	{
		disp(_DN_read_instruction_file(inst_num-1,argc[inst_num]));
	}

	//read file and print graph and percentage of reading file task.
	


	return 0;
}