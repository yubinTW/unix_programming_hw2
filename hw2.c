#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#define true 1
#define false 0

typedef struct {
	unsigned int pid;
	unsigned int uid;
	unsigned int gid;
	unsigned int ppid;
	unsigned int pgid;
	unsigned int sid;
	char tty[50];
	unsigned int tty_nr;
	char status[50];
	char cmd[50];
	char comm[50];
}ProcessInfo;


int isNumberString(char* str){
	int i = 0;
	while(isdigit(str[i]))
		i++;
	return i == strlen(str);
} // end of isNumberString()

char* getDeviceName(unsigned int major) {
	char major_str[10];
	sprintf(major_str, "%d", major);
	FILE* deviceFile = fopen("/proc/devices", "r");
	char buffer[1024];
	char device_id[10];
	char* device_name = (char*)malloc(sizeof(char) * 50);
	while(fgets(buffer, 1024, deviceFile) != NULL) {
		sscanf(buffer, "%s %s", device_id, device_name);
		if(strcmp(major_str, device_id) == 0) {
			return device_name;
		}
	}
	return "-";
} // end of getDeviceName()

char* getTTY(unsigned int tty_nr) {
	if(tty_nr == 0)
		return "-";
	unsigned int major = (tty_nr & 65280) >> 8;
	unsigned int minor = tty_nr & 4294902015;
	char* dn = getDeviceName(major);
	char* result = (char*)malloc(sizeof(char) * 50);
	sprintf(result, "%s/%d", dn, minor);
	return result;
} // end of getTTY


void getProcessInfo(char* pid, ProcessInfo* pInfo) {
	// get stat
	char filename[30];
	sprintf(filename, "/proc/%s/stat", pid);
	FILE* statFile = fopen(filename, "r");
	char buffer[1024];
	fgets(buffer, 1024, statFile);
	sscanf(buffer, "%u %s %s %u %u %u %u", &(pInfo->pid), &(pInfo->comm), &(pInfo->status), &(pInfo->ppid), &(pInfo->pgid), &(pInfo->sid), &(pInfo->tty_nr));
	
	// get uid, gid
	sprintf(filename, "/proc/%s/status", pid);
	FILE* statusFile = fopen(filename, "r");
	char tmp[10];
	while(fgets(buffer, 1024, statusFile) != NULL) {
		if(strncmp(buffer, "Uid", 3) == 0){
			sscanf(buffer, "%s %u", tmp, &(pInfo->uid));
		}
		if(strncmp(buffer, "Gid", 3) == 0) {
			sscanf(buffer, "%s %u", tmp, &(pInfo->gid));
		}
	}

	// get cmdline
	sprintf(filename, "/proc/%s/cmdline", pid);
	FILE* cmdlineFile = fopen(filename, "r");
	fgets(pInfo->cmd, 20, cmdlineFile);

	// get tty
	strcpy(pInfo->tty, getTTY(pInfo->tty_nr));

} // end of getProcessInfo()

void printTitle(int hasTTY) {
	if(hasTTY)
		printf("%7s %7s %7s %7s %7s %7s %7s St %-20s cmd\n", "pid", "uid", "gid", "ppid", "pgid", "sid", "tty", "(img)");
	else
		printf("%7s %7s %7s %7s %7s %7s St %-20s cmd\n", "pid", "uid", "gid", "ppid", "pgid", "sid", "(img)");
} // end of printTile()

void printProcessInfo(ProcessInfo pInfo, int hasTTY) {
	if(hasTTY)
		printf("%7d %7d %7d %7d %7d %7d %7s %2c %-20s %7s\n", pInfo.pid, pInfo.uid, pInfo.gid, pInfo.ppid, pInfo.pgid, pInfo.sid, pInfo.tty, pInfo.status[0],pInfo.comm , pInfo.cmd);
	else
		printf("%7d %7d %7d %7d %7d %7d %2c %-20s %7s\n", pInfo.pid, pInfo.uid, pInfo.gid, pInfo.ppid, pInfo.pgid, pInfo.sid, pInfo.status[0],pInfo.comm , pInfo.cmd);
} // end of printProcessInfo()

int process_count = 0;
ProcessInfo* process_array;
char names[500][10];

void printAllProcess(int allUser, int hasTTY){
	int i = 0;
	for(i=0;i<process_count;i++){
		if(allUser || (!allUser && getuid() == process_array[i].uid))
			printProcessInfo(process_array[i], hasTTY);
	}
} // end of printAllProcess()

int compareByPID(const void* a, const void* b) {
	ProcessInfo* aInfo = (ProcessInfo*)a;
	ProcessInfo* bInfo = (ProcessInfo*)b;
	return (int)(aInfo->pid) - (int)(bInfo->pid);
} // end of compareByPID()

int compareByPPID(const void* a, const void* b) {
	ProcessInfo* aInfo = (ProcessInfo*)a;
	ProcessInfo* bInfo = (ProcessInfo*)b;
	return (int)(aInfo->ppid) - (int)(bInfo->ppid);
} // end of compareByPPID()

int compareByPGID(const void* a, const void* b) {
	ProcessInfo* aInfo = (ProcessInfo*)a;
	ProcessInfo* bInfo = (ProcessInfo*)b;
	return (int)(aInfo->pgid) - (int)(bInfo->pgid);
} //end of compareByPGID()

int compareBySID(const void* a, const void* b) {
	ProcessInfo* aInfo = (ProcessInfo*)a;
	ProcessInfo* bInfo = (ProcessInfo*)b;
	return (int)(aInfo->sid) - (int)(bInfo->sid);
} //end of compareBySID()

int allUser = false;
int hasTTY = true;

void setOption(char* opt) {
	if(strcmp(opt, "-a") == 0)
		allUser = true;
	if(strcmp(opt, "-x") == 0)
		hasTTY = false;
	if(strcmp(opt, "-p") == 0)
		qsort(process_array, process_count,sizeof(ProcessInfo), compareByPID);
	if(strcmp(opt, "-q") == 0)
		qsort(process_array, process_count,sizeof(ProcessInfo), compareByPPID);
	if(strcmp(opt, "-r") == 0)
		qsort(process_array, process_count,sizeof(ProcessInfo), compareByPGID);
	if(strcmp(opt, "-s") == 0)
		qsort(process_array, process_count,sizeof(ProcessInfo), compareBySID);
} //end of setOption()

void getAllProcessID() {
	DIR* d;
	struct dirent *dir;
	d = opendir("/proc");
	if(d){
		while((dir = readdir(d)) != NULL){
			char* filename = dir->d_name;	
			if(isNumberString(filename)){
				strcpy(names[process_count], dir->d_name);
				process_count++;
			}
		}
	}
} // end of getAllProcessID()

int main(int argc, char** argv) {
	int i;
	
	getAllProcessID();

	process_array = (ProcessInfo*)malloc(sizeof(ProcessInfo) * process_count); 
	for(i=0 ; i<process_count ; i++)
		getProcessInfo(names[i], &(process_array[i]));

	// handle optional argument
	for(i=1;i<argc;i++)
		setOption(argv[i]);
	
	printTitle(hasTTY);
	printAllProcess(allUser, hasTTY);

	return 0;
} //end of main()
