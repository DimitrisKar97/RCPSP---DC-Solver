#include "MainFrame.h"
#include "rcpsp.h"
#include "bba.h"
#include "chartcontrol.h"
#include <wx/wx.h>
#include <wx/textfile.h>
#include <wx/event.h>
#include <wx/spinctrl.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/hyperlink.h>
#include <wx/datetime.h>
#include <wx/thread.h>
#include "wx/wxprec.h"
#include <thread>
#include <chrono>
#include <random>
#include <omp.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;
using namespace std::chrono;

class App : public wxApp
{
public:
    bool OnInit();
};

DECLARE_APP(App);

bool App::OnInit() {
    MainFrame* mainframe = new MainFrame("GUI");
    mainframe->SetClientSize(1000, 600);
    mainframe->Center();

    mainframe->Show();
    return true;
}
wxIMPLEMENT_APP(App);

int f_counter = 0;
constexpr double MIN = 0.001;
constexpr double MAX = 0.999;
random_device rd;
default_random_engine eng(rd());
uniform_real_distribution<double> distr(MIN, MAX);

string filename;


wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(101, MainFrame::OnMenuOpen)
EVT_MENU(103, MainFrame::OnMenuExit)
EVT_MENU(110, MainFrame::ExportCSV)
EVT_CLOSE(MainFrame::OnClose)
EVT_MENU(130, MainFrame::OnHelp)
EVT_MENU(120, MainFrame::OnDefineRates)
EVT_MENU(121, MainFrame::OnSensitivity)
EVT_MENU(122, MainFrame::OnSense)
EVT_MENU(111, MainFrame::ExportSA)
wxEND_EVENT_TABLE()

int initial = 5;
int frg = 10;
double rate = 0.05;
int res = 1;
int sa_sol = 0;
int separatot = 1;
vector<double> s_rates;

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
    panel = new wxPanel(this);
	wxMenuBar* menub = new wxMenuBar();
	this->SetMenuBar(menub);
	menu->Append(101, "Open");
    menu->AppendSeparator();
	menu->Append(103, "Exit");

    menu1->Append(110,"Export to CSV"); 
    menu1->Enable(110, false);
    menu1->AppendSeparator();
    menu1->Append(111, "Export SA");
    menu1->Enable(111, false);
    menu2->Append(120, "Define Rates");
    menu2->Append(121, "Run Sensitivity Analysis");
    menu2->Append(122, "Plot Sensitivity Analysis");
    menu2->Enable(122, false);

    menu3->Append(130, "Steps");

	menub->Append(menu, "File");
    menub->Append(menu1, "Export Solution");
    menub->Append(menu2, "Sensitivity Analysis");
    menub->Append(menu3, "Help");

	wxStaticText* initial_text = new wxStaticText(panel, wxID_ANY, "Insert Size of Scouts:", wxPoint(20, 33), wxDefaultSize);
	wxSpinCtrl* initial_count = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(210, 30), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 1, 100, 5);
	wxStaticText* initial_size_text = new wxStaticText(panel, wxID_ANY, "Specify the size of Initial Population", wxPoint(330, 33), wxDefaultSize);
	wxFont font = initial_size_text->GetFont();
	font.SetPointSize(10);
	font.SetWeight(wxFONTWEIGHT_EXTRABOLD);
	initial_size_text->SetFont(font);

	initial_count->Bind(wxEVT_SPINCTRL, &MainFrame::OnInitialSpin, this);

	wxStaticText* forager_text = new wxStaticText(panel, wxID_ANY, "Insert Number of Foragers:", wxPoint(20, 93), wxDefaultSize,wxALIGN_LEFT);
	wxSpinCtrl* forager_count = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(210, 90), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 10, 2000, 50);
	wxStaticText* forager_size_text = new wxStaticText(panel, wxID_ANY, "Specify the size of Local Search", wxPoint(330, 93), wxDefaultSize);
    wxFont font1 = forager_size_text->GetFont();
    font.SetPointSize(10);
    font.SetWeight(wxFONTWEIGHT_EXTRABOLD);
    forager_size_text->SetFont(font);
	
	forager_count->Bind(wxEVT_SPINCTRL, &MainFrame::OnForagerSPCTRL, this);

	wxStaticText* rate_text = new wxStaticText(panel, wxID_ANY, "Insert rate:", wxPoint(20, 150), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	wxSpinCtrlDouble* rate_count = new wxSpinCtrlDouble(panel, wxID_ANY, "", wxPoint(210, 150), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP,0.01,0.99,0.05,0.01);

	rate_count->Bind(wxEVT_SPINCTRLDOUBLE, &MainFrame::OnRateSpin, this);

    wxStaticText* solution_text = new wxStaticText(panel, wxID_ANY, "Plot graph for Solution:", wxPoint(20, 210), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    solution_count = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(210, 210), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 1, 1, 1);
    solution_count->Bind(wxEVT_SPINCTRL, &MainFrame::OnSolutionSpin, this);
   
    wxStaticText* resource_text = new wxStaticText(panel, wxID_ANY, "Plot graph for Resource:", wxPoint(20, 270), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    resource_count = new wxSpinCtrl(panel, wxID_ANY, "", wxPoint(210, 270), wxSize(80, -1), wxSP_ARROW_KEYS | wxSP_WRAP, 1, 1, 1);
    resource_count->Bind(wxEVT_SPINCTRL, &MainFrame::OnResourceSpin, this);

    plot = new wxButton(panel, wxID_ANY, "Plot", wxPoint(330, 210), wxSize(85, 25));
   
    plot->Enable(false);

    wxStaticText* choice_text = new wxStaticText(panel, wxID_ANY, "Specify Separator:", wxPoint(20, 330), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    wxArrayString choices;
    choices.Add("Comma");
    choices.Add("Semicolon");

    choice = new wxChoice(panel, wxID_ANY, wxPoint(210, 330), wxDefaultSize, choices);
    choice->Bind(wxEVT_CHOICE, &MainFrame::OnChoice, this);
    choice->Select(0);

    wxStaticText* local_text = new wxStaticText(panel, wxID_ANY, "Local search:", wxPoint(50, 510), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	gauge = new wxGauge(panel, wxID_ANY, initial, wxPoint(50, 530), wxSize(650, -1), wxGA_SMOOTH, wxDefaultValidator);
    gauge->SetValue(0);   
    wxStaticText* delay_text = new wxStaticText(panel, wxID_ANY, "Activities Delay:", wxPoint(50, 550), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    Netgauge = new wxGauge(panel, wxID_ANY, initial, wxPoint(50, 570), wxSize(650, -1), wxGA_SMOOTH, wxDefaultValidator);
    Netgauge->SetValue(0);   
    
    label = new wxStaticText(panel, 110, "", wxPoint(720, 570), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    label1 = new wxStaticText(panel, 110, "", wxPoint(720, 550), wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);

	run_button = new wxButton(panel, wxID_ANY, "Run", wxPoint(50, 445), wxSize(85, 25));
	init = new wxButton(panel, wxID_ANY, "Initialize Solution", wxPoint(50, 480), wxSize(130, 25));
	run_button->Bind(wxEVT_BUTTON, &MainFrame::OnRun, this);

    plot->Bind(wxEVT_BUTTON, &MainFrame::OnResourceGraph, this);
	init->Bind(wxEVT_BUTTON, &MainFrame::OnSolInit, this);

    init->Enable(false);
    run_button->Enable(false);
    menu2->Enable(121, false);
	CreateStatusBar();
}

void MainFrame::OnSolutionSpin(wxSpinEvent& evt) {
    sa_sol = evt.GetValue() - 1;
    wxLogStatus("Solution set to %d", evt.GetValue());
}

void  MainFrame::OnChoice(wxCommandEvent& evt)
{
    ch = evt.GetString();
    wxLogStatus("Choice set to %s", evt.GetString());
}

void MainFrame::ExportSA(wxCommandEvent& evt) {
    if (backgroundThread.joinable()) { this->backgroundThread.join(); }
    string c1("Semicolon");
    string c2("Comma");
    if (ch == c2) {
        wxFileDialog dlg(this, "Save Sensitivity Analysis", "", "", "Save Files(*.csv) | *.csv | All files(*.*) | *.*", wxFD_SAVE);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFile file(dlg.GetPath(), wxFile::write);
            if (file.IsOpened())
            {
                file.Write("Date: ,");
                wxString tt;
                wxDateTime now = wxDateTime::Now();
                wxString today_str = now.FormatDate();
                wxString hour_str = now.FormatTime();
                file.Write(today_str);
                file.Write(",");
                file.Write(hour_str);
                file.Write(wxT("\n"));
                file.Write(filename);
                file.Write("\nTime: ,");

                wxString txt1;
                txt1.Printf(wxT("%.2lf [ms] || %.2f [s]"), dur, dur_sec);
                file.Write(txt1);
                file.Write("\nLoops:,");
                int ll;
                int l = int(s_rates.size());
                ll = l * (initial * frg) + f_counter * frg;
                wxString loop_text;
                loop_text.Printf(wxT("%d"), ll);
                file.Write(loop_text);
                for (size_t i = 0; i < s_rates.size(); i++)
                {
                    file.Write("\n\nRate: ,");
                    wxString txt;
                    txt.Printf(wxT("%1.3lf"), s_rates[i]);
                    file.Write(txt);
                    file.Write("\n");

                    file.Write("Duration: , ");
                    wxString txt3;
                    txt3.Printf(wxT("%d"), final[i].getDuration());
                    file.Write(txt3);
                    file.Write("\n");
                    file.Write("NPVCum: ");
                    wxString txt4;
                    txt4.Printf(wxT(", %5.2lf"), final[i].getTotalNPV());
                    file.Write(txt4);
                    file.Write("\n");
                    file.Write("Act: ");
                    file.Write(",");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%d,"), final[i].getSol(j));
                        file.Write(txt);
                    }
                    file.Write("\n");
                    file.Write("Start: ");
                    file.Write(",");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%d, "), final[i].getStart(final[i].getSol(j)));
                        file.Write(txt);
                    }
                    file.Write("\n");
                    file.Write("Finish: ");
                    file.Write(",");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%d, "), final[i].getFinish(final[i].getSol(j)));
                        file.Write(txt);
                    }
                    file.Write("\n");
                    file.Write("NPV: ");
                    file.Write(",");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%5.2lf,"), final[i].getNPV(final[i].getSol(j)));
                        file.Write(txt);
                    }
                    file.Write("\n\n");
                    for (int j = 0; j < resourceNum; j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("R%d,"), j + 1);
                        file.Write(txt);
                        for (int k = 0; k < final[i].getDuration(); k++)
                        {
                            wxString txt1;
                            txt1.Printf(wxT("%d,"), final[i].PrintGannt(k, j));
                            file.Write(txt1);
                        }
                        file.Write("\n");
                    }
                    file.Write("Time:,");
                    for (int k = 0; k < final[i].getDuration(); k++)
                    {
                        wxString txt1;
                        txt1.Printf(wxT("%d,"), k + 1);
                        file.Write(txt1);
                    }
                    file.Write("\n");
                }
                file.Close();
                wxMessageBox(".csv File Created.");
            }
        }
    }

    else if(ch == c1)
    {
        wxFileDialog dlg(this, "Save Sensitivity Analysis", "", "", "Save Files(*.csv) | *.csv | All files(*.*) | *.*", wxFD_SAVE);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFile file(dlg.GetPath(), wxFile::write);
            if (file.IsOpened())
            {
                file.Write("Date: ;");
                wxString tt;
                wxDateTime now = wxDateTime::Now();
                wxString today_str = now.FormatDate();
                wxString hour_str = now.FormatTime();
                file.Write(today_str);
                file.Write(";");
                file.Write(hour_str);
                file.Write(wxT("\n"));
                file.Write(filename);
                file.Write("\nTime: ;");

                wxString txt1;
                txt1.Printf(wxT("%.2lf [ms] || %.2f [s]"), dur, dur_sec);
                file.Write(txt1);
                file.Write("\nLoops:;");
                int ll;
                int l = int(s_rates.size());
                ll = l * (initial * frg) + f_counter * frg;
                wxString loop_text;
                loop_text.Printf(wxT("%d"), ll);
                file.Write(loop_text);
                for (size_t i = 0; i < s_rates.size(); i++)
                {
                    file.Write("\n\nRate: ;");
                    wxString txt;
                    txt.Printf(wxT("%1.3lf"), s_rates[i]);
                    file.Write(txt);
                    file.Write("\n");

                    file.Write("Duration: ; ");
                    wxString txt3;
                    txt3.Printf(wxT("%d"), final[i].getDuration());
                    file.Write(txt3);
                    file.Write("\n");
                    file.Write("NPVCum: ");
                    wxString txt4;
                    txt4.Printf(wxT("; %5.2lf"), final[i].getTotalNPV());
                    file.Write(txt4);
                    file.Write("\n");
                    file.Write("Act: ");
                    file.Write(";");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%d;"), final[i].getSol(j));
                        file.Write(txt);
                    }
                    file.Write("\n");
                    file.Write("Start: ");
                    file.Write(";");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%d; "), final[i].getStart(final[i].getSol(j)));
                        file.Write(txt);
                    }
                    file.Write("\n");
                    file.Write("Finish: ");
                    file.Write(";");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%d; "), final[i].getFinish(final[i].getSol(j)));
                        file.Write(txt);
                    }
                    file.Write("\n");
                    file.Write("NPV: ");
                    file.Write(";");
                    for (int j = 0; j < final[i].getSolSize(); j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("%5.2lf;"), final[i].getNPV(final[i].getSol(j)));
                        file.Write(txt);
                    }
                    file.Write("\n\n");
                    for (int j = 0; j < resourceNum; j++)
                    {
                        wxString txt;
                        txt.Printf(wxT("R%d;"), j + 1);
                        file.Write(txt);
                        for (int k = 0; k < final[i].getDuration(); k++)
                        {
                            wxString txt1;
                            txt1.Printf(wxT("%d;"), final[i].PrintGannt(k, j));
                            file.Write(txt1);
                        }
                        file.Write("\n");
                    }
                    file.Write("Time:;");
                    for (int k = 0; k < final[i].getDuration(); k++)
                    {
                        wxString txt1;
                        txt1.Printf(wxT("%d;"), k + 1);
                        file.Write(txt1);
                    }
                    file.Write("\n");
                }
                file.Close();
                wxMessageBox(".csv File Created.");
            }
        }
    }
    return;
}

