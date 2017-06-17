#include "file_read.h"

bool char_to_INST(char* buffer, struct INST * out_inst);

bool read_instruction(FILE *in_filename, struct INST * out_inst)
{// bool is_readed, read one line and convert INST form
	
	char buffer[30];//buffer for read one line (inst)
	if (fgets(buffer, 30, in_filename) == NULL) { printf("File read failed \n"); return false; }//if we can't read (get eof)

	return char_to_INST(buffer, out_inst);
}

bool make_inst_array(char* filename, struct INST ** out_inst_arr, int *len)
{		
	printf("Read %s ", filename);
	
	//printf("Instruction File Reader\n");
	//printf("Open %s ",filename);
	char filename_tmp[100];
	strcpy(filename_tmp,filename);
	char* thread_name_buffer=strtok(filename_tmp,",");
	int thread_num=0;
	while(thread_name_buffer!=NULL)
	{
		thread_name_buffer=strtok(NULL,",");
		thread_num++;
	}
	
	char thread_name[thread_num][15];
	strcpy(thread_name[0],strtok(filename,","));
	for(int i=1; i<thread_num; i++)
	{
		strcpy(thread_name[i],strtok(NULL,","));
	}
	
	struct INST *multi_thread_out[thread_num];
	
	for(int i=0; i<thread_num; i++)
	{
	
		FILE* p_file = fopen(filename, "rb");
		if (p_file== NULL) { printf("- Failed \n"); return false; }//if fail to open file, return false
		//printf("- Done \n");
	
		//get file size
		//printf("File size = ");
		fseek(p_file, 0, SEEK_END);
		int p_len = ftell(p_file);
			//fseek(p_file, -10, SEEK_END);
			//char tempss[10];
			//fgets(tempss, 10, p_file);
			//printf("%s", tempss);
		fseek(p_file, 0, SEEK_SET);
		//printf("%dKB\n", sizeof(char)*p_len/1024);
	
		//all file load on memory
		//printf("Loading file on memory ");
		char* p_file_buffer = (char*)calloc(p_len + 1,sizeof(char));
		if (p_file_buffer == NULL) { printf("\nLack of memory\n"); return false; }
	
		int read_ith;
		for  (read_ith = 0; read_ith < (p_len/4096); ++read_ith)
		{
			fread(p_file_buffer+(read_ith*4096), sizeof(char), 4096, p_file);
		}
		int temp=fread(p_file_buffer + (read_ith * 4096), sizeof(char), p_len % 4096, p_file);
		//printf("%s-%d", p_file_buffer + (read_ith * 4096), temp);
		//system("PAUSE");
	
		//Make file always end \n
		if (p_file_buffer[p_len-1] != '\n'){ p_file_buffer[p_len] = '\n'; ++p_len; }
		//printf("- Done \n");
	
		//close file
		fclose(p_file);
		
		//debug - check file end
		//for (int i = 5000; i > 0; --i)
		//{
		//	printf(".%c",*(p_file_buffer-i+p_len));
		//}
	
		//Make blank instruction array
		//printf("Allocate Memory for Instruction Array");
		int length = 0;
		for (int p_idx = 0; p_idx < p_len; ++p_idx)
		{
			if (p_file_buffer[p_idx]=='\n') { ++length; }
		}//get line number 
	
		printf("%d\n",length);
		multi_thread_out[i]=(struct INST*)malloc(sizeof(struct INST)*length);
		if (multi_thread_out[i] == NULL) { printf("\nLack of memory\n"); return false; }
		//printf("- Done \n");
		printf("%d\n",*multi_thread_out[i]);
		//Translate File and Fill Instruction
		//printf("Make Instruction Array -   0%%");
		printf("-   0%%");
	

	
		char* p_line = p_file_buffer;//file_pointer_in_memory
		char* p_token_line;
		for (int token_length = 0; token_length < length; ++token_length)
		{
			//remember start point of line
			p_token_line = p_line;
	
			//find next line start point
			p_line = strchr(p_line, '\n');
			//(*p_line) = '\0';
			++p_line;
	
			//translate this line 
			//printf("%s", p_token_line);
			is_worked = ( is_worked && char_to_INST(p_token_line,(multi_thread_out[i])+token_length) );
			
			if (token_length % (length / 100) == 0)
			{
				printf("\b\b\b\b%3d%%", token_length * 100 / length);//for make people not boring
			}
			//system("PAUSE");
		}
		(*len) = length;
		printf("\b\b\b\b100%%\n");
	
		free(p_file_buffer);
	}
	*out_inst_arr=*multi_thread_out;
	bool is_worked = true;
	return is_worked;
}

bool char_to_INST(char* buffer, struct INST * out_inst)
{
	char* inst_name = strtok(buffer, " ");

	switch (inst_name[3])
	{
	case 'A':
		out_inst->opcode = IntAlu;
		break;
	case 'R':
		out_inst->opcode = MemRead;
		break;
	case 'W':
		out_inst->opcode = MemWrite;
		break;
	default:
		printf("Instruction read failed \n");
		return false;
	}
	out_inst->dest = atoi(strtok(NULL, " "));
	out_inst->oprd_1 = atoi(strtok(NULL, " "));
	out_inst->oprd_2 = atoi(strtok(NULL, " "));
	//if (out_inst->op != IntAlu) { out_inst->oprd2 = atoi(strtok(NULL, " ")); }

	return true;
}


bool config_reader(char* filename, struct CONFIG *out_config)
{
	char buffer[30];
	FILE *configp;
	if ((configp = fopen(filename, "r")) == NULL) { return false; }//if configp == null, it mean fail.
	for (int idx = 0; idx < 4; ++idx)
	{
		fgets(buffer, 30, configp);
		switch (idx)
		{
		case 0: out_config->Dump = atoi(buffer); break;
		case 1: out_config->Width = atoi(buffer); break;
		case 2: out_config->ROB_size = atoi(buffer); break;
		case 3: out_config->RS_size = atoi(buffer);
		}
	}
	return true;

	//if "fgets"function cannot get any value, then while phrase would not activated. In that case this function return 0 meaning false. 
	//if once while phrase activated, it finally goes to switch case 4 and return 1.
}




//
//void main()
//{
///*	FILE* in_file=fopen("hw2_trace_bzip2.out","r");
//	if(in_file==NULL)
//	{
//		fputs("cannot open input file...\n",stderr);
//		exit(1);
//	}
//	*/
//	struct INST instruction;
//	FILE* fid;
//	if ((fid = fopen("hw2_trace_bzip2.out", "r")) != NULL)
//	{
//		read_instruction(fid, &instruction);
//		INST_printer(&instruction);
//		fclose(fid);
//	}
//	//system("PAUSE");
//
//	struct INST *ptr_instruction;
//	int length;
//	if (!make_inst_array("hw2_trace_bzip2.out",&ptr_instruction, &length))
//	{
//		printf("Read Failed\n");
//	}
//	else
//	{
//		printf("1st-");
//		INST_printer(ptr_instruction);
//		printf("end-");
//		INST_printer(ptr_instruction + length - 1);
//	}
//	//system("PAUSE");
//
//	struct Config config;
//	if (!config_reader("config.txt", &config)) { printf("Read Failed\n"); }
//	Config_printer(&config);
//
//	//system("PAUSE");
//	getchar();
////	fclose(in_file);
//}
//
