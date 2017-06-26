#ifndef __DISP_MESSAGE__
#define __DISP_MESSAGE__

#include <stdio.h>
#include <stdlib.h>

//printf title, program status and error message
#define disp(message) printf("%s",message); 

//print pause and pause progrma. it include get_char();
#define pause(message)                                 \
{                                                      \
	printf(message);                                   \
	getchar();                                         \
    fseek(stdin, 0, SEEK_END);                         \
}



/*---------Messages---------*/

/*Display Normal*/

//program title
#define _DN_title                                      \
"+-----------------------------------------------+\n"##\
"|   Computer Architecture #3                    |\n"##\
"|   Out of Order with Cache                     |\n"##\
"|   Noh Dong Ju & Park Kyung Won & Yu Jin woo   |\n"##\
"+-----------------------------------------------+\n"##\
" \n"

//program manual
#define _DN_manual                                     \
" Usage                                           \n"##\
" Give arguments under 2 type.                    \n"##\
" 1)'program.exe <config_file> <instruction_file>'\n"##\
" 2)'program.exe <config_file> <inst_1> <inst_2>' \n"##\
" First one just run one thread in OoO with C,    \n"##\
" Second one run two threads same time in OoOC.   \n"##\
"                                                 \n"##\
" After run, program will save report under form. \n"##\
" './<config_name>_<inst_name1>_<inst_name2>.out' \n"##\
" if you not use inst_2, it will replace 'None'   \n"##\
" \n"

/*Display Pause*/

//pause
#define _DP_pause                                      \
" Press enter to continue.                        \n"

//done
#define _DP_done                                       \
" Press enter to quit.                            \n"


#endif // !__DISP_MESSAGE__
