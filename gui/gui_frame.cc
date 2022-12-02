#include "gui_frame.h"

#ifndef _WIN32
#include "icon.xpm"
#endif

GuiFrame::GuiFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                   const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style) {
  wxSize frame_size = FromDIP(wxSize(800, 600));
  this->SetSize(frame_size);
  this->SetMaxSize(frame_size);
  this->SetMinSize(frame_size);
  this->SetTitle(_T("assfonts"));

#ifdef _WIN32
  wxIcon icon = wxICON(icon_resource);
#else
  wxIcon icon = wxIcon(icon_xpm);
#endif
  this->SetIcon(icon);

  main_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                           wxTAB_TRAVERSAL);
  wxBoxSizer* inner_sizer;
  inner_sizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* top_sizer;
  top_sizer = new wxFlexGridSizer(4, 3, 0, 0);
  top_sizer->SetFlexibleDirection(wxBOTH);
  top_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  input_label = new wxStaticText(main_panel, wxID_ANY, _T("Input ASS file"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  input_label->Wrap(-1);
  top_sizer->Add(input_label, 0, wxALIGN_CENTER | wxALL, 5);

  input_text =
      new wxTextCtrl(main_panel, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(600, 50)), wxTE_MULTILINE | wxTE_READONLY);
  top_sizer->Add(input_text, 0, wxALL, 5);

  input_button = new wxButton(main_panel, wxID_ANY, _T("..."),
                              wxDefaultPosition, FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(input_button, 0, wxALIGN_CENTER | wxALL, 5);

  output_label = new wxStaticText(main_panel, wxID_ANY, _T("Output directory"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  output_label->Wrap(-1);
  top_sizer->Add(output_label, 0, wxALIGN_CENTER | wxALL, 5);

  output_text =
      new wxTextCtrl(main_panel, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(600, 50)), wxTE_MULTILINE | wxTE_READONLY);
  top_sizer->Add(output_text, 0, wxALL, 5);

  output_button = new wxButton(main_panel, wxID_ANY, _T("..."),
                               wxDefaultPosition, FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(output_button, 0, wxALIGN_CENTER | wxALL, 5);

  font_label = new wxStaticText(main_panel, wxID_ANY, _T("Font directory"),
                                wxDefaultPosition, wxDefaultSize, 0);
  font_label->Wrap(-1);
  top_sizer->Add(font_label, 0, wxALIGN_CENTER | wxALL, 5);

  font_text =
      new wxTextCtrl(main_panel, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(600, 50)), wxTE_MULTILINE | wxTE_READONLY);
  top_sizer->Add(font_text, 0, wxALL, 5);

  font_button = new wxButton(main_panel, wxID_ANY, _T("..."), wxDefaultPosition,
                             FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(font_button, 0, wxALIGN_CENTER | wxALL, 5);

  db_label = new wxStaticText(main_panel, wxID_ANY, _T("Database directory"),
                              wxDefaultPosition, wxDefaultSize, 0);
  db_label->Wrap(-1);
  top_sizer->Add(db_label, 0, wxALIGN_CENTER | wxALL, 5);

  db_text =
      new wxTextCtrl(main_panel, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(600, 50)), wxTE_MULTILINE | wxTE_READONLY);
  top_sizer->Add(db_text, 0, wxALL, 5);

  db_button = new wxButton(main_panel, wxID_ANY, _T("..."), wxDefaultPosition,
                           FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(db_button, 0, wxALIGN_CENTER | wxALL, 5);

  inner_sizer->Add(top_sizer, 1, wxALIGN_CENTER | wxALL, 5);

  wxBoxSizer* middle_sizer;
  middle_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* checkbox_sizer;
  checkbox_sizer = new wxBoxSizer(wxVERTICAL);

  subset_check = new wxCheckBox(main_panel, wxID_ANY, _T("Subset only"),
                                wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(subset_check, 0, wxALL, 5);

  embed_check = new wxCheckBox(main_panel, wxID_ANY, _T("Embed only"),
                               wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(embed_check, 0, wxALL, 5);

  build_button = new wxButton(main_panel, wxID_ANY, _T("Build database"),
                              wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(build_button, 0, wxALL, 5);

  middle_sizer->Add(checkbox_sizer, 1, wxALIGN_CENTER | wxALL, 5);

  run_button = new wxButton(main_panel, wxID_ANY, _T("Run"), wxDefaultPosition,
                            FromDIP(wxSize(70, 70)), 0);
  middle_sizer->Add(run_button, 0, wxALIGN_CENTER | wxALL, 5);

  inner_sizer->Add(middle_sizer, 1, wxALIGN_CENTER | wxALL, 5);

  log_text =
      new wxTextCtrl(main_panel, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(-1, 230)), wxTE_MULTILINE | wxTE_READONLY);
  inner_sizer->Add(log_text, 0, wxEXPAND, 5);

  main_panel->SetSizer(inner_sizer);
  main_panel->Layout();
  inner_sizer->Fit(main_panel);

  this->Layout();

  this->Centre(wxBOTH);
}

GuiFrame::~GuiFrame() {}
