// Assigning the current process to a unique logic core
int SelectUniqueCore(DWORD selected_core){
    DWORD_PTR affinityMask = 1 << selected_core; 
    DWORD_PTR result = SetProcessAffinityMask(GetCurrentProcess(), affinityMask);
    if (result == 0) {
        printf("Error when selecting the core.\n");
        return -1;
    }
    DWORD core = GetCurrentProcessorNumber();
    //printf("Logic core %lu selected succesfully.\n", core);
	return 0;
}

// Attempts an exclusive access to the unique logic core
int ExclusiveAccessToCore(DWORD selected_core){
	DWORD_PTR selected_mask = 1 << selected_core;
	// Capturing the processes 
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("Error during the capture of the processes.\n");
        return -1;
    }

	// Reading the first process
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot, &pe)) {
        printf("Error when readin the first process.\n");
        CloseHandle(snapshot);
        return -1;
    }

	// Modification of the affinity of each process that run
	// on the selected core except the current process
    do {
        // Ignoring the current process
        if (pe.th32ProcessID == GetCurrentProcessId()){
            continue;
		}

        HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
        if (hProcess != NULL) {
            DWORD_PTR current_affinity, system_affinity;
            if (GetProcessAffinityMask(hProcess, &current_affinity, &system_affinity)) {
                DWORD_PTR new_affinity = current_affinity & (~selected_mask);
                if (new_affinity != 0) {
                    if(SetProcessAffinityMask(hProcess, new_affinity)){
						//printf("Affinity modified for the PID %lu (%ws)\n", pe.th32ProcessID, pe.szExeFile);
					}
                }
            }
            CloseHandle(hProcess);
        }
    } while (Process32Next(snapshot, &pe));

    CloseHandle(snapshot);
    //printf("Exclusive access to the logic core %lu.\n", selected_core);
	return 0;
}

