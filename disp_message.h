/*----------------------------------------------------*
/* disp_message.h
/* 17-06-26 21:32
/*----------------------------------------------------*
/* What is disp_message do?
/* - Show awesome texts and graphs
/* - example) main screen, program manual.
/* - Pause program until press enter
/* - Show error messages
/*----------------------------------------------------*/

#ifndef __DISP_MESSAGE__
#define __DISP_MESSAGE__

#include <stdio.h>
#include <stdlib.h>

//print title, program status and error message
#define disp(message) printf(message); 

//print one blank line
#define blank() printf("\n");

//print pause and pause progrma. it include get_char();
#define pause(message)                                 \
{                                                      \
	printf(message);                                   \
	getchar();                                         \
    fseek(stdin, 0, SEEK_END);                         \
}

//draw bar graph with percent
#define graph(length,percent)                          \
{	                                                   \
    printf("[");                                       \
	for (int len = (length); len > 0; --len)           \
	{                                                  \
		if (len > (1 - ((percent) / 100.0))*(length))  \
		{                                              \
			printf("|");                               \
		}                                              \
		else                                           \
		{                                              \
			printf(" ");                               \
		}                                              \
	}                                                  \
	printf("]");                                       \
	printf("%3d%%", (length));                         \
}
			

/*---------Messages---------*/

/*Display Normal*/

//program title
#define _DN_title                                      \
"+-----------------------------------------------+\n"  \
"|   Computer Architecture #3                    |\n"  \
"|   Out of Order with Cache                     |\n"  \
"|   Noh Dong Ju & Park Kyung Won & Yu Jin woo   |\n"  \
"+-----------------------------------------------+\n"  \
" \n"

//program manual
#define _DN_manual                                     \
" Usage                                           \n"  \
" Give arguments under 2 type.                    \n"  \
" 1)'program.exe <config_file> <instruction_file>'\n"  \
" 2)'program.exe <config_file> <inst_1> <inst_2>' \n"  \
" First one just run one thread in OoO with C,    \n"  \
" Second one run two threads same time in OoOC.   \n"  \
"                                                 \n"  \
" After run, program will save report under form. \n"  \
" './<config_name>_<inst_name1>_<inst_name2>.out' \n"  \
" if you not use inst_2, it will replace 'None'   \n"  \
" \n"

//display reading file name
#define _DN_read_file                                  \
" Programe will read these files.                 \n"

//display reading file name
#define _DN_read_config_file(config_name)              \
" Config         : %39s\n", config_name

//display reading file name
#define _DN_read_instruction_file(num,inst_name)       \
" Instruction %d : %-39s\n", num, inst_name




/*Display Pause*/

//pause
#define _DP_pause                                      \
" Press enter to continue.                        \n"

//done
#define _DP_done                                       \
" Press enter to quit.                            \n"




/*Display Error*/



#endif // !__DISP_MESSAGE__
