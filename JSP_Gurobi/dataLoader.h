#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

#include "nlohmann/json.hpp"
#include "jspmodel.h"

using namespace std;
using namespace nlohmann;

class DataLoader
{
private:
	string instance_folder_name;
	vector<string> all_instance_names;
	unordered_map<string, json> hash_map; // build a hashmap to index a instance quickly
	void loadInstanceList();
public: 
	DataLoader(string folder_name) :instance_folder_name(folder_name) { loadInstanceList(); };
	vector<string> getAllInstanceNames();
	JSPModel* loadInstance(string set_name);
};

void DataLoader::loadInstanceList()
{
	string json_filename = instance_folder_name + "instances.json";
	ifstream ifs(json_filename);
	json j = json::parse(ifs);
	cout << "Loaded number of instances: " << j.size() << "." << endl;
	for (json j_inst : j)
	{
		hash_map[j_inst["name"]] = j_inst;
		all_instance_names.push_back(j_inst["name"]);
	}
 }

vector<string> DataLoader::getAllInstanceNames()
{
	return all_instance_names;
}

JSPModel* DataLoader::loadInstance(string instance_name)
{
	cout << "Loading instance " << instance_name  <<  " ..." << endl;
	if (hash_map.find(instance_name) == hash_map.end())
	{
		cout << "Instance " << instance_name << " not found." << endl;
		return nullptr;
	}
	else
	{
		cout << "Instance " << instance_name << " loaded successfully!" << endl;
		cout << endl;

		json j_instance = hash_map[instance_name];
		cout << "Problem information:" << endl;
		cout << "Name: " << j_instance["name"] << endl;
		cout << "Jobs: " << j_instance["jobs"] << endl;
		cout << "Machines: " << j_instance["machines"] << endl;
		cout << "Optimum: " << j_instance["optimum"] << endl;
		cout << endl;

		// Read the instance file
		string instance_path = instance_folder_name + j_instance["path"].get<string>();
		ifstream instance_ifs(instance_path);
		if (!instance_ifs.is_open())
		{
			cout << "Cannot open file." << endl;
			return nullptr;
		}

		// Skip the comments and read m and n
		string line;
		int num_machines, num_jobs;
		while (getline(instance_ifs, line))
		{
			if (line[0] == '#') continue;
			istringstream line_ifs(line);
			line_ifs >> num_jobs >> num_machines;
			break;
		};

		// read matrix
		int machine_idx, proc_time;
		vector<vector<int>> sequence_list;
		vector<vector<int>> production_time(num_machines, vector<int>(num_jobs, 0));
		for (int i = 0; i < num_jobs; i++)
		{
			vector<int> sequence_job;
			for (int j = 0; j < num_machines; j++)
			{
				instance_ifs >> machine_idx >> proc_time;
				sequence_job.push_back(machine_idx);
				production_time[machine_idx][i] = proc_time;
			}
			sequence_list.push_back(sequence_job);
		}
		JSPModel* jsp = new JSPModel(num_machines, num_jobs, sequence_list, production_time);
		return jsp;
	}
}