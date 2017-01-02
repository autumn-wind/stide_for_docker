#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>

using namespace std;

int main( )
{
	while(true)
	{
		char time[50], container[20], start[20], cid[100], image[20], name[20];
		cin >> time >> container >> start >> cid >> image >> name;
		sleep(10);
		cout << cid << endl;
		string path_part_1("/sys/fs/cgroup/devices/docker/");
		string path_part_2(cid);
		string path_part_3("/tasks");
		string whole_path = path_part_1 + path_part_2 + path_part_3;
		ifstream tasks_file;
		tasks_file.open(whole_path);
		if(!tasks_file.is_open()) {
			cout << "open file failed!" << endl;
			continue;
		}
		vector<int> pids;
		int pid;
		while(tasks_file >> pid) {
			pids.push_back(pid);
		}
		cout << "process number: " << pids.size() << endl;
		for(int i = 0; i < pids.size(); ++i) {
			cout << pids[i] << endl;
		}
	}
	return 0;
}
