#pragma once
#include "chartcontrol.h"
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/spinctrl.h>
#include <thread>
#include <chrono>
#include <random>
#include <omp.h>
#include <iostream>
#include <vector>
using namespace std;
				
class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);
	// menu
	void OnMenuOpen(wxCommandEvent& evt);
	void OnMenuExit(wxCommandEvent& evt);
	//menu1
	void ExportCSV(wxCommandEvent& evt);
	void ExportSA(wxCommandEvent& evt);
	//menu2
	void OnResourceGraph(wxCommandEvent& evt);
	void OnSense(wxCommandEvent& evt);
	//menu3
	void OnHelp(wxCommandEvent& evt);
	void OnSolInit(wxCommandEvent& evt);
	void OnForagerSPCTRL(wxSpinEvent& evt);
	void OnRateSpin(wxSpinDoubleEvent& evt);
	void OnInitialSpin(wxSpinEvent& evt);
	void OnResourceSpin(wxSpinEvent& evt);
	void OnSolutionSpin(wxSpinEvent& evt);
	void OnRun(wxCommandEvent& evt);
	void OnChoice(wxCommandEvent& evt);
	void OnClose(wxCloseEvent& evt);
	void OnDefineRates(wxCommandEvent& evt);
	void OnSensitivity(wxCommandEvent& evt);
	void data();
	void CreatePredecessors();
	int createCF();
	int Early_Start(int act, int trigger, int s_index, int frg);
	int Latest_Finish(int act);
	vector<int> Forward_Activity_Check(vector<int> next_act, int* act, vector<double> random, int trigger, int s_index, int frg);
	vector<int> Forward_Activity_List(vector<double> random, int trigger, int s_index, int frg);
	vector<vector<int>> Forward_Resource_Check(vector<vector<int>> GanntR, vector<int> solution, double* npv, int* finish, int trigger, int s_index, int frg);
	void SSGS_Forward(vector<double> random, int s_index, int frg, int trigger);
	void Local_Search(int local_size, int scout, vector<bool> dummy_status, vector<int> dummy_start, vector<int> dummy_finish, vector<double> dummy_npv, vector<bool> dummy_check);
	vector<int> delay(vector<int> set, vector<int> set2, int sol);
	bool Push_Delay(int bestbee, int delay, vector<int> set1, int deadline);
	void Find_Network_Set(int current, int start, int pt1[], int pt2[], double* total_npv, int sol, bool status, int count1, int count2);
	void Network_Delay(int deadline, int bestbee);
	void Find_Schedule_Set(int current, int start, int pt1[], double* total_npv, int sol, bool status, int count1);
	vector<int> Schedule_Set_2(vector<int> set1, int sol);
	void Schedule_Delay(int bestbee, int deadline);
	void Check_Schedule(int bees);
	void Initial_Population(int N, vector<double> random, vector<int> solution);

protected: 
	bool processing{ false };
	std::atomic<bool> quitRequested{ false };
	wxGauge* gauge;
	wxGauge* Netgauge;
	double dur = 0.0;
	double dur_sec = 0.0;
	wxString va;
	wxPanel* panel;
	int time_limit = 300;
	int global_counter;
	thread backgroundThread;
	wxButton* run_button;
	wxButton* init;
	wxStaticText* gauge_text;
	wxMenu* menu = new wxMenu();
	wxMenu* menu1 = new wxMenu();
	wxMenu* menu2 = new wxMenu();
	wxMenu* menu3 = new wxMenu();
	wxStaticText* label;
	wxStaticText* label1;
	int first = 0;
	ChartControl* chart; 
	ChartControlSA* gchart;
	wxSpinCtrl* resource_count;
	wxSpinCtrl* solution_count;
	wxButton* plot;
	wxChoice* choice;
	string ch =("Comma");
	wxDECLARE_EVENT_TABLE();
};

class NumberDlg : public wxDialog
{
public:
	NumberDlg() : wxDialog(NULL, wxID_ANY, _("Specify number of rates"))
	{
		wxFlexGridSizer* rate_size_win = new wxFlexGridSizer(2);
		rate_size_win->Add(new wxStaticText(this, wxID_ANY, _("&Rate Size:")), 0, wxALL | wxALIGN_RIGHT, 5);
		rate_size = new wxTextCtrl(this, wxID_ANY);
		rate_size_win->Add(rate_size, 0, wxALL, 5);
		wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(rate_size_win, 0, wxALL, 5);
		mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL, 5);
		SetSizerAndFit(mainSizer);
		Centre();
	}
	int GetNumber() const
	{
		int test = wxAtoi(rate_size->GetValue());
		return test;
	};
	void SetNumber(const wxString& user)
	{
		rate_size->SetValue(user);
	}
private:
	wxTextCtrl* rate_size;
};

class RateDlg : public wxDialog
{
public:
	RateDlg() : wxDialog(NULL, wxID_ANY, _("Enter rate"))
	{
		wxFlexGridSizer* rate_win = new wxFlexGridSizer(2);
		rate_win->Add(new wxStaticText(this, wxID_ANY, _("&Rate:")), 0, wxALL | wxALIGN_RIGHT, 5);
		rate_input = new wxTextCtrl(this, wxID_ANY);
		rate_win->Add(rate_input, 0, wxALL, 5);
		wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
		mainSizer->Add(rate_win, 0, wxALL, 5);
		mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL, 5);
		SetSizerAndFit(mainSizer);
		Centre();
	}
	double GetNumber() const
	{
		double test = wxAtof(rate_input->GetValue());
		return test;
	};
	void SetNumber(const wxString& user)
	{
		rate_input->SetValue(user);
	}
private:
	wxTextCtrl* rate_input;
};