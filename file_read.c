#include "file_read.h"

bool char_to_INST(char* buffer, struct INST * out_inst);

bool read_instruction(FILE *in_filename, struct INST * out_inst)
{// bool is_readed, read one line and convert INST form
	
	char buffer[30];//buffer for read one line (inst)
	if (fgets(buffer, 30, in_filename) == NULL) { printf("File read failed \n"); return false; }//if we can't read (get eof)

	return char_to_INST(buffer, out_inst);
}

bool make_inst_array(char* filename, struct INST *** out_inst_arr, int **out_len)
{		
	printf("Read %s ", filename);
	
	// thread number check
	char* filename_tmp[4096];
	strcpy(filename_tmp,filename);
	char* thread_name_buffer=strtok(filename_tmp,",");
	int thread_num=0;
	while(thread_name_buffer!=NULL)
	{
		thread_name_buffer=strtok(NULL,",");
		thread_num++;
	}
	strcpy(filename_tmp, filename);//filename restore

	//thread name storage
	char** thread_name = (char**)calloc(thread_num, sizeof(char*));
	int* thread_len = (int*)calloc(thread_num, sizeof(int));
	if (thread_name == NULL || thread_len == NULL) { printf("- failed Mem alloc for name \n"); return false; }//if fail to alloc, return false

	thread_name[0] = strtok(filename_tmp,",");
	for(int i=1; i<thread_num; i++) {
		thread_name[i] = strtok(NULL,",");
	}
	
	printf("\n");

	//make storage
	struct INST **multi_thread_out = (struct INST **)calloc(thread_num,sizeof(struct INST*));

	for(int i=0; i<thread_num; i++)
	{

		//file open
		FILE* p_file = fopen(thread_name[i], "rb");
		if (p_file== NULL) { printf("- Failed \n"); return false; }//if fail to open file, return false
		
		//file size check
		fseek(p_file, 0, SEEK_END);
		int p_len = ftell(p_file);
		fseek(p_file, 0, SEEK_SET);
	
		//make storage for file to load on mem
		char* p_file_buffer = (char*)calloc(p_len + 1,sizeof(char));
		if (p_file_buffer == NULL) { printf("\nLack of memory\n"); return false; }
	
		//uploading all data in file to mem
		int read_ith;
		for  (read_ith = 0; read_ith < (p_len/4096); ++read_ith)
		{
			fread(p_file_buffer+(read_ith*4096), sizeof(char), 4096, p_file);
		}
		int temp=fread(p_file_buffer + (read_ith * 4096), sizeof(char), p_len % 4096, p_file);
	
		//make file always end '\n'
		if (p_file_buffer[p_len-1] != '\n'){ p_file_buffer[p_len] = '\n'; ++p_len; }
		
		//close file
		fclose(p_file);
		
		//get line number from loaded file
		int length = 0;
		for (int p_idx = 0; p_idx < p_len; ++p_idx) {
			if (p_file_buffer[p_idx]=='\n') { ++length; }
		}
		
		//make storage
		multi_thread_out[i]=(struct INST*)malloc(sizeof(struct INST)*length);
		if (multi_thread_out[i] == NULL) { printf("\nLack of memory\n"); return false; }
		

		printf("Instruction file %d read ",i);
		printf("-   0%%");
	
		//pasing data and make inst array

		char* p_line = p_file_buffer;//file_pointer_in_memory
		char* p_token_line;
		for (int token_length = 0; token_length < length; ++token_length)
		{
			//remember start point of line
			p_token_line = p_line;
			
			//find next line start point
			p_line = strchr(p_line, '\n');
			*p_line = 0;//strtok = div string
			++p_line;

			//translate this line 
			if (!char_to_INST(p_token_line, (multi_thread_out[i]) + token_length)) {
				printf("\nNot good file!\n");
				return false;
			}
			
			if ((length<100)||(token_length % length/100 == 0))
			{
				printf("\b\b\b\b%3d%%", token_length * 100 / length);//for make people not boring
			}
			//system("PAUSE");
		}
		printf("\b\b\b\b100%%\n");
	
		thread_len[i] = length;
		free(p_file_buffer);
	}

	*out_inst_arr=multi_thread_out;
	*out_len = thread_len;

	return true;
}

bool char_to_INST(char* buffer, struct INST * out_inst)
{
	char* inst_name = strtok(buffer, " ");
	if (inst_name == NULL) { return false; }
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
	char buffer[30]; buffer[0] = 0;
	FILE *configp;
	if ((configp = fopen(filename, "r")) == NULL) { return false; }//if configp == null, it mean fail.
	for (int idx = 0; idx < 6; ++idx)
	{
		fgets(buffer, 30, configp);
		if (buffer[0] == 0) { return false; }//there is lack of arg
		switch (idx)
		{
		case 0: out_config->Dump = atoi(buffer); break;
		case 1: out_config->Width = atoi(buffer); break;
		case 2: out_config->ROB_size = atoi(buffer); break;
		case 3: out_config->RS_size = atoi(buffer); break;
		case 4: out_config->LSQ_size = atoi(buffer); break;
		case 5: out_config->Cache_size = atoi(buffer);
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