void MainFrame::OnDefineRates(wxCommandEvent& evt)
{
    NumberDlg dlg;
    RateDlg rdlg;
    int t = 0;
    while (t < 2)
    {
        dlg.ShowModal();
        t = dlg.GetNumber();
        wxLogStatus("Rate number: %d", t);
        if (t <= 1)
        {
            if (wxMessageBox(_("Invalid value, try again?"),
                _("Question"), wxYES_NO) != wxYES)
            {
                break;
            }
        }
    }   
    if (t > 1)
    {
        s_rates.clear();
        int flag = 1;
        double val = 0.0;
        int co = 0;
        while (co < t)
        {
            if (rdlg.ShowModal() == wxID_CANCEL)
            {
                flag = 0;
                break;
            }
            if (rdlg.GetNumber() > 0.00001)
            {
                s_rates.push_back(rdlg.GetNumber());
                co++;
            }            
        }
        if (flag == 1) {
            f_counter = 0;
            menu2->Enable(121, true);
        }
    }    
    sort(s_rates.begin(), s_rates.end());
    wxString Foobar;
}

void MainFrame::OnSense(wxCommandEvent& evt)
{
    wxFrame* chart_fr = new wxFrame(this, wxID_ANY, "Chart Frame", wxPoint(1000, 300), wxSize(600, 500));
    gchart = new ChartControlSA(chart_fr, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    auto text = wxString::Format("NPV - Rate Chart");    
    gchart->title = text;
    gchart->lv = s_rates[0];
    gchart->hv = *std::max_element(s_rates.begin(), s_rates.end());
    gchart->number = s_rates.size();
    for (int i = 0; i < s_rates.size(); i++)
    {
        gchart->rate_values.push_back(s_rates[i]);
        gchart->npv_values.push_back(final[i].getTotalNPV());
    }
   
    chart_fr->Show();
}

void MainFrame::OnResourceGraph(wxCommandEvent& evt)
{
    wxFrame* chart_fr = new wxFrame(this, wxID_ANY, "Chart Frame", wxPoint(1000, 300), wxSize(600, 500));
    chart = new ChartControl(chart_fr, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    auto text = wxString::Format("Resource %d Usage Chart, Duration = %d, Solution %d, NPV = %.2f", res, final[sa_sol].getDuration(), sa_sol + 1, final[sa_sol].getTotalNPV());
    chart->title = text;
    chart->max = resourceCap[res - 1];
    chart->min = 0.0;
    chart->time_value = final[sa_sol].getDuration();

    for (int i = 0; i < final[sa_sol].getGanntSize(); i++)
        chart->values.push_back(final[sa_sol].PrintGannt(i, res - 1));

    chart_fr->Show();
}

void MainFrame::OnSensitivity(wxCommandEvent& evt)
{
    run_button->Enable(false);
    gauge->SetRange(initial);
    Netgauge->SetRange(initial);
    if (!this->processing)
    {
        this->processing = true;
        const auto f = [this]()
        {
            auto startTime = high_resolution_clock::now();

            double X;
            vector<double> random; random.clear();
            vector<int> solution; solution.clear();
            data();
            resource_count->SetRange(1, resourceNum);
            CreatePredecessors();
            createCF();
            vector<bool> dummy_status; dummy_status.clear();
            vector<int> dummy_start; dummy_start.clear();
            vector<int> dummy_finish; dummy_finish.clear();
            vector<double> dummy_npv; dummy_npv.clear();
            vector<bool> dummy_check; dummy_check.clear();
            for (int i = 0; i < inputs; i++)
            {
                if (i == 0) dummy_status.push_back(true);
                else dummy_status.push_back(false);
                dummy_start.push_back(0);
                dummy_finish.push_back(0);
                dummy_npv.push_back(0.0);

                dummy_check.push_back(false);
            }
            int rate_size = s_rates.size();
            final = new Opt[rate_size];
            int position = 0;
            for (size_t s = 0; s < rate_size; s++)
            {
                rate = s_rates[s];
                int nn = 0;                     // counter gia while
                scout_bees = new In_Pop[initial];
                for (int i = 0; i < initial; i++)
                    scout_bees[i] = In_Pop();
                while (nn < initial)
                {
                    random.clear();
                    random.push_back(1.0);   // pithanotita start
                    for (int i = 0; i < inputs - 1; i++)                        // random values gia oles tis activities
                    {
                        X = distr(eng);
                        random.push_back(X);
                    }
                    random.push_back(0.0);  //pithanotita finish
                    TotalNPV = 0.0;
                    SSGS_Forward(random, nn, 0, 0);
                    for (int i = 0; i < inputs; i++)
                    {
                        element[i].setNPV(element[i].getCashFlow(), rate, element[i].getFinish());
                        TotalNPV += element[i].getNPV();
                    }
                    scout_bees[nn] = In_Pop(scout_bees[nn].GetSolution(), TotalNPV, scout_bees[nn].GetSolutionDuration(), scout_bees[nn].GetPrio());
                    nn++;
                    solution.clear();
                }

                forager_bees = new Foragers * *[initial];
                best_bees = new Best[initial];
                elite_bees = new Elite[initial];
                for (int i = 0; i < initial; i++)
                {
                    forager_bees[i] = new Foragers * [frg];
                    for (int j = 0; j < frg; j++) {
                        forager_bees[i][j] = new Foragers();            // i = scout
                    }
                }
                for (int i = 0; i < initial; i++)
                {
                    Local_Search(frg, i, dummy_status, dummy_start, dummy_finish, dummy_npv, dummy_check);
                    wxGetApp().CallAfter([this, i]()
                        {this->gauge->SetValue(i + 1); });
                }

                delete[] scout_bees;
                delete[] forager_bees;
                for (int n = 0; n < initial; n++)
                {
                    wxGetApp().CallAfter([this, n]()
                        {this->Netgauge->SetValue(n + 1); });
                    int del = best_bees[n].getFinish(inputs - 1) + int(best_bees[n].getFinish(inputs - 1) * 0.15);      // deadline == arxiko delay sthn finish
                    Network_Delay(del, n);                     // kalw to NPV Optimization
                    double t = 0.0;
                    for (int i = 0; i < inputs; i++)
                    {
                        t += elite_bees[n].getNPV(i);
                    }
                    elite_bees[n].setTotalNPV(t);
                    t = 0.0;
                    Schedule_Delay(n, del);
                    for (int i = 0; i < inputs; i++)
                    {
                        t += elite_bees[n].getNPV(i);
                    }
                    elite_bees[n].setTotalNPV(t);
                }
                Check_Schedule(initial);
                double best_npv = best_bees[0].getTotalNPV();
                int best_time = 10000;
                for (int i = 0; i < initial; i++)
                {
                    if (elite_bees[i].getTotalNPV() > best_npv)
                    {
                        best_npv = elite_bees[i].getTotalNPV();
                        first = i;
                    }
                }
                final[position] = Opt(elite_bees[first].getSolVec(), elite_bees[first].getDuration(), elite_bees[first].getGannt(), elite_bees[first].getTotalNPV(),
                    elite_bees[first].getNPVVec(), elite_bees[first].getStartVec(), elite_bees[first].getFinishVec());

                delete[] best_bees;
                delete[] elite_bees;
                position++;
            }
            menu2->Enable(122, true);
            menu1->Enable(111, true);
            plot->Enable(true);
            init->Enable(true);
            wxMessageBox("Solution Found");
            auto stopTime = high_resolution_clock::now();
            auto duration = stopTime - startTime;
            dur = std::chrono::duration<double, std::milli>(duration).count();
            dur_sec = std::chrono::duration<double>(duration).count();
            this->label1->SetLabelText(wxString::Format("Processing time: %.2f [ms]", std::chrono::duration<double, std::milli>(duration).count()));
            this->label->SetLabelText(wxString::Format("Processing time: %.2f [s]", std::chrono::duration<double>(duration).count()));
            solution_count->SetRange(1, rate_size);
        };
        this->backgroundThread = thread{ f };
    }
}

void MainFrame::OnHelp(wxCommandEvent& evt){
    wxFrame* help_fr = new wxFrame(this, wxID_ANY, "Steps", wxPoint(1000, 300), wxSize(600, 500));
    wxPanel* panel1 = new wxPanel(help_fr);
    wxStaticText* test_txt = new wxStaticText(panel1, wxID_ANY, "Step 1: Specify the size of the Initial Population (Scout Bees).", wxPoint(15, 20), wxDefaultSize);
    wxStaticText* test_txt1 = new wxStaticText(panel1, wxID_ANY, "Step 2: Specify the size of the Forager Bees (loops per Scout Bee).", wxPoint(15, 50), wxDefaultSize);
    wxStaticText* note = new wxStaticText(panel1, wxID_ANY, "Important: The pool of solutions will be at least the size of Scout Bees * Forager Bees.", wxPoint(15, 70), wxDefaultSize);
    wxFont font = note->GetFont();
    font.SetPointSize(9);
    font.SetWeight(wxFONTWEIGHT_EXTRABOLD);
    note->SetFont(font);
    wxStaticText* test_txt2 = new wxStaticText(panel1, wxID_ANY, "Step 3: Specify rate.", wxPoint(15, 95), wxDefaultSize);
    wxStaticText* test_txt3 = new wxStaticText(panel1, wxID_ANY, "Step 4: Open .RCP or .txt file (", wxPoint(15, 120), wxDefaultSize);
    wxHyperlinkCtrl* thyper = new wxHyperlinkCtrl(panel1, wxID_ANY, " Patterson ", "http://www.p2engine.com/p2reader/patterson_format", wxPoint(171, 120), wxDefaultSize);
    wxStaticText* test_txt3_1 = new wxStaticText(panel1, wxID_ANY, "format ONLY).", wxPoint(232, 120), wxDefaultSize);
    wxStaticText* test_txt4 = new wxStaticText(panel1, wxID_ANY, "Step 5: Run algorithm.", wxPoint(15, 145), wxDefaultSize);
    wxStaticText* test_txt5 = new wxStaticText(panel1, wxID_ANY, "Step 6: Export Solution.", wxPoint(15, 170), wxDefaultSize);
    help_fr->Show();
}

void MainFrame::OnRun(wxCommandEvent& evt)
{
    f_counter = 0;
    run_button->Enable(false);
    wxYield();
    gauge->SetRange(initial);
    Netgauge->SetRange(initial);
    if (!this->processing)
    {
        this->processing = true;
        const auto f = [this]()
        {
            auto startTime = high_resolution_clock::now();
            vector<double> random; random.clear();
            vector<int> solution; solution.clear();
            data();
            resource_count->SetRange(1, resourceNum);
            CreatePredecessors();
            createCF();
            vector<bool> dummy_status; dummy_status.clear();
            vector<int> dummy_start; dummy_start.clear();
            vector<int> dummy_finish; dummy_finish.clear();
            vector<double> dummy_npv; dummy_npv.clear();
            vector<bool> dummy_check; dummy_check.clear();
            for (int i = 0; i < inputs; i++)
            {
                if (i == 0) dummy_status.push_back(true);
                else dummy_status.push_back(false);
                dummy_start.push_back(0);
                dummy_finish.push_back(0);
                dummy_npv.push_back(0.0);
                dummy_check.push_back(false);
            }

            scout_bees = new In_Pop[initial];
            for (int i = 0; i < initial; i++)
                scout_bees[i] = In_Pop();

            Initial_Population(initial, random, solution);                     
            
            forager_bees = new Foragers * *[initial];
            best_bees = new Best[initial];
            elite_bees = new Elite[initial];
            for (int i = 0; i < initial; i++)
            {
                forager_bees[i] = new Foragers * [frg];
                for (int j = 0; j < frg; j++) {
                    forager_bees[i][j] = new Foragers();            // i = scout
                }
            }
            for (int i = 0; i < initial; i++)
            {
                Local_Search(frg, i, dummy_status, dummy_start, dummy_finish, dummy_npv, dummy_check);
                wxGetApp().CallAfter([this, i]()
                    {this->gauge->SetValue(i + 1); });
            }

            delete[] scout_bees;
            delete[] forager_bees;
            for (int n = 0; n < initial; n++)
            {
                wxGetApp().CallAfter([this, n]()
                    {this->Netgauge->SetValue(n + 1); });
                int del = best_bees[n].getFinish(inputs - 1) + int(best_bees[n].getFinish(inputs - 1) * 0.15);      // deadline == arxiko delay sthn finish
                Network_Delay(del, n);                     // kalw to NPV Optimization
                double t = 0.0;
                for (int i = 0; i < inputs; i++)
                {
                    t += elite_bees[n].getNPV(i);
                }
                elite_bees[n].setTotalNPV(t);
                t = 0.0;
                Schedule_Delay(n, del);
                for (int i = 0; i < inputs; i++)
                {
                    t += elite_bees[n].getNPV(i);
                }
                elite_bees[n].setTotalNPV(t);
            }
            Check_Schedule(initial);
            final = new Opt[1];
            double best_npv = best_bees[0].getTotalNPV();
            int best_time = 10000;
            for (int i = 0; i < initial; i++)
            {
                if (elite_bees[i].getTotalNPV() > best_npv)
                {
                    best_npv = elite_bees[i].getTotalNPV();
                    first = i;
                }
            }
            final[0] = Opt(elite_bees[first].getSolVec(), elite_bees[first].getDuration(), elite_bees[first].getGannt(), elite_bees[first].getTotalNPV(),
                elite_bees[first].getNPVVec(), elite_bees[first].getStartVec(), elite_bees[first].getFinishVec());
            delete[] best_bees;
            delete[] elite_bees;

            menu1->Enable(110, true);
            plot->Enable(true);
            init->Enable(true);
            wxMessageBox("Solution Found");
            auto stopTime = high_resolution_clock::now();
            auto duration = stopTime - startTime;
            dur = std::chrono::duration<double, std::milli>(duration).count();
            dur_sec = std::chrono::duration<double>(duration).count();
            this->label1->SetLabelText(wxString::Format("Processing time: %.2f [ms]", std::chrono::duration<double, std::milli>(duration).count()));
            this->label->SetLabelText(wxString::Format("Processing time: %.2f [s]", std::chrono::duration<double>(duration).count()));
        };
        this->backgroundThread = thread{ f };
    }
}

void MainFrame::OnResourceSpin(wxSpinEvent& evt)
{
    res = evt.GetValue();
    wxLogStatus("Resource set to %d", evt.GetValue());
}

void MainFrame::OnSolInit(wxCommandEvent& evt) {
    delete[] final;
    delete[] element;
    s_rates.clear();
    menu2->Enable(121, false);
    menu2->Enable(122, false);
    init->Enable(false);
    run_button->Enable(true);
    menu1->Enable(110,false);
    menu1->Enable(111,false);
    plot->Enable(false);
    gauge->SetValue(0);
    Netgauge->SetValue(0);
    this->label1->SetLabelText(wxString::Format(""));
    this->label->SetLabelText(wxString::Format(""));
    this->processing = false;
    if (backgroundThread.joinable()) { this->backgroundThread.join(); }
    wxMessageBox("Solution Deleted.");
}

void MainFrame::ExportCSV(wxCommandEvent& evt) {
    if (backgroundThread.joinable()) { this->backgroundThread.join(); }
    string c1("Semicolon");
    string c2("Comma");
    if (ch == c2) {
        wxFileDialog dlg(this, "Save Best Solutions", "", "", "Save Files(*.csv) | *.csv | All files(*.*) | *.*", wxFD_SAVE);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFile file(dlg.GetPath(), wxFile::write);
            if (file.IsOpened())
            {
                file.Write("Date: ,");
                wxString tt;
                wxDateTime now = wxDateTime::Now();
                wxString today_str = now.FormatDate();
                wxString hour_str = now.FormatTime();
                file.Write(today_str);
                file.Write(",");
                file.Write(hour_str);
                file.Write(wxT("\n"));
                file.Write(filename);
                file.Write("\nTime: ,");

                wxString txt1;
                txt1.Printf(wxT("%.2lf [ms] || %.2f [s]"), dur, dur_sec);
                file.Write(txt1);
                file.Write("\nLoops:,");
                int ll;
                ll = initial * frg + f_counter * frg;
                wxString loop_text;
                loop_text.Printf(wxT("%d"), ll);
                file.Write(loop_text);

                file.Write("\nRate: ,");
                wxString txt;
                txt.Printf(wxT("%1.3lf"), rate);
                file.Write(txt);
                file.Write("\n");
                file.Write(wxT("Solution: ,"));
                wxString txt2;
                txt2.Printf(wxT("%d"), first);
                file.Write(txt2);
                file.Write("\n");

                file.Write("Duration: , ");
                wxString txt3;
                txt3.Printf(wxT("%d"), final[0].getDuration());
                file.Write(txt3);
                file.Write("\n");
                file.Write("NPVCum: ");
                wxString txt4;
                txt4.Printf(wxT(", %5.2lf"), final[0].getTotalNPV());
                file.Write(txt4);
                file.Write("\n\n");
                file.Write("Act: ");
                file.Write(",");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%d,"), final[0].getSol(j));
                    file.Write(txt);
                }
                file.Write("\n");
                file.Write("Start: ");
                file.Write(",");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%d, "), final[0].getStart(final[0].getSol(j)));
                    file.Write(txt);
                }
                file.Write("\n");
                file.Write("Finish: ");
                file.Write(",");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%d, "), final[0].getFinish(final[0].getSol(j)));
                    file.Write(txt);
                }
                file.Write("\n");
                file.Write("NPV: ");
                file.Write(",");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%5.2lf,"), final[0].getNPV(final[0].getSol(j)));
                    file.Write(txt);
                }  
                file.Write("\n\n");
                for (int j = 0; j < resourceNum; j++)
                {
                    wxString txt;
                    txt.Printf(wxT("R%d,"), j + 1);
                    file.Write(txt);
                        for (int i = 0; i < final[0].getDuration(); i++)
                        {
                            wxString txt1;
                            txt1.Printf(wxT("%d,"), final[0].PrintGannt(i,j));
                            file.Write(txt1);
                        }
                        file.Write("\n");
                }
                file.Write("Time:,");
                for (int i = 0; i < final[0].getDuration(); i++)
                {
                    wxString txt1;
                    txt1.Printf(wxT("%d,"), i + 1);
                    file.Write(txt1);
                }
                file.Close();
                wxMessageBox(".csv File Created.");
            }
        }
    }

    else if (ch == c1) 
    {
        wxFileDialog dlg(this, "Save Best Solutions", "", "", "Save Files(*.csv) | *.csv | All files(*.*) | *.*", wxFD_SAVE);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFile file(dlg.GetPath(), wxFile::write);
            if (file.IsOpened())
            {
                file.Write("Date: ;");
                wxString tt;
                wxDateTime now = wxDateTime::Now();
                wxString today_str = now.FormatDate();
                wxString hour_str = now.FormatTime();
                file.Write(today_str);
                file.Write(";");
                file.Write(hour_str);
                file.Write(wxT("\n"));
                file.Write(filename);
                file.Write("\nTime: ;");

                wxString txt1;
                txt1.Printf(wxT("%.2lf [ms] || %.2f [s]"), dur, dur_sec);
                file.Write(txt1);
                file.Write("\nLoops:;");
                int ll;
                ll = initial * frg + f_counter * frg;
                wxString loop_text;
                loop_text.Printf(wxT("%d"), ll);
                file.Write(loop_text);

                file.Write("\nRate: ;");
                wxString txt;
                txt.Printf(wxT("%1.3lf"), rate);
                file.Write(txt);
                file.Write("\n");
                file.Write(wxT("Solution: ;"));
                wxString txt2;
                txt2.Printf(wxT("%d"), first);
                file.Write(txt2);
                file.Write("\n");

                file.Write("Duration: ; ");
                wxString txt3;
                txt3.Printf(wxT("%d"), final[0].getDuration());
                file.Write(txt3);
                file.Write("\n");
                file.Write("NPVCum: ");
                wxString txt4;
                txt4.Printf(wxT("; %5.2lf"), final[0].getTotalNPV());
                file.Write(txt4);
                file.Write("\n\n");
                file.Write("Act: ");
                file.Write(";");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%d;"), final[0].getSol(j));
                    file.Write(txt);
                }
                file.Write("\n");
                file.Write("Start: ");
                file.Write(";");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%d; "), final[0].getStart(final[0].getSol(j)));
                    file.Write(txt);
                }
                file.Write("\n");
                file.Write("Finish: ");
                file.Write(";");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%d; "), final[0].getFinish(final[0].getSol(j)));
                    file.Write(txt);
                }
                file.Write("\n");
                file.Write("NPV: ");
                file.Write(";");
                for (int j = 0; j < final[0].getSolSize(); j++)
                {
                    wxString txt;
                    txt.Printf(wxT("%5.2lf;"), final[0].getNPV(final[0].getSol(j)));
                    file.Write(txt);
                }
                file.Write("\n\n");
                for (int j = 0; j < resourceNum; j++)
                {
                    wxString txt;
                    txt.Printf(wxT("R%d;"), j + 1);
                    file.Write(txt);
                    for (int i = 0; i < final[0].getDuration(); i++)
                    {
                        wxString txt1;
                        txt1.Printf(wxT("%d;"), final[0].PrintGannt(i, j));
                        file.Write(txt1);
                    }
                    file.Write("\n");
                }
                file.Write("Time:;");
                for (int i = 0; i < final[0].getDuration(); i++)
                {
                    wxString txt1;
                    txt1.Printf(wxT("%d;"), i + 1);
                    file.Write(txt1);
                }
                file.Close();
                wxMessageBox(".csv File Created.");
                file.Close();
                wxMessageBox(".csv File Created.");
            }
        }
    }
    return;  
}


