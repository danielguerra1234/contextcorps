void load_PCB (char *name, int priorityc, int classcm, char* directory_name){
	pcb* pcb1;
	int size;
	int class = classc;
	int* prog_len_p;
	int* start_offset_p;
	int* load_address;
	int exec_address;
	//	printf("SetupPCB: Name: %s, Prior: %d, Class: %d\n\n", name, priorityc, classc);
	
	if (name == NULL) {
		errorCodeTranslator(ERR_PCB_NONAME);
		return;
	}
	if (strlen(name) > 15) {
		errorCodeTranslator(ERR_PCB_NMETOLONG);
		return;
	}
/*	if (Find_PCB(name) != NULL) {
		errorCodeTranslator(ERR_PCB_NMEEXISTS);
		return;
	}  */
	if(priorityc >127 || priorityc<-128) {
		errorCodeTranslator(ERR_PCB_INVPRIORITY);
		return;
	}
	if (class!= 1 && class!= 2) {
		errorCodeTranslator(ERR_PCB_INVCLASS);
		return;
	}
	pcb1 = allocatePcb();
	printf("PCB name: %s\n", name);
	pcb1->process_name = strdup(name);
	//sprintf("%s, %s", name, pcb1->process_name);
	pcb1->priority = priorityc;
	pcb1->process_class = classc;
	pcb1->state = 102; //whatever reflects suspended_ready
	pcb1->exe_addr = &load_address + & start_offset_p;;
	printf("PCB succesfully created\n\n");
	//printf("\nName: %s, Class: %d, State: %d\n",pcb1->process_name, pcb1->process_class,pcb1->state);
	//Show_PCB(name);
	//printf("\nReadyQ check in setup: %s\n", (readyQ->head)->process_name);
	size= sys_check_program(directory_name, directory_name.mpx , prog_len_p, start_offset_p);
	load_address= sys_alloc_mem(size);
	sys_load_program (load_address, 4096, directory_name, directory_name.mpx);
	//we should initialize pcb fields
	//initialize load
	//initialize exec
	//initialize memory size
	save_context(); //this is supposed to setup the pcb context
	Insert_PCB(pcb1); // should go to the suspended ready queue
	return pcb1;
}


void terminate_process(char* pcb_name){
	pcb* pcb1;
	pcb* pcb2;
	pcb1= find_pcb(pcb_name);
	if(pcb1== NULL)
		//add error message
		printf("Process does not exist");
	pcb2= remove_pcb(pcb1);
	sys_free_mem(pcb2);
}

