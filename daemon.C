#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

#include "common.h"

#define MAX_MONITORED_CONTAINER_NR 50

using namespace std;

int main( )
{
	//cids contain all the container's id which is being monitored
	vector<string> cids;

	//monitoring_process_pids contain all the monitoring process's pid
	vector<pid_t> monitoring_process_pids;

	//number of containers being monitored
	int container_index = 0;

	while (container_index < MAX_MONITORED_CONTAINER_NR)
	{
		string time, container, start, cid, image, name;

		/* read from docker events output */
		cin >> time >> container >> start >> cid >> image >> name;

		pid_t pid;

		if ((pid = fork()) < 0 )
			err_EXIT("fork");

		//fork a new monitoring process to monitor the container cid
		else if (pid == 0) {
			cout << "Start monitoring container: cid " << cid << endl;

			//the odd things to do when mix C++&C code...
			string path = "/home/gen/stide_for_docker/monitor";
			char *const arg0 = new char[path.length() + 1];
			char *const arg1 = new char[cid.length() + 1];
			strcpy(arg0, path.c_str());
			strcpy(arg1, cid.c_str());
			char *const argv[] = {arg0, arg1, NULL};

			//execve into the new process
			execv(argv[0], argv);
			err_EXIT("execv failure");
		}

		cids.push_back(cid);
		monitoring_process_pids.push_back(pid);
		container_index += 1;
	}

	return 0;
}
