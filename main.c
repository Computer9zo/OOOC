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

int main(char* argc[], int* argv)//Get argv and argc
{
	//show main display


	disp(_DN_title);
	disp(_DN_manual);

	pause(_DP_pause);
	pause(_DP_done);

	return 0;
}