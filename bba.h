#ifndef BBA_H
#define BBA_H

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <numeric>
#include <cmath>
#include "rcpsp.h"

using namespace std;

class In_Pop
{
private:
    vector<int> solution;
    double NPV;
    int duration;
    vector<double> prio;

public:
    In_Pop()
    {
        solution.clear();
        NPV = 0.0;
        duration = 0;
        prio.clear();
    }

    In_Pop(vector<int> in_solution, double in_npv, int in_dur, vector<double> in_prio)
    {
        solution = in_solution;
        NPV = in_npv;
        duration = in_dur;
        prio = in_prio;
    }
    ~In_Pop() {

    }
    double GetNPV() { return NPV; }
    vector<int> GetSolution() { return solution; }
    vector<double> GetPrio() { return prio; }
    int GetSolutionDuration() { return duration; }
    int Sol(int value) { return solution[value]; }
    double Prio(int value) { return prio[value]; }
}*scout_bees;


class Foragers
{
private:
    vector<int> solution;
    int duration;
    vector<vector<int>> Gannt;
    double Total_NPV;
    vector<double> prio;   
    vector<double> NPV;
    vector<bool> status;
    vector<int> act_start;
    vector<int> act_finish;
    vector<bool> check;
public:
    Foragers()
    {
        solution.clear();
        Total_NPV = 0.0;
        NPV.clear();
        duration = 0;
        prio.clear();
        Gannt.clear();
        status.clear();
        act_start.clear();
        act_finish.clear();
        check.clear();
    }

    Foragers(vector<int> in_sol, int in_dur, vector<vector<int>> in_gannt, vector<double> in_prio, double in_total_npv, vector<double> in_npv,
        vector<bool> in_status, vector<int> in_start, vector<int> in_finish, vector<bool> in_check)
    {
        solution = in_sol;
        duration = in_dur;
        Gannt = in_gannt;
        prio = in_prio;
        Total_NPV = in_total_npv;
        NPV = in_npv;
        status = in_status;
        act_start = in_start;
        act_finish = in_finish;
        check = in_check;
    }
    ~Foragers() {

    }

    int getStart(int pos) { return act_start[pos]; }
    void setStart(int pos, int value) { act_start[pos] = value; }
    void setFinish(int act, int value) { act_finish[act] = act_start[act] + value; }
    int getFinish(int act) { return act_finish[act]; }
    int getDuration() { return duration; }
    bool getStatus(int pos) { return status[pos]; }
    void setStatus(int pos, bool stat) { status[pos] = stat; }
    void setNPV(int act, double value) { NPV[act] = value; }
    vector<double> getNPVVec() { return NPV; }
    vector<double> getPrio() { return prio; }
    vector<bool> getStatusVec() { return status; }
    vector<int> getStartVec() { return act_start; }
    vector<int> getFinishVec() { return act_finish; }
    vector<bool> getCheckVec() { return check; }
    vector<int> getSolVec() { return solution; }
    vector<vector<int>> getGannt() { return Gannt; }
    double getTotalNPV() { return Total_NPV; }

}***forager_bees;


class Best
{
private:
    vector<int> solution;
    int duration;
    vector<vector<int>> Gannt;
    double Total_NPV;
    vector<double> NPV;
    vector<int> act_start;
    vector<int> act_finish;
    vector<bool> check;
public:
    Best()
    {
        solution.clear();
        Total_NPV = 0.0;
        NPV.clear();
        duration = 0;
        Gannt.clear();
        act_start.clear();
        act_finish.clear();
        check.clear();
    }

    Best(vector<int> in_sol, int in_dur, vector<vector<int>> in_gannt, double in_total_npv, vector<double> in_npv, vector<int> in_start, vector<int> in_finish, vector<bool> in_check)
    {
        solution = in_sol;
        duration = in_dur;
        Gannt = in_gannt;
        Total_NPV = in_total_npv;
        NPV = in_npv;
        act_start = in_start;
        act_finish = in_finish;
        check = in_check;
    }

    ~Best() {

    }

