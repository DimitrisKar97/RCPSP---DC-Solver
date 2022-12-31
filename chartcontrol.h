#pragma once
#include <wx/wx.h>
#include <vector>
#include <string>

class ChartControl : public wxWindow
{
public:
    ChartControl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);

    std::vector<double> values;
    std::string title;
    double max;
    double min;
    int time_value;

private:
    void OnPaint(wxPaintEvent& evt);
    std::tuple<int, double, double> calculateChartSegmentCountAndRange(double origLow, double origHigh);
    std::tuple<int, double, double> calculateChartSegmentCountAndRange2(double origLow, double origHigh);
};

class ChartControlSA : public wxWindow
{
public:
    ChartControlSA(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);

    std::vector<double> rate_values;
    std::vector<double> npv_values;
    std::string title;
    int number = 0;
    double lv;
    double hv;

private:
    void OnPaint(wxPaintEvent& evt);
    std::tuple<int, double, double> calculateChartSegmentCountAndRange(double origLow, double origHigh);
    std::tuple<int, double, double> calculateChartSegmentCountAndRange2(double origLow, double origHigh);
};