void MainFrame::OnInitialSpin(wxSpinEvent& evt) {
	initial = evt.GetValue();
	wxLogStatus("Initial Population Size set to %d", evt.GetValue());
}

void MainFrame::OnForagerSPCTRL(wxSpinEvent& evt) {
	frg = evt.GetValue();
	wxLogStatus("Foragers set to %d", evt.GetValue());
}

void MainFrame::OnRateSpin(wxSpinDoubleEvent& evt) {
	rate = evt.GetValue();
	wxLogStatus("Rate set to %2.3f", evt.GetValue());
}


void MainFrame::OnClose(wxCloseEvent& evt) {
    if (this->processing)
    {
        evt.Veto();
        if (backgroundThread.joinable()) this->backgroundThread.join();
        this->quitRequested = true;
    }
    else
    {
        if (backgroundThread.joinable()) this->backgroundThread.join();
        this->Destroy();
    }
	evt.Skip();
}

void MainFrame::OnMenuOpen(wxCommandEvent& evt) {
	wxFileDialog fileDlg(this, _("Choose file"), wxEmptyString, wxEmptyString, _("RCP file|*.RCP|TXT file|*.txt|All files|*.*"));
	if (fileDlg.ShowModal() == wxID_OK) {
		wxString path = fileDlg.GetPath();
        filename = path;
	}	
    run_button->Enable(true);    
}