    void setCheck(int act, bool val) { check[act] = val; }
    bool getCheck(int act) { return check[act]; }
    int getFinish(int act) { return act_finish[act]; }
    double getTotalNPV() { return Total_NPV; }
    vector<int> getSolVec() { return solution; }
    vector<vector<int>> getGannt() { return Gannt; }
    vector<double> getNPVVec() { return NPV; }
    vector<int> getStartVec() { return act_start; }
    vector<int> getFinishVec() { return act_finish; }
    double getNPV(int act) { return NPV[act]; }

}*best_bees;

class Elite
{
private:
    vector<int> solution;
    int duration;
    vector<vector<int>> Gannt;
    double Total_NPV;
    vector<double> NPV;
    vector<int> act_start;
    vector<int> act_finish;
public:
    Elite()
    {
        solution.clear();
        Total_NPV = 0.0;
        NPV.clear();
        duration = 0;
        Gannt.clear();
        act_start.clear();
        act_finish.clear();
    }
    Elite(vector<int> in_sol, int in_dur, vector<vector<int>> in_gannt, double in_total_npv, vector<double> in_npv, vector<int> in_start, vector<int> in_finish)
    {
        solution = in_sol;
        duration = in_dur;
        Gannt = in_gannt;
        Total_NPV = in_total_npv;
        NPV = in_npv;
        act_start = in_start;
        act_finish = in_finish;
    }
    ~Elite() {

    }
    int getStart(int pos) { return act_start[pos]; }
    void setStart(int pos, int value) { act_start[pos] = value; }
    void setNEWStart(int pos, int value) { act_start[pos] = act_start[pos] + value; }
    vector<int> getStartVec() { return act_start; }
    int getFinish(int act) { return act_finish[act]; }
    void setFinish(int act, int value) { act_finish[act] = act_start[act] + value; }
    void setNEWFinish(int act, int value) { act_finish[act] = value; }
    vector<int> getFinishVec() { return act_finish; }
    vector<vector<int>> getGannt() { return Gannt; }
    void setGannt(vector<vector<int>> value) { Gannt = value; }
    void resizeGannt(int rows, int new_size) { Gannt.resize(rows, vector<int>(new_size)); }
    int PrintGannt(int time, int resource) { return Gannt[time][resource]; }
    int getSol(int pos) { return solution[pos]; }
    size_t getSolSize() { return solution.size(); }
    vector<int> getSolVec() { return solution; }
    int getDuration() { return duration; }
    void setDurationNEW(int value) { duration = value; }
    double getNPV(int act) { return NPV[act]; }
    void setNPV(int act, double value) { NPV[act] = value; }
    vector<double> getNPVVec() { return NPV; }
    void setTotalNPV(double val) { Total_NPV = val; }
    double getTotalNPV() { return Total_NPV; }

}*elite_bees;

class Opt
{
private:
    vector<int> solution;
    int duration;
    vector<vector<int>> Gannt;
    double Total_NPV;
    vector<double> NPV;
    vector<int> act_start;
    vector<int> act_finish;
public:
    Opt()
    {
        solution.clear();
        Total_NPV = 0.0;
        NPV.clear();
        duration = 0;
        Gannt.clear();
        act_start.clear();
        act_finish.clear();
    }
    Opt(vector<int> in_sol, int in_dur, vector<vector<int>> in_gannt, double in_total_npv, vector<double> in_npv, vector<int> in_start, vector<int> in_finish)
    {
        solution = in_sol;
        duration = in_dur;
        Gannt = in_gannt;
        Total_NPV = in_total_npv;
        NPV = in_npv;
        act_start = in_start;
        act_finish = in_finish;
    }
    ~Opt() {

    }
    size_t getGanntSize() { return Gannt.size(); }
    int PrintGannt(int time, int resource) { return Gannt[time][resource]; }
    double getNPV(int act) { return NPV[act]; }
    int getFinish(int act) { return act_finish[act]; }
    int getStart(int pos) { return act_start[pos]; }
    int getSol(int pos) { return solution[pos]; }
    size_t getSolSize() { return solution.size(); }
    double getTotalNPV() { return Total_NPV; }
    int getDuration() { return duration; }
}*final;

#endif