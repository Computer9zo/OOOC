// Computer Architecture Homework #3
// Noh Dong Ju & Park Kyung Won & Yu Jin woo

#include <stdio.h>
#include <stdlib.h>

#include "data_structures.h"
#include "file_read.h"
#include "simulator.h"

//long disp functions
void disp_title(void);
void disp_end(void);
void disp_error(void);
void disp_blank_line(void);

const char *get_filename(const char* filepath);//get filename from full filepath
int read_all_data(int argc, char** argv, struct CONFIG* out_config,
	              struct THREAD** out_threads, int* out_inst_num, char* out_report_name);//package for reading function with statement
int free_all_data(struct THREAD* out_threads, int out_inst_num);//free data

int main(int argc, char* argv[])
{

	disp_title();//display awesome title

	//declear data containers
	struct CONFIG config; //config
	struct THREAD*threads = NULL;//inst_arr's array
	int           thread_num = 0;//inst_arr's array length
	char		  report_name[256]; report_name[0] = 0;//report filename 
	
	//read data
	if (read_all_data(argc, argv, &config, &threads, &thread_num, report_name) != 0) {
		disp_error();
		return 1; //if there is error, quit
	}
	
	disp_blank_line();

	//run simulation

	printf("Start simulation\n");
	struct REPORT report;
	if (core_simulator(&config, threads, thread_num, &report) != 0) { //simulate
		disp_error();
		return 1; //if there is error, quit
	}

	//print out report
	FILE* f_report = fopen(report_name, "w");
	if (f_report == NULL)
	{
		printf("Create report file error\n");
		disp_error();
		return 1;
	}

	REPORT_fprinter(&report, f_report);
	fclose(f_report);
	
	printf("Report saved : %s\n", report_name);
	disp_blank_line();
	
	free_all_data(threads, thread_num);
	disp_end();
	return 0;//program quit
}

void disp_title(void)
{
	printf("+------------------------------------------------------------------------------+\n");
	printf("                     Computer Architecture Homework #2\n");
	printf("                 Noh Dong Ju & Park Kyung Won & Yu Jin woo\n");
	printf("+------------------------------------------------------------------------------+\n");
	printf(" Usage : 1) Just excute this program, \n");
	printf("            then program take 'config.conf' and 'instruction.inst'\n");
	printf("            and simulate OoO. when end simulation, program will print out\n");
	printf("            '[dump]_[width]_[ROS_size]_[RS_size]_output.out'\n");
	printf("\n");
	printf("         2) drag and drop on exe.file or give filepath as argument,\n");
	printf("            then program take every '~.conf' and '~.inst' as input,\n");
	printf("            and simulate all combinaion of these. when end simulation,\n");
	printf("            program will print out\n");
	printf("            '[dump]_[width]_[ROS_size]_[RS_size]_[inst_name]_output.out'\n");
	printf("+------------------------------------------------------------------------------+\n");
}

void disp_end(void)
{
	printf("Program done - Press any key to quit");
	getchar();
}

void disp_error(void)
{
	printf("Program error - Press any key to quit");
	getchar();
}

void disp_blank_line(void)
{
	printf("\n");
}

const char *get_filename(const char* filepath)//get filename from full filepath
{
	int idx;
	for (idx = strlen(filepath); idx > 0; --idx) {
		if (filepath[idx - 1] == '\\' || filepath[idx - 1] == '/') {
			return filepath + idx;
		}
	}
	return filepath;
}

int read_all_data(int argc, char** argv, struct CONFIG* out_config,
	struct THREAD** out_threads, int* out_inst_num, char* out_report_name)
	//package for reading function with statement
{
	if (argc < 3) {//no inst	
		printf("Too few arguments!\n");
		return 1;
	}
	else if(argc > 4) { //more than two inst
		printf("Too many arguments!\n");
		return 1;
	} 
	else {
		//mem alloc
		*out_inst_num = (argc - 2);

		//data read
		printf("Read config %s\n", argv[1]);
		if (!config_reader(argv[1], out_config)) {
			printf("Config read error!\n");
			return 1;
		}

		//inst reading
		char* inst_filename = (char*)calloc(256 * (*out_inst_num), sizeof(char));
		for (int idx = (*out_inst_num); idx > 0; --idx) {
			strcat(inst_filename, ",");
			strcat(inst_filename, argv[idx+1]);
		}
		if (1==make_thread(inst_filename + 1, *out_inst_num, out_threads)) {
			printf("Data read error\n");
			return 1;
		}
		free(inst_filename);
		
		//make report name
		strcat(out_report_name, get_filename(argv[1]));
		for (int idx = 0; idx < (*out_inst_num); ++idx) {
			strcat(out_report_name, "_");
			strcat(out_report_name, get_filename(argv[idx+2]));
		}
		strcat(out_report_name, "_out.txt");

	}
	return 0;
}

int free_all_data(struct THREAD* out_threads, int out_inst_num)//free data
{
	for (int idx = 0; idx < out_inst_num; ++idx) {
		THREAD_delete(&out_threads[idx]);
	}
	free(out_threads);
	
	return 0;
}