void MainFrame::OnMenuExit(wxCommandEvent& evt) {
	Close();
	evt.Skip();
}


void MainFrame::data()
{
    ifstream infile(filename);
    string ch;
    int positive = 0;
    int line = 0;
    int act = 0, duration, * demand, numofSucc, * successors, start = 0;

    vector<int> activity;
    vector<int> act_duration;
    vector<int> in_vec_demand;
    vector<vector<int>> resource_array;         // diplos pinakas
    vector<int> in_succ;
    vector<vector<int>> successors_array;       // -//-
    vector<int> pred;
    vector<double> cash_flow;

    activity.clear();
    act_duration.clear();
    in_vec_demand.clear();
    resource_array.clear();
    in_succ.clear();
    successors_array.clear();
    pred.clear();
    cash_flow.clear();
    rcost.clear();

    if (!infile)
    {
        exit(1);
    }
    else
    {

        fstream infile(filename, fstream::in);
        while (!infile.eof())
        {
            if (line == 0)
            {
                infile >> inputs >> resourceNum;
                resourceCap = new int[resourceNum];
            }

            if (line == 1)
            {
                for (int i = 0; i < resourceNum; i++)
                {
                    infile >> resourceCap[i];
                }
            }

            if (line > 1)
            {
                in_vec_demand.clear();
                in_succ.clear();

                infile >> duration;
                act_duration.push_back(duration);

                demand = new int[resourceNum];
                for (int i = 0; i < resourceNum; i++)
                {
                    infile >> demand[i];
                    in_vec_demand.push_back(demand[i]);
                }
                resource_array.push_back(in_vec_demand);
                delete[] demand;

                infile >> numofSucc;
                successors = new int[numofSucc];
                for (int i = 0; i < numofSucc; i++)
                {
                    infile >> successors[i];
                    successors[i]--;
                    in_succ.push_back(successors[i]);
                }
                successors_array.push_back(in_succ);
                delete[] successors;

                activity.push_back(act);
                act++;
            }
            line++;
        }
    }
    infile.close();
    element = new rcpsp[inputs]; 
    for (int i = 0; i < inputs; i++)
        element[i] = rcpsp();
    for (int i = 0; i < inputs; i++)
    {
        vector<int> rdemand;
        vector<int> succ;
        for (size_t j = 0; j < resource_array[i].size(); j++)
            rdemand.push_back(resource_array[i][j]);
        for (size_t k = 0; k < successors_array[i].size(); k++)
            succ.push_back(successors_array[i][k]);
        element[i] = rcpsp(activity[i], act_duration[i], rdemand, int(successors_array[i].size()), succ, pred, 0.0, start, false, 0.0);
    }
    for (int i = 0; i < inputs; i++)
        MaxTime += element[i].getDuration();
}

