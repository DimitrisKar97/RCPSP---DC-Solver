#ifndef RCPSP_H
#define RCPSP_H
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <string.h>
#include <fstream>
#include <sstream>
#include <numeric>
#include <cmath>
#include <iostream>
using namespace std;

class rcpsp
{
private:
    int activity;
    int duration;
    vector<int> demand;
    int numofsucc;
    vector<int> successors;
    vector<int> predecessors;
    double cf;
    int start;
    bool activityCheck;
    double NPV;
public:
    rcpsp()
    {
        activity = 0;
        duration = 0;
        demand.clear();
        numofsucc = 0;
        successors.clear();
        predecessors.clear();
        cf = 0.0;
        start = 0;
        activityCheck = false;
        NPV = 0.0;
    }

    rcpsp(int in_activity, int in_duration, vector<int> in_demand, int in_numofsucc, vector<int> in_successors, vector<int> in_predecessors,
        double in_cf, int in_start, bool in_activityCheck, double in_NPV)
    {
        activity = in_activity;
        duration = in_duration;
        demand = in_demand;
        numofsucc = in_numofsucc;
        successors = in_successors;
        predecessors = in_predecessors;
        cf = in_cf;
        start = in_start;
        activityCheck = in_activityCheck;
        NPV = in_NPV;
    }

    ~rcpsp() {

    }

    void display()
    {
        vector<int>::iterator it;
        cout << setw(3) << activity << setw(10) << setfill(' ') << duration;
        for (auto it = demand.begin(); it < demand.end(); it++)
            cout << setw(10) << *it;
        cout << setw(10) << numofsucc;
        for (auto it = successors.begin(); it < successors.end(); it++)
            cout << setw(10) << *it;
        cout << setw(20) << setfill(' ') << "| " << right;
        for (auto it = predecessors.begin(); it < predecessors.end(); it++)
            cout << setw(10) << *it;
        cout << setw(10) << cf;
        cout << setw(10) << activityCheck << endl;

    }
    int getActivity() { return activity; }
    int getDuration() { return duration; }
    int getDemand(int position) { return demand[position]; }
    int getNumOfSucc() { return numofsucc; }
    int getSuccessor(int position) { return successors[position]; }
    double getCashFlow() { return cf; }
    void setCF(double value) { cf = value; }
    double getCF() { return cf; }
    int getPredecessor(int position) { return predecessors[position]; }
    size_t getNumOfPred() { return predecessors.size(); }
    int getStart() { return start; }
    void setStart(int value) { start = value; }
    bool getStatus() { return activityCheck; }
    void setStatus(bool stat) { activityCheck = stat; }
    double getNPV() { return NPV; }
    void setNPV(double cash, double rate, int period) { double t = pow(1.0 + rate, period); NPV = cash / t; }
    int getFinish() { return start + duration; }
    void PredecessorVector(int Act) { predecessors.push_back(Act); }
}*element;

int inputs;
int* resourceCap;
int resourceNum;
int MaxTime = 0;
double TotalNPV = 0.0;
vector<double> rcost;
double ConstantV = 100.0;
double ResourceCost = 15.0;
#endif