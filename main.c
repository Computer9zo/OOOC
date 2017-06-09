// Computer Architecture Homework #2
// Noh Dong Ju & Park Kyung Won & Yu Jin woo

#include <stdio.h>
#include <stdlib.h>

#include "data_structures.h"
#include "file_read.h"
#include "simulator.h"

//#include "read.h" //read config and instruction
//#include "simul.h"//func related simulation

//typedef struct Config { int justtest;  } Config;

void disp_title(void);
void disp_taskdata(struct CONFIG* configs, int inst_idx, int conf_idx, char** inst_filename);
void disp_end(void);
void disp_error(void);
int get_filepath_from_arg(int argc, char** argv, char*** conf_path, int* conf_len, char*** inst_path, int* inst_len, char*** inst_filename);

int main(int argc, char* argv[])
{
	//
	// Print intro message
	//
	disp_title();

	// 
	// Read config and Data file and make config array
	//
	printf("\n");
	printf("Read Config and Instruction files : \n");
	

	char** conf_name=0;  int conf_len=0;//name = 파일경로, len= 총 개수,
	char** inst_name=0;  int inst_len=0; char** inst_filename=0;

	if (get_filepath_from_arg(argc, argv, &conf_name, &conf_len, &inst_name, &inst_len, &inst_filename) != 0)
	{//만약 메모리 부족이나 기타 이유로 정상 실행이 안되면,
		disp_error();
		return 1;
	}

	printf("Total %3d Config and %3d Instruction founded\n", conf_len, inst_len);
	//읽을 파일의 개수와 경로 파악 끝

	if (conf_len == 0 || inst_len == 0)
	{//만약 데이터 셋이 제대로 주어지지 않았다면 이후를 실행할 수 없으므로 정상 종료한다.
		printf("Too few files. Need .conf and .inst both.\n");
		disp_end();
		return 0;
	}


	//
	// simulate and for every config combination
	//


	//config 파일을 읽어 저장
	struct CONFIG* configs;
	configs = malloc(sizeof(struct CONFIG)*(conf_len));

	int for_ignore = 0;
	for (int idx = 0; idx < conf_len; ++idx)
	{
		if (!config_reader(conf_name[idx], configs + idx - for_ignore))
		{
			printf("Cannot find file %s\n", conf_name[idx]);
			++for_ignore;
		}
	}
	conf_len -= for_ignore;

	//inst 파일은 필요할 때 한개씩 읽어 저장(용량이 크므로)
	struct INST* inst_arr; int inst_arr_len;

	for (int inst_idx = 0; inst_idx < inst_len; ++inst_idx)
	{//모든 인스트럭션에 대해 
		if (!make_inst_array(inst_name[inst_idx], &inst_arr, &inst_arr_len)) 
		{
			printf("Error dectection - ignore this file %s\n", inst_filename[inst_idx]);
			continue;
			//return 1;
		}
		//printf("inst_%d : %s\n", inst_idx + 1, inst_filename[inst_idx]);

		for (int conf_idx = 0; conf_idx < conf_len; ++conf_idx)
		{//모든 config에 대해

			//정보 출력
			disp_taskdata(configs, inst_idx, conf_idx, inst_filename);

			//시뮬레이션
			struct REPORT* report;
			report = core_simulator(configs + conf_idx, inst_arr, inst_arr_len);//simulate

			//보고서 출력
			char out_filename[256];
			sprintf(out_filename, "%d_%d_%d_%d_%s_report.out",
					configs[conf_idx].Dump, configs[conf_idx].Width, configs[conf_idx].ROB_size, configs[conf_idx].RS_size, inst_filename[inst_idx]);
			FILE* f_report = fopen(out_filename, "w");
			
			if (f_report == NULL)
			{
				printf("Error : Lack of memory\n");
				disp_error();
				return 1;
			}
			REPORT_fprinter(report, f_report);

			fclose(f_report);			
			free(report);

			printf("out : %s", out_filename);
			printf("\n\n");
		}

		free(inst_arr);

	}

	free(configs);// free config
	free(conf_name);
	free(inst_name);
	free(inst_filename);

	disp_end();

	return 0;
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

int get_filepath_from_arg(int argc, char** argv,char*** conf_path, int* conf_len, char*** inst_path, int* inst_len, char*** inst_filename)
{

	(*conf_path) = malloc(sizeof(char*)*argc);
	(*inst_path) = malloc(sizeof(char*)*argc);//공간 낭비지만 그렇게 크지는 않음
	(*inst_filename) = malloc(sizeof(char*)*argc);
	if ((*conf_path) == NULL ||
		(*inst_path) == NULL ||
		(*inst_filename) == NULL)
	{
		printf("Lack of memory!\n");
		return 1;
	}
	(*conf_len) = 0;
	(*inst_len) = 0;

	if (argc == 1)
	{//if there is no input, set basic value;

		(*conf_len) = 1;
		(*conf_path)[0] = "config.conf";

		(*inst_len) = 1;
		(*inst_path)[0] = "instruction.inst";
		(*inst_filename)[0] = "instruction.inst";

	}
	else
	{//if there is input from argv,  모든 입력 인수를 읽어 .conf는 conf에 .inst는 inst에, 나머지는 무시.

		char* file_name = 0;//확장자나 파일 명을 기록할 임시 포인터

		for (int idx = 1; idx < argc; ++idx)//every config_file 
		{
			//printf("%s\n", argv[idx]);

			file_name = argv[idx] + (strlen(argv[idx]) - 5); // 맨 끝 5자리를 가져온다. 그 후 .conf인지 .inst인지 둘다 아닌지를 확인한다. 
			
			if (0 == strcmp(file_name, ".conf"))//if conf file
			{
				(*conf_path)[*conf_len] = argv[idx];
				++(*conf_len);
			}
			else if (0 == strcmp(file_name, ".inst"))
			{
				
				(*inst_path)[*inst_len] = argv[idx];
				(*inst_filename)[*inst_len] = get_filename(argv[idx]);
				//printf("%s", inst_filename[inst_len]);
				++(*inst_len);
			}
			else
			{
				printf("File %s is nethier .conf and .inst! \n", argv[idx]);
			}
		}

	}
	return 0;
}