void MainFrame::CreatePredecessors()
{
    for (int i = 0; i < inputs; i++)
        for (int j = 0; j < element[i].getNumOfSucc(); j++)
            element[element[i].getSuccessor(j)].PredecessorVector(element[i].getActivity());
}

int MainFrame::createCF()  
{
    int count = 0;
    for (int i = 0; i < resourceNum; i++) rcost.push_back(ResourceCost);
    double v;
    double vv = 0.0;
    for (int i = 1; i < inputs - 1; i++)
    {
        v = 0.0;
        for (int j = 0; j < rcost.size(); j++)
            v += element[i].getDemand(j) * rcost[j];
        vv = ConstantV - v;                             
        element[i].setCF(vv);
        if (vv > 0.0) count++;
    }
    return count;
}

int MainFrame::Early_Start(int act, int trigger, int s_index, int frg)
{
    int max = 0; 
    if (trigger == 0)
    {
        for (int i = 0; i < element[act].getNumOfPred(); i++)
            if (element[element[act].getPredecessor(i)].getFinish() > max)
                max = element[element[act].getPredecessor(i)].getFinish();
    }
    else if (trigger == 1)
    {
        for (int i = 0; i < element[act].getNumOfPred(); i++)
            if (forager_bees[s_index][frg]->getStart(element[act].getPredecessor(i)) + element[element[act].getPredecessor(i)].getDuration() > max)
                max = forager_bees[s_index][frg]->getStart(element[act].getPredecessor(i)) + element[element[act].getPredecessor(i)].getDuration();
    }
    return max;
}

int MainFrame::Latest_Finish(int act)
{
    int max = 0;
    for (int i = 0; i < element[act].getNumOfSucc(); i++)
        if (element[element[act].getSuccessor(i)].getFinish() > max)
            max = element[element[act].getSuccessor(i)].getFinish();
    return max;
}

vector<int> MainFrame::Forward_Activity_Check(vector<int> next_act, int* act, vector<double> random, int trigger, int s_index, int frg)
{
    double max = 0.0;
    vector<int> temp;
    if (trigger == 0)
    {
        for (size_t i = 0; i < next_act.size(); i++)
        {
            int count = 0;
            for (int j = 0; j < element[next_act[i]].getNumOfPred(); j++)  
            {
                if (element[element[next_act[i]].getPredecessor(j)].getStatus() == true)           // check if all the preds are checked
                    count++;
            }
            if ((count == element[next_act[i]].getNumOfPred()) && (random[next_act[i]] > max))
            {
                max = random[next_act[i]];                           // next_act[i] - 1 = 9 enw random[8]
                *act = next_act[i];
            }
        }
        element[*act].setStatus(true);                 //vale true to activity poy mpainei sthn lush

        for (size_t i = 0; i < next_act.size(); i++)           //ananewse to vector
        {
            if ((element[next_act[i]].getStatus() == false) && (find(temp.begin(), temp.end(), next_act[i]) == temp.end()))
                temp.push_back(next_act[i]);

            else if (element[next_act[i]].getStatus() == true)
                for (int j = 0; j < element[next_act[i]].getNumOfSucc(); j++)
                {
                    if (find(temp.begin(), temp.end(), element[next_act[i]].getSuccessor(j)) == temp.end())
                        temp.push_back(element[next_act[i]].getSuccessor(j));
                }
        }
    }
    else if (trigger == 1)
    {
        for (size_t i = 0; i < next_act.size(); i++)
        {
            int count = 0;
            for (int j = 0; j < element[next_act[i]].getNumOfPred(); j++)
            {
                if (forager_bees[s_index][frg]->getStatus(element[next_act[i]].getPredecessor(j)) == true)           // check if all the preds are checked
                    count++;
            }
            if ((count == element[next_act[i]].getNumOfPred()) && (random[next_act[i]] > max))
            {
                max = random[next_act[i]];                           // next_act[i] - 1 = 9 enw random[8]
                *act = next_act[i];
            }
        }
        forager_bees[s_index][frg]->setStatus(*act, true);         //vale true to activity poy mpainei sthn lush
        for (size_t i = 0; i < next_act.size(); i++)           //ananewse to vector
        {
            if ((forager_bees[s_index][frg]->getStatus(next_act[i]) == false) && (find(temp.begin(), temp.end(), next_act[i]) == temp.end()))
                temp.push_back(next_act[i]);
            else if (forager_bees[s_index][frg]->getStatus(next_act[i]) == true)
                for (int j = 0; j < element[next_act[i]].getNumOfSucc(); j++)
                {
                    if (find(temp.begin(), temp.end(), element[next_act[i]].getSuccessor(j)) == temp.end())
                        temp.push_back(element[next_act[i]].getSuccessor(j));
                }
        }
    }
    return temp;
}

vector<int> MainFrame:: Forward_Activity_List(vector<double> random, int trigger, int s_index, int frg)                      // function gia na vrei to activity list gia forward
{
    vector<int> solution; solution.clear(); solution.push_back(0);
    vector<int> next_act;   // candidate activities
    int current = 0;        // prwta mpainei h 0 profanws
    int next;
    int act;
    int check = 1;

    if (trigger == 0)
    {
        double max = 0.0;
        int suc = 0;
        for (int i = 0; i < element[current].getNumOfSucc(); i++)
            if (random[element[current].getSuccessor(i)] > max)
            {
                max = random[element[current].getSuccessor(i)];
                suc = element[current].getSuccessor(i);
            }
        next = suc;
        element[next].setStatus(true);      // set true thn epomenh apo thn 0

        for (int i = 0; i < element[current].getNumOfSucc(); i++)
            if (element[element[current].getSuccessor(i)].getStatus() == false)
                next_act.push_back(element[current].getSuccessor(i));                      // an oi upoloipes apo thn 0 (current) == false, valtes sto vector

        for (int i = 0; i < element[next].getNumOfSucc(); i++)
            next_act.push_back(element[next].getSuccessor(i));            // vale kai tis epomenes ths next

        solution.push_back(next); check++;                          // push_back next kai check == 2 epeidh mpainei h start + 1 act
        while (check < inputs)
        {
            next_act = Forward_Activity_Check(next_act, &act, random, trigger, s_index, frg);
            solution.push_back(act);
            check++;
        }
    }
    else if (trigger == 1)
    {
        double max = 0.0;
        int suc = 0;
        for (int i = 0; i < element[current].getNumOfSucc(); i++)
            if (random[element[current].getSuccessor(i)] > max)
            {
                max = random[element[current].getSuccessor(i)];
                suc = element[current].getSuccessor(i);
            }
        next = suc;
        forager_bees[s_index][frg]->setStatus(next, true);

        for (int i = 0; i < element[current].getNumOfSucc(); i++)                  // current = 0
            if (forager_bees[s_index][frg]->getStatus(element[current].getSuccessor(i)) == false)
                next_act.push_back(element[current].getSuccessor(i));                      // an oi upoloipes apo thn 0 (current) == false, valtes sto vector

        for (int i = 0; i < element[next].getNumOfSucc(); i++)
            next_act.push_back(element[next].getSuccessor(i));            // vale kai tis epomenes ths next

        solution.push_back(next); check++;                          // push_back next kai check == 2 epeidh mpainei h start + 1 act
        while (check < inputs)
        {
            next_act = Forward_Activity_Check(next_act, &act, random, trigger, s_index, frg);
            solution.push_back(act);
            check++;
        }
    }
    return solution;
}

vector<vector<int>> MainFrame::Forward_Resource_Check(vector<vector<int>> GanntR, vector<int> solution, double* npv, int* finish, int trigger, int s_index, int frg)
{
    int counter = 1;
    int count = 0;
    int act = 1; 
    double sum = 0.0;
    while (counter < solution.size())
    {
        int ES = Early_Start(solution[act], trigger, s_index, frg); int dur = element[solution[act]].getDuration();
        int i = ES;
        while (i < ES + dur)
        {
            for (int j = 0; j < resourceNum; j++)
            {
                if (GanntR[i][j] + element[solution[act]].getDemand(j) > resourceCap[j])
                {
                    ES++; i = ES - 1; count = 0;
                    break;
                }
                else count++;
            }
            if (count / resourceNum == dur)
            {
                for (int k = ES; k < i + 1; k++)
                    for (int j = 0; j < resourceNum; j++)
                        GanntR[k][j] += element[solution[act]].getDemand(j);
            }
            i++;
        }
        if (trigger == 0) element[solution[act]].setStart(ES);
        else if (trigger == 1)
        {
            forager_bees[s_index][frg]->setStart(solution[act], ES);
            forager_bees[s_index][frg]->setFinish(solution[act], element[solution[act]].getDuration());
            double val = element[solution[act]].getCashFlow() / (pow(1.0 + rate, forager_bees[s_index][frg]->getFinish(solution[act])));
            forager_bees[s_index][frg]->setNPV(solution[act], val);
            sum += val;
        }
        act++;
        count = 0;
        counter++;
    }
    if (trigger == 0) { *finish = element[inputs - 1].getFinish(); }
    else if (trigger == 1)
    {
        *finish = forager_bees[s_index][frg]->getStart(inputs - 1);
        *npv = sum;
    }
    return GanntR;
}

