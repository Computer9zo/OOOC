// Computer Architecture Homework #3
// Noh Dong Ju & Park Kyung Won & Yu Jin woo

#include <stdio.h>
#include <stdlib.h>

#include "data_structures.h"
#include "file_read.h"
//#include "simulator.h"

//long disp functions
void disp_title(void);
void disp_end(void);
void disp_error(void);

char *get_filename(char* filepath);//get filename from full filepath
int read_all_data(int argc, char** argv, struct CONFIG* out_config,
	              struct INST*** out_inst_arrs, int** out_inst_len, int* out_inst_num, char* report_name);//package for reading function with statement
int free_all_data(struct INST** out_inst_arrs, int* out_inst_len, int out_inst_num);//free data

int main(int argc, char* argv[])
{

	disp_title();//display awesome title

	//declear data containers
	struct CONFIG config; //config
	struct INST** inst_arr;//inst_arr's array
	int*          inst_len;//inst_arr's length array
	int           inst_num;//inst_arr's array length
	char		  report_name[256];//report filename

	//read data
	if (read_all_data(argc, argv, &config, &inst_arr, &inst_len, &inst_num, report_name) != 0) {
		disp_error();
		return 1; //if there is error, quit
	}
	/*
	//run simulation
	struct REPORT report;
	report = core_simulator(&config, inst_arr, inst_len, inst_num);//simulate

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
	*/
	printf("Report saved : %s", report_name);
	printf("\n\n");
	
	free_all_data(inst_arr, inst_len, inst_num);
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

void disp_taskdata(struct CONFIG* configs, int inst_idx, int conf_idx, char** inst_filename)
{
	printf("Task %d in total %d\n", inst_idx + 1, (inst_idx + 1)*(conf_idx + 1));
	printf("inst_%d : %s\n", inst_idx + 1, inst_filename[inst_idx]);
	printf("conf_%d : ", conf_idx + 1);
	Config_printer(configs + conf_idx);
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

char *get_filename(char* filepath)//get filename from full filepath
{
	int idx;
	for (idx = strlen(filepath); idx > 0; --idx)
	{
		if (filepath[idx - 1] == '\\' || filepath[idx - 1] == '/')
		{
			return filepath + idx;
		}
	}
	return filepath;
}

int read_all_data(int argc, char** argv, struct CONFIG* out_config,
	struct INST*** out_inst_arrs, int** out_inst_len, int* out_inst_num, char* report_name)//package for reading function with statement
{
	if (argc < 3)
	{//no inst	
		printf("Too few arguments!");
		return 1;
	}
	else if(argc > 4)
	{//more than two inst
		printf("Too many arguments!");
		return 1;
	}
	else
	{
		//mem alloc
		*out_inst_num = (argc - 2);
		*out_inst_len = (int*)calloc(*out_inst_num, sizeof(int));
		*out_inst_arrs= (struct INST**)calloc(*out_inst_num, sizeof(struct INST*));

		//data read
		if (!config_reader(argv[1], out_config))
		{
			printf("Config read error!\n");
			return 1;
		}

		for (int idx = *out_inst_num; idx > 0; --idx) {
			if (!make_inst_array(argv[idx + 1], (*out_inst_arrs) + idx - 1, (*out_inst_len) + idx - 1))
			{
				printf("Data read error %d\n", idx);
				return 1;
			}
		}

		//make report name
		strcat(report_name, get_filename(argv[1]));
		for (int idx = 0; idx < (*out_inst_num); ++idx) {
			strcat(report_name, "_");
			strcat(report_name, get_filename(argv[idx+2]));
		}
		strcat(report_name, "_out.out");
	}
}

int free_all_data(struct INST** out_inst_arrs, int* out_inst_len, int out_inst_num)//free data
{
	for (int idx = 0; idx < out_inst_num; ++idx)
	{
		free(out_inst_arrs[idx]);
	}
	free(out_inst_arrs);
	free(out_inst_len);

	return 0;
}

