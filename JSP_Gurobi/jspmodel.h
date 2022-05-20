#pragma once

#include <iostream>
#include <vector>
#include "gurobi_c++.h"

using namespace std;

class JSPModel
{
private:
	int m; // number of machines
	int n; // number of jobs
	vector<vector<int>> job_seqs; // Sequence of each job on each machine
	vector<vector<int>> production_time; // Production time of job j on machine i
public:
	JSPModel(int num_machines, int num_jobs, vector<vector<int>> job_seqs_vec, vector<vector<int>> prod_time_mat)
		:m(num_machines), n(num_jobs), job_seqs(job_seqs_vec), production_time(prod_time_mat) {};
	void solveGurobi(bool save_to_file, string save_file_name);
};



void JSPModel::solveGurobi(bool save_to_file=false, string save_file_name="")
{
	cout << "Starting Gurobi ..." << endl;
	cout << endl;
	try
	{
		GRBEnv env = GRBEnv();
		GRBModel model = GRBModel(env);

		// Add decision variable X
		// X[i][j] means the integer start time of job j on machine i
		vector<vector<GRBVar>> X;
		for (int i = 0; i < m; i++)
		{
			vector<GRBVar> row;
			for (int j = 0; j < n; j++)
			{
				string var_name = "x_{" + to_string(i) + ", " + to_string(j) + "}";
				GRBVar x = model.addVar(0, GRB_INFINITY, 0, GRB_INTEGER, var_name);
				row.push_back(x);
			}
			X.push_back(row);
		}

		// Add decision variable Z
		// Z[i][j][k] is equal to 1 if job j precedes job k on machine i
		vector<vector<vector<GRBVar>>> Z;
		for (int i = 0; i < m; i++)
		{
			vector<vector<GRBVar>> mat;
			for (int j = 0; j < n; j++)
			{
				vector<GRBVar> row;
				for (int k = 0; k < n; k++)
				{
					string var_name = "z_{" + to_string(i) + ", " + to_string(j) + ", " + to_string(k) + "}";
					GRBVar z = model.addVar(0, 1, 0, GRB_BINARY, var_name);
					row.push_back(z);
				}
				mat.push_back(row);
			}
			Z.push_back(mat);
		}

		// Temporary variable Cmax
		GRBVar Cmax = model.addVar(0, GRB_INFINITY, 0, GRB_INTEGER, "Cmax");

		// Set objective funtion
		GRBLinExpr obj = Cmax;
		model.setObjective(obj, GRB_MINIMIZE);

		// Add constraints
		// Equation (3), means that each step can be taken only after the precedent step finished
		for (int j = 0; j < n; j++)
		{
			for (int h = 1; h < m; h++)
			{
				model.addConstr(X[job_seqs[j][h]][j] >= X[job_seqs[j][h - 1]][j] + production_time[job_seqs[j][h - 1]][j]);
			}
		}
		

		// Equation (4) and (5), means that no two jobs can be scheduled on the same machine at the same time
		int V = 100000; // Set to a very big number
		for (int i = 0; i < m; i++)
		{
			for (int k = 0; k < n; k++)
			{
				for (int j = 0; j < k; j++)
				{
					model.addConstr(X[i][j] >= X[i][k] + production_time[i][k] - V * Z[i][j][k]);
					model.addConstr(X[i][k] >= X[i][j] + production_time[i][j] - V * (1 - Z[i][j][k]));
				}
			}
		}

		
		// Equation (6), ensure Cmax is the latest job finish time
		for (int j = 0; j < n; j++)
		{
			model.addConstr(Cmax >= X[job_seqs[j][m-1]][j] + production_time[job_seqs[j][m-1]][j]);
		}
		model.optimize();

		// Finished. 
		cout << "Optimization finished." << endl;
		cout << "OBJ: " << model.get(GRB_DoubleAttr_ObjVal) << endl;
		cout << endl;

		// Save to file.
		if (save_to_file)
		{
			cout << "Saving results..." << endl;
			model.write(save_file_name+".json");
		}
		
	}
	catch (GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}

}