void MainFrame::SSGS_Forward(vector<double> random, int s_index, int frg, int trigger)                 // forward schedule
{
    vector<int> Solution; Solution.clear();
    vector<vector<int>> GanntR; GanntR.clear();
    vector<int> vec;
    int finish;
    double npv;

    for (int i = 0; i < resourceNum; i++) vec.push_back(0);
    for (int i = 0; i < MaxTime; i++) GanntR.push_back(vec);

    for (int i = 0; i < MaxTime; i++)
        for (int j = 0; j < resourceNum; j++)
            GanntR[i][j] = 0;
    if (trigger == 0)
    {
        element[0].setStatus(true);
        for (int i = 1; i < inputs; i++)
            element[i].setStatus(false);
    }
    Solution = Forward_Activity_List(random, trigger, s_index, frg);                          // Activity list
    GanntR = Forward_Resource_Check(GanntR, Solution, &npv, &finish, trigger, s_index, frg);              // Resource check
    GanntR.resize(finish);

    if (trigger == 0) scout_bees[s_index] = In_Pop(Solution, 0.0, finish, random);
    else if (trigger == 1) *(forager_bees[s_index][frg]) = Foragers(Solution, finish, GanntR, forager_bees[s_index][frg]->getPrio(),
        npv, forager_bees[s_index][frg]->getNPVVec(), forager_bees[s_index][frg]->getStatusVec(), forager_bees[s_index][frg]->getStartVec(),
        forager_bees[s_index][frg]->getFinishVec(), forager_bees[s_index][frg]->getCheckVec());
}


void MainFrame::Local_Search(int local_size, int scout, vector<bool> dummy_status, vector<int> dummy_start, vector<int> dummy_finish, vector<double> dummy_npv, vector<bool> dummy_check) 
{
    vector<vector<int>> gannt; gannt.clear();
    vector<int> sol;
    sol.clear();
    sol = scout_bees[scout].GetSolution();
    vector<double> prob;
    prob.clear();
    prob = scout_bees[scout].GetPrio();
    double best_npv = scout_bees[scout].GetNPV();
    int best_dur = scout_bees[scout].GetSolutionDuration();
    int index = 0;
    int flag = 0;

    omp_set_dynamic(0);
    omp_set_num_threads(3);    
#pragma omp parallel
    {
#pragma omp for schedule(static)
        for (int i = 0; i < local_size; i++)
        {
            if (i == 0)
            {
                *(forager_bees[scout][i]) = Foragers(sol, 0, gannt, prob, 0.0, dummy_npv, dummy_status, dummy_start, dummy_finish, dummy_check);
                SSGS_Forward(prob, scout, i, 1);
            }
            else if (i <= (local_size / 3))
            {
                vector<double> prob1; prob1.clear();
                prob1 = scout_bees[scout].GetPrio();
                vector<int> sol1 = scout_bees[scout].GetSolution();
                int* temp1 = (int*)calloc(inputs, sizeof(int));
                for (int j = 0; j < int(prob1.size() * 0.48); j++)      // 0.48 max sta 30, 15 act | 0.48 max sta 60, 30 act | 0.48 max sta 120, 60 act
                {
                    int a1;
                    double aa1;
                    do
                    {
                        a1 = rand() % (prob1.size() / 2 - 1) + 1;
                        aa1 = distr(eng);
                        temp1[sol[a1]]++;
                    } while (temp1[sol[a1]] > 1);
                    prob1[sol[a1]] = aa1;
                }
                free(temp1);
                sol1.clear();
                *(forager_bees[scout][i]) = Foragers(sol1, 0, gannt, prob1, 0.0, dummy_npv, dummy_status, dummy_start, dummy_finish, dummy_check);
                SSGS_Forward(prob1, scout, i, 1);
            }
            else if (i <= (local_size / 3) * 2 - 1)
            {
                int* temp2 = (int*)calloc(inputs, sizeof(int));
                vector<double> prob2 = scout_bees[scout].GetPrio();
                vector<int> sol2 = scout_bees[scout].GetSolution();
                for (int k = 0; k < int(prob2.size() * 0.48); k++)
                {
                    int a2;
                    double aa2;
                    do
                    {
                        a2 = rand() % int((prob2.size() / 2)) + int(prob2.size()) / 2;
                        aa2 = distr(eng);
                        temp2[sol[a2]]++;
                    } while (temp2[sol[a2]] > 1);
                    prob2[sol[a2]] = aa2;
                }
                free(temp2);
                sol2.clear();
                *(forager_bees[scout][i]) = Foragers(sol2, 0, gannt, prob2, 0.0, dummy_npv, dummy_status, dummy_start, dummy_finish, dummy_check);
                SSGS_Forward(prob2, scout, i, 1);
            }
            else if (i > (local_size / 3) * 2 - 1)
            {
                vector<double> prob3 = scout_bees[scout].GetPrio();
                vector<int> sol3 = scout_bees[scout].GetSolution();
                int* temp3 = (int*)calloc(inputs, sizeof(int));
                for (int jj = 0; jj < int(prob3.size() * 0.96); jj++)
                {
                    int a3;
                    double aa3;
                    do
                    {
                        a3 = rand() % (prob3.size() - 2) + 1;
                        aa3 = distr(eng);
                        temp3[sol[a3]]++;
                    } while (temp3[sol[a3]] > 1);
                    prob3[sol[a3]] = aa3;
                }
                free(temp3);
                sol3.clear();
                *(forager_bees[scout][i]) = Foragers(sol3, 0, gannt, prob3, 0.0, dummy_npv, dummy_status, dummy_start, dummy_finish, dummy_check);
                SSGS_Forward(prob3, scout, i, 1);
            }
            if ((forager_bees[scout][i]->getTotalNPV() - best_npv > 0.0)  && (forager_bees[scout][i]->getDuration() <= best_dur))
            {
                best_npv = forager_bees[scout][i]->getTotalNPV();
                index = i;
                flag = 1;
            }
            else if (flag == 0)
            {
                index = 0;
            }
        }
    }
    if (flag == 1)
    {
        f_counter++;
        scout_bees[scout] = In_Pop(forager_bees[scout][index]->getSolVec(), forager_bees[scout][index]->getTotalNPV(),
            forager_bees[scout][index]->getDuration(), forager_bees[scout][index]->getPrio());
        Local_Search(local_size, scout, dummy_status, dummy_start, dummy_finish, dummy_npv, dummy_check);
    }
    else if (flag == 0)
    {
        best_bees[scout] = Best(forager_bees[scout][index]->getSolVec(), forager_bees[scout][index]->getDuration(), forager_bees[scout][index]->getGannt(),
            forager_bees[scout][index]->getTotalNPV(), forager_bees[scout][index]->getNPVVec(), forager_bees[scout][index]->getStartVec(), forager_bees[scout][index]->getFinishVec(),
            forager_bees[scout][index]->getCheckVec());
    }
}

vector<int> MainFrame::delay(vector<int> set, vector<int> set2, int bestbees)
{
    vector<int> delta;
    for (size_t k = 0; k < set2.size(); k++)            // gia kathe act tou set2 
    {
        for (size_t i = 0; i < set.size(); i++)
        {
            for (int j = 0; j < element[set[i]].getNumOfSucc(); j++)
            {
                if (set2[k] == element[set[i]].getSuccessor(j))   // an h act tou set2 EINAI successor kapoias activity tou set1 tote
                {
                    delta.push_back(elite_bees[bestbees].getStart(set2[k]) - elite_bees[bestbees].getFinish(set[i]));       // to delay einai act_start set2 - act_finish set1
                }
            }
        }
    }
    sort(delta.begin(), delta.end());    // sort gia na exw to minimum prwto, delta[0]
    return delta;
}

