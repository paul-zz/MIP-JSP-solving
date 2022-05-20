/* Disjunctive JSP model solving based on Mixed Integer Programming
* Compared and tested on JSPLIB dataset
* Author: Zijian Zhong
* Affiliation: Shenzhen International Graduate School, Tsinghua University
*/

/* This program is implemented on Gurobi Optimization*/
/* Copyright 2018, Gurobi Optimization, LLC */

#include "dataLoader.h"
#include "jspmodel.h"

using namespace std;
int main()
{
	// Specify the JSPLIB dataset path
	DataLoader dloader("F:\\JSPLIB-master\\");
	// Define several test cases
	vector<string> test_cases = { "ft06", "la01", "ft10", "la06"};
	// Load instance
	for (string test_case : test_cases)
	{
		JSPModel* jsp = dloader.loadInstance(test_case);
		// Solve the problem and save solution
		jsp->solveGurobi(true, test_case);
	}
	return 0;
}