bool MainFrame::Push_Delay(int bestbee, int delay, vector<int> set1, int deadline)
{
    bool change = false;                                 // deikths opou true an ginei kathusterhsh tou set1
    vector<int> vir_start; vir_start.clear();            // act 29 -> 51
    vector<int> vir_finish; vir_finish.clear();          // act 29 -> 53 praktika 52 sto gannt einai to finish - 1 ****
    int dur = 0;                                         // total duration olou tou set1   
    for (size_t i = 0; i < set1.size(); i++)
    {
        vir_start.push_back(elite_bees[bestbee].getStart(set1[i]));      // pairnw to start, finish kai duration olwn twn act tou set1       
        vir_finish.push_back(elite_bees[bestbee].getFinish(set1[i]));
        dur += element[set1[i]].getDuration();
    }
    bool ok = false;
    int count = 0;
    int rcount = 0;
    int flag = 0;
    int flag1 = 0;
    vector<vector<int>> dummy_gannt = elite_bees[bestbee].getGannt();    // Gannt[i][j], i = xronos, j = poroi | dummy_gannt epeidh den allazw kateutheian sto original gannt ths lushs
    for (size_t k = 0; k < set1.size(); k++)
    {
        for (int i = vir_start[k]; i < vir_finish[k]; i++)   // **** to thelw edw gia na paw sth swsth thesi kai na mhn exw overflow sto vector
        {
            for (int j = 0; j < resourceNum; j++)            // gia kathe act tou set1 AFAIRW OLOUS (resourceNum) porous apo start -> finish
            {
                dummy_gannt[i][j] = dummy_gannt[i][j] - element[set1[k]].getDemand(j);
            }
        }
    }
    vector<vector<int>> backup = dummy_gannt;               // orizw ena backup gannt = me to dummy_gannt me tous porous na exoun afairethei twn act tou set1
    while (true)
    {
        flag = 0;                 // deikths gia break
        flag1 = 0;                 // deikths gia break
        count = 0;
        rcount = 0;               // metrhths upervashs oriou porwn
        vector<int> virt_NEW_start; virt_NEW_start.clear();                 // orizw ta NEW start, finish, dhladh start + delay kai finish + delay **
        vector<int> virt_NEW_finish;  virt_NEW_finish.clear();
        for (size_t k = 0; k < set1.size(); k++)
        {
            virt_NEW_start.push_back(elite_bees[bestbee].getStart(set1[k]) + delay);     // **
            virt_NEW_finish.push_back(elite_bees[bestbee].getFinish(set1[k]) + delay);   // **
            if (virt_NEW_finish[k] > deadline)
            {
                flag1 = 1;
            }
        }
        if (flag1 == 1)
        {
            break;
        }
        for (size_t k = 0; k < set1.size(); k++)
        {
            for (int i = virt_NEW_start[k]; i < virt_NEW_finish[k]; i++)  // gia kathe act tou set1 apo NEW start -> NEW finish elegxw an uparxei upervash oriou porwn 
            {
                for (int j = 0; j < resourceNum; j++)
                {
                    if (dummy_gannt[i][j] + element[set1[k]].getDemand(j) > resourceCap[j])
                    {
                        rcount++;               // an uparxei upervash, rcount++
                    }
                }
                if (rcount > 0)                 // an uparxei upervash ESTW kai mias meras, enos porou                                    
                {
                    delay--;                    // h kathusterhsh meiwnetai kata 1
                    dummy_gannt = backup;       // to dummy ksanaginetaito backup (auto einai se periptwsh pou exoun hdh mpei merikes meres alla exoume upervash argotera)
                    flag = 1;                   // flag = 1 gia to break ths for (size_t k = 0; k < set1.size(); k++)  ****
                    break;                      // break for (int i = virt_NEW_start[k]; i < virt_NEW_finish[k]; i++)
                }
                else
                {
                    for (int j = 0; j < resourceNum; j++)     // edw shmainei pws rcount == 0, ara xwrane oloi oi poroi gia mia mera 
                    {
                        {
                            count++;                          // count++ gia kathe poro pou mphke     
                            dummy_gannt[i][j] = dummy_gannt[i][j] + element[set1[k]].getDemand(j);     // mpainoun oi poroi sto gannt                            
                            if (count == resourceNum * dur)   // an to count == plithos porws * thn sunonlikh diarkeia olws twn act tou set1, shmainei pws mphkan oloi oi poroi gia oles tis act  
                            {
                                ok = true;                    // oles oi act tou set1 kathusterhsan
                            }
                        }
                    }
                }
            }
            if (flag == 1) break;     // ****
        }
        if ((delay <= 0) || (ok == true))     // an to delay == 0, ara den tha kathusterhsh kamia h an kathusterhsan oles
        {
            break;           // break thn while
        }
    }
    if (ok == true)         // an kathusterhsan oles
    {

        for (size_t i = 0; i < set1.size(); i++)
        {
            elite_bees[bestbee].setNEWStart(set1[i], delay);                                                                     // orizw to NEW start 
            elite_bees[bestbee].setFinish(set1[i], element[set1[i]].getDuration());                                         // orizw to NEW finish
            double val = element[set1[i]].getCashFlow() / (pow(1.0 + rate, elite_bees[bestbee].getFinish(set1[i])));        // upologizw NEW NPV
            elite_bees[bestbee].setNPV(set1[i], val);                                                                            // orizw tp NEW NPV
        }
        elite_bees[bestbee].setGannt(dummy_gannt);        // to gannt ths lushs einai pleon to dummy_gannt, opou tha lhfthei upopsin sthn epomenh epanalipsi                                           
        change = true;              // egine kathusterhsh (allagh)
    }
    return change;
}

void MainFrame::Find_Network_Set(int current, int start, int pt1[], int pt2[], double* total_npv, int bestbees, bool status, int count1, int count2)
{
    int fi = elite_bees[bestbees].getFinish(current);     // finish current
    int fk = elite_bees[bestbees].getFinish(start);       // finish start/
    best_bees[bestbees].setCheck(start, true);
    if (status == true)
    {
        for (int i = 0; i < element[current].getNumOfPred(); i++)
        {
            // if ((an den exei prostethei hdh) && (DEN einai to dummy start, dhladh h 0) && (to finish tou pred einai >= tou finish tou start activity) && (NPV < 0.0) )
            if ((best_bees[bestbees].getCheck(element[current].getPredecessor(i)) == false) && (element[current].getPredecessor(i) != start)
                && (elite_bees[bestbees].getFinish(element[current].getPredecessor(i)) >= fk) && (elite_bees[bestbees].getNPV(element[current].getPredecessor(i)) < 0.0))
            {
                pt1[count1] = element[current].getPredecessor(i);              // idia diadikasia gia tous successors, ananewnoume to pt1 akoma
                count1++;
                *total_npv += elite_bees[bestbees].getNPV(element[current].getPredecessor(i));
                best_bees[bestbees].setCheck(element[current].getPredecessor(i), true);
            }
        }
    }
    for (int i = 0; i < element[current].getNumOfSucc(); i++) // pairnw tous successors ths current
    {
        // if ((to START tou successor ths current einai == me to finish ths current) && (DEN einai h dummy finish, dhladh h inputs - 1) && (den exei hdh mpei h elegexthei)) 
        if ((elite_bees[bestbees].getStart(element[current].getSuccessor(i)) == fi) && (element[current].getSuccessor(i) != inputs - 1) && (best_bees[bestbees].getCheck(element[current].getSuccessor(i)) == false))
        {
            pt1[count1] = element[current].getSuccessor(i);                    // mpainei ston static pinaka pt1
            count1++;                                                               // etoimazei thn epomenh prosthikh sthn epomenh thesi tou pt1
            *total_npv += elite_bees[bestbees].getNPV(element[current].getSuccessor(i)); // prostheti sto total_npv, thn npv tou successor
            best_bees[bestbees].setCheck(element[current].getSuccessor(i), true);       // thetei ton successor CHECKED gia na mhn ksanampei
            Find_Network_Set(element[current].getSuccessor(i), start, pt1, pt2, total_npv, bestbees, true, count1, count2);     // kalh thn Find_Set() ksana, alla me Current ton successor kai thetei to status == true
                                                                                                                        // gia elegxo twn predecessors
        }
        else if (best_bees[bestbees].getCheck(element[current].getSuccessor(i)) == false)   // gia osous successors den mphkane sto set1 (tha exoun false to check epeidh den plhroun tis upoloipes proupotheseis
        {
            pt2[count2] = element[current].getSuccessor(i);                        // add to set2 wste na vrethei to allowable delay apo ton earliest successor
            count2++;
            best_bees[bestbees].setCheck(element[current].getSuccessor(i), true);       // check successor
        }
    }
}

void MainFrame::Network_Delay(int deadline, int bestbee)
{
    elite_bees[bestbee] = Elite(best_bees[bestbee].getSolVec(), deadline, best_bees[bestbee].getGannt(),
        best_bees[bestbee].getTotalNPV(), best_bees[bestbee].getNPVVec(), best_bees[bestbee].getStartVec(), best_bees[bestbee].getFinishVec());
    elite_bees[bestbee].setStart(inputs - 1, deadline);
    elite_bees[bestbee].setNEWFinish(inputs - 1, deadline);       // arxika h orizw deadline kai paw to finish kai to start ths 31 sto deadline
    elite_bees[bestbee].resizeGannt(deadline, resourceNum);       // to gannt megawlwnei 
    best_bees[bestbee].setCheck(0, true);
    int del = 0;
    bool change = false;           // bool poy tha epistrafei apo thn Push_Delay()
    int change_count = 1;         // deikths opou an einai > 0 shmainei pws toulaxiston ena set activities exei kathusterhsei kai kaleitai ksana o algorithmos
    while (change_count > 0)
    {
        change_count = 0;
        for (int j = inputs - 1; j > 0; j--)            // start from last activity on Priority List except dummy finish activity (inputs - 1)
        {
            vector<int> set1; set1.clear();             // candidate activites for delay set
            vector<int> set2; set2.clear();             // successors of set1 activites NOT in set1 + predecessors w/ finish >= set1[0]          
            double total = 0.0;
            vector<int> delta; delta.clear();           // allowable delays
            int pos = elite_bees[bestbee].getSol(j);
            if (elite_bees[bestbee].getNPV(pos) < 0.0)       // if act on Priority List has negative NPV
            {
                int* pt1 = NULL;
                int* pt2 = NULL;
                pt1 = (int*)calloc(inputs, sizeof(int));   // static array epeidh h Find_Set() kalei ton eauto ths, theloume na epistrepsoyme 2 pinakes kai me ta vectors den leitourgei swsta
                pt2 = (int*)calloc(inputs, sizeof(int));
                int count1 = 0;                                 // thesi gia tous pt1 kai pt2 antistoixa
                int count2 = 0;

                set1.push_back(pos);                            // to delay tha ginei eite se mia activity (pos, set1.size() == 1), eite se ena activities set (pos + successors me start == finish ths pos, set1.size() > 1)
                total = best_bees[bestbee].getNPV(pos);         // sunolikh npv toy set1
                Find_Network_Set(pos, pos, pt1, pt2, &total, bestbee, false, count1, count2);   // klhsh Find_set(), status == false (7th parameter) gia na MHN parw ta predecessors ths pos activity
                for (int i = 0; i < inputs; i++)
                {
                    if (pt1[i] > 0) set1.push_back(pt1[i]);
                    if (pt2[i] > 0) set2.push_back(pt2[i]);
                }

                for (int k = 0; k < inputs; k++)
                {
                    best_bees[bestbee].setCheck(k, false);     // check false gia oles tis activities
                }
                free(pt1);
                free(pt2);
                if (total < 0.0)                  // an to total_npv tote mono psaxnw na vrw delay, diaforetika to set twn activities exei positive sunolo NPV kai den xreiazetai delay
                {
                    sort(set1.begin(), set1.end());
                    set1.erase(unique(set1.begin(), set1.end()), set1.end());
                    sort(set2.begin(), set2.end());
                    set2.erase(unique(set2.begin(), set2.end()), set2.end());
                    delta = delay(set1, set2, bestbee);         // find allowable delays
                    if (delta.size() > 0)
                    {
                        for (size_t i = 0; i < delta.size(); i++)
                        {
                            if (delta[i] > 0)
                            {
                                del = delta[i];
                                break;
                            }
                        }
                        if (del > 0)
                        {
                            change = Push_Delay(bestbee, del, set1, deadline);   // arxizw to delay
                            if (change == true) change_count++;      // an exei ginei kathusterhsh, tote change == true kai h while ksekinaei ksana
                        }
                    }
                }
            }
        }
    }
}

void MainFrame::Find_Schedule_Set(int current, int start, int pt1[], double* total_npv, int bestbees, bool status, int count1)
{
    int fi = elite_bees[bestbees].getFinish(current);     // finish current
    int si = elite_bees[bestbees].getStart(current);     // finish current
    int fk = elite_bees[bestbees].getFinish(start);       // finish start/
    best_bees[bestbees].setCheck(start, true);
    if (status == true)
    {
        for (int j = 0; j < elite_bees[bestbees].getSolSize(); j++)        // oles tis activities
        {
            if ((si == elite_bees[bestbees].getFinish(elite_bees[bestbees].getSol(j))) && (best_bees[bestbees].getCheck(elite_bees[bestbees].getSol(j)) == false) && (elite_bees[bestbees].getSol(j) != start)
                && (elite_bees[bestbees].getFinish(elite_bees[bestbees].getSol(j)) >= fk) && (elite_bees[bestbees].getNPV(elite_bees[bestbees].getSol(j)) < 0.0))
            {
                pt1[count1] = elite_bees[bestbees].getSol(j);
                count1++;
                *total_npv += elite_bees[bestbees].getNPV(elite_bees[bestbees].getSol(j));
                best_bees[bestbees].setCheck(elite_bees[bestbees].getSol(j), true);
            }
        }
    }
    for (int j = 0; j < elite_bees[bestbees].getSolSize(); j++)        // oles tis activities
    {
        if ((fi == elite_bees[bestbees].getStart(elite_bees[bestbees].getSol(j))) && (elite_bees[bestbees].getSol(j) != inputs - 1) && (best_bees[bestbees].getCheck(elite_bees[bestbees].getSol(j)) == false))
        {
            pt1[count1] = elite_bees[bestbees].getSol(j);
            count1++;
            *total_npv += elite_bees[bestbees].getNPV(elite_bees[bestbees].getSol(j));
            best_bees[bestbees].setCheck(elite_bees[bestbees].getSol(j), true);
            Find_Schedule_Set(elite_bees[bestbees].getSol(j), start, pt1, total_npv, bestbees, true, count1);
        }
    }
}

vector<int> MainFrame::Schedule_Set_2(vector<int> set1, int bestbees)
{
    vector<int> set2; set2.clear();
    for (size_t i = 0; i < set1.size(); i++)
    {
        for (int k = 0; k < element[set1[i]].getNumOfSucc(); k++)
        {
            if (best_bees[bestbees].getCheck(element[set1[i]].getSuccessor(k)) == false)
                set2.push_back(element[set1[i]].getSuccessor(k));
        }
    }
    sort(set2.begin(), set2.end());
    return set2;
}

void MainFrame::Schedule_Delay(int bestbee, int deadline)
{
    best_bees[bestbee].setCheck(0, true);
    int del = 0;
    bool change = false;           // bool poy tha epistrafei apo thn Push_Delay()
    int change_count = 1;         // deikths opou an einai > 0 shmainei pws toulaxiston ena set activities exei kathusterhsei kai kaleitai ksana o algorithmos
    while (change_count > 0)
    {
        change_count = 0;
        for (int j = inputs - 1; j > 0; j--)            // start from last activity on Priority List except dummy finish activity (inputs - 1)
        {
            vector<int> set1; set1.clear();             // candidate activites for delay set
            vector<int> set2; set2.clear();             // successors of set1 activites NOT in set1 + predecessors w/ finish >= set1[0]          
            double total = 0.0;
            vector<int> delta; delta.clear();           // allowable delays
            int pos = elite_bees[bestbee].getSol(j);
            if (elite_bees[bestbee].getNPV(pos) < 0.0)       // if act on Priority List has negative NPV
            {
                int* pt1 = (int*)calloc(inputs, sizeof(int));   // static array epeidh h Find_Set() kalei ton eauto ths, theloume na epistrepsoyme 2 pinakes kai me ta vectors den leitourgei swsta
                int count1 = 0;                                 // thesi gia tous pt1 kai pt2 antistoixa

                set1.push_back(pos);                            // to delay tha ginei eite se mia activity (pos, set1.size() == 1), eite se ena activities set (pos + successors me start == finish ths pos, set1.size() > 1)
                total = best_bees[bestbee].getNPV(pos);         // sunolikh npv toy set1
                Find_Schedule_Set(pos, pos, pt1, &total, bestbee, false, count1);   // klhsh Find_set(), status == false (7th parameter) gia na MHN parw ta predecessors ths pos activity
                for (int i = 0; i < inputs; i++)
                    if (pt1[i] > 0) set1.push_back(pt1[i]);

                set2 = Schedule_Set_2(set1, bestbee);
                for (int k = 0; k < inputs; k++)
                {
                    best_bees[bestbee].setCheck(k, false);     // check false gia oles tis activities
                }
                free(pt1);
                if (total < 0.0)                  // an to total_npv tote mono psaxnw na vrw delay, diaforetika to set twn activities exei positive sunolo NPV kai den xreiazetai delay
                {
                    sort(set1.begin(), set1.end());
                    set1.erase(unique(set1.begin(), set1.end()), set1.end());
                    sort(set2.begin(), set2.end());
                    set2.erase(unique(set2.begin(), set2.end()), set2.end());
                    delta = delay(set1, set2, bestbee);         // find allowable delays
                    if (delta.size() > 0)
                    {
                        for (size_t i = 0; i < delta.size(); i++)
                        {
                            if (delta[i] > 0)
                            {
                                del = delta[i];
                                break;
                            }
                        }
                        if (del > 0)
                        {
                            change = Push_Delay(bestbee, del, set1, deadline);   // arxizw to delay
                            if (change == true) {
                                change_count++;      // an exei ginei kathusterhsh, tote change == true kai h while ksekinaei ksana
                            }
                        }
                    }
                }
            }
        }
    }
}

void MainFrame::Check_Schedule(int bees)
{
    for (int i = 0; i < bees; i++)
    {
        size_t pre_size = element[inputs - 1].getNumOfPred();
        int pc = 0;
        for (int j = 0; j < pre_size; j++)
        {
            if (elite_bees[i].getFinish(element[inputs - 1].getPredecessor(j)) != elite_bees[i].getDuration()) {
                pc++;
            }
        }
        int max = 0;
        if (pc == pre_size)
        {
            for (int j = 0; j < pre_size; j++)
            {
                if (elite_bees[i].getStart(element[inputs - 1].getPredecessor(j)) >= max)
                {
                    max = elite_bees[i].getFinish(element[inputs - 1].getPredecessor(j));
                }
            }
            elite_bees[i].setStart(elite_bees[i].getSol(inputs - 1), max);
            elite_bees[i].setNEWFinish(elite_bees[i].getSol(inputs - 1), max);
            elite_bees[i].setDurationNEW(elite_bees[i].getFinish(elite_bees[i].getSol(inputs - 1)));
            vector<vector<int>> gg;
            gg = elite_bees[i].getGannt();
            gg.resize(elite_bees[i].getDuration());
            elite_bees[i].setGannt(gg);
        }

        int suc_size = element[0].getNumOfSucc();
        int sc = 0;
        for (int j = 0; j < suc_size; j++)
        {
            if (elite_bees[i].getStart(element[0].getSuccessor(j)) != 0) { sc++; }
        }
        int min = 500;
        if (sc == suc_size)
        {
            for (int j = 0; j < suc_size; j++)
            {
                if (elite_bees[i].getStart(element[0].getSuccessor(j)) <= min)
                {
                    min = elite_bees[i].getStart(element[0].getSuccessor(j));
                }
            }
            int old_start = min;
            int old_finish = elite_bees[i].getDuration();
            for (int k = 1; k < inputs; k++)
            {
                elite_bees[i].setStart(elite_bees[i].getSol(k), elite_bees[i].getStart(elite_bees[i].getSol(k)) - min);
                elite_bees[i].setFinish(elite_bees[i].getSol(k), element[elite_bees[i].getSol(k)].getDuration());
                double val = element[elite_bees[i].getSol(k)].getCashFlow() / (pow(1.0 + rate, elite_bees[i].getFinish(elite_bees[i].getSol(k))));        // upologizw NEW NPV
                elite_bees[i].setNPV(elite_bees[i].getSol(k), val);
            }
            elite_bees[i].setDurationNEW(elite_bees[i].getFinish(elite_bees[i].getSol(inputs - 1)));
            double tt = 0.0;
            for (int k = 0; k < inputs; k++)
            {
                tt += elite_bees[i].getNPV(elite_bees[i].getSol(k));
            }
            elite_bees[i].setTotalNPV(tt);
            vector<vector<int>> dg(elite_bees[i].getDuration(), vector<int>(resourceNum, 0));
            int k = 0;
            for (int t = old_start; t < old_finish; t++)
            {
                for (int j = 0; j < resourceNum; j++)
                {
                    dg[k][j] = elite_bees[i].PrintGannt(t, j);
                }
                k++;
            }
            elite_bees[i].setGannt(dg);
        }
    }
}

void MainFrame::Initial_Population(int N, vector<double> random, vector<int> solution)
{
    int nn = 0;
    double X = 0.0;
    while (nn < N)
    {
        random.clear();
        random.push_back(1.0);   // pithanotita start
        for (int i = 0; i < inputs - 1; i++)                        // random values gia oles tis activities
        {
            X = distr(eng);
            random.push_back(X);
        }
        random.push_back(0.0);  //pithanotita finish
        TotalNPV = 0.0;
        SSGS_Forward(random, nn, 0, 0);
        for (int i = 0; i < inputs; i++)
        {
            element[i].setNPV(element[i].getCashFlow(), rate, element[i].getFinish());
            TotalNPV += element[i].getNPV();
        }
        scout_bees[nn] = In_Pop(scout_bees[nn].GetSolution(), TotalNPV, scout_bees[nn].GetSolutionDuration(), scout_bees[nn].GetPrio());
        nn++;
        solution.clear();
    }
}