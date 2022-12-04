/*  This file is part of assfonts.
 *
 *  assfonts is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  assfonts is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with assfonts. If not, see <https://www.gnu.org/licenses/>.
 *
 *  written by wyzdwdz (https://github.com/wyzdwdz)
 */

#include "gui_frame.h"

#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#ifndef _WIN32
#include "icon.xpm"
#endif
#include "assfonts_gui.h"
#include "run.h"
#include "wxwidgets_sink.h"

namespace fs = boost::filesystem;

GuiFrame::GuiFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                   const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style) {
  wxFileName f(wxStandardPaths::Get().GetExecutablePath());
  app_path_ = f.GetPath();

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

  main_panel_ = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                            wxTAB_TRAVERSAL);
  wxBoxSizer* inner_sizer;
  inner_sizer = new wxBoxSizer(wxVERTICAL);

  wxFlexGridSizer* top_sizer;
  top_sizer = new wxFlexGridSizer(4, 4, 0, 0);
  top_sizer->SetFlexibleDirection(wxBOTH);
  top_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

  input_label_ = new wxStaticText(main_panel_, wxID_ANY, _T("Input ASS file"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  input_label_->Wrap(-1);
  wxFont font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  input_label_->SetFont(font);
  top_sizer->Add(input_label_, 0, wxALIGN_CENTER | wxALL, 5);

  input_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(550, 50)), wxTE_MULTILINE | wxTE_READONLY);
  input_text_->DragAcceptFiles(true);
  top_sizer->Add(input_text_, 0, wxALL, 5);

  input_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."),
                               wxDefaultPosition, FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(input_button_, 0, wxALIGN_CENTER | wxALL, 5);

  input_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("¨w"), wxDefaultPosition,
                   FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(input_clean_button_, 0, wxALIGN_CENTER | wxALL, 5);

  output_label_ =
      new wxStaticText(main_panel_, wxID_ANY, _T("Output directory"),
                       wxDefaultPosition, wxDefaultSize, 0);
  output_label_->Wrap(-1);
  font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  output_label_->SetFont(font);
  top_sizer->Add(output_label_, 0, wxALIGN_CENTER | wxALL, 5);

  output_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(550, 50)), wxTE_MULTILINE | wxTE_READONLY);
  output_text_->DragAcceptFiles(true);
  top_sizer->Add(output_text_, 0, wxALL, 5);

  output_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."),
                                wxDefaultPosition, FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(output_button_, 0, wxALIGN_CENTER | wxALL, 5);

  output_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("¨w"), wxDefaultPosition,
                   FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(output_clean_button_, 0, wxALIGN_CENTER | wxALL, 5);

  font_label_ = new wxStaticText(main_panel_, wxID_ANY, _T("Font directory"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  font_label_->Wrap(-1);
  font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  font_label_->SetFont(font);
  top_sizer->Add(font_label_, 0, wxALIGN_CENTER | wxALL, 5);

  font_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(550, 50)), wxTE_MULTILINE | wxTE_READONLY);
  font_text_->DragAcceptFiles(true);
  top_sizer->Add(font_text_, 0, wxALL, 5);

  font_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."),
                              wxDefaultPosition, FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(font_button_, 0, wxALIGN_CENTER | wxALL, 5);

  font_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("¨w"), wxDefaultPosition,
                   FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(font_clean_button_, 0, wxALIGN_CENTER | wxALL, 5);

  db_label_ = new wxStaticText(main_panel_, wxID_ANY, _T("Database directory"),
                               wxDefaultPosition, wxDefaultSize, 0);
  db_label_->Wrap(-1);
  font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  db_label_->SetFont(font);
  top_sizer->Add(db_label_, 0, wxALIGN_CENTER | wxALL, 5);

  db_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, app_path_, wxDefaultPosition,
                     FromDIP(wxSize(550, 50)), wxTE_MULTILINE | wxTE_READONLY);
  db_text_->DragAcceptFiles(true);
  top_sizer->Add(db_text_, 0, wxALL, 5);

  db_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."), wxDefaultPosition,
                            FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(db_button_, 0, wxALIGN_CENTER | wxALL, 5);

  db_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("¨w"), wxDefaultPosition,
                   FromDIP(wxSize(30, 30)), 0);
  top_sizer->Add(db_clean_button_, 0, wxALIGN_CENTER | wxALL, 5);

  inner_sizer->Add(top_sizer, 1, wxALIGN_CENTER | wxALL, 5);

  wxBoxSizer* middle_sizer;
  middle_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxBoxSizer* checkbox_sizer;
  checkbox_sizer = new wxBoxSizer(wxVERTICAL);

  subset_check_ = new wxCheckBox(main_panel_, wxID_ANY, _T("Subset only"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(subset_check_, 0, wxBOTTOM, 2);

  embed_check_ = new wxCheckBox(main_panel_, wxID_ANY, _T("Embed only"),
                                wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(embed_check_, 0, wxBOTTOM, 5);

  build_button_ = new wxButton(main_panel_, wxID_ANY, _T("Build database"),
                               wxDefaultPosition, FromDIP(wxSize(100, 40)), 0);
  checkbox_sizer->Add(build_button_, 0, 0, 5);

  middle_sizer->Add(checkbox_sizer, 1, wxALIGN_CENTER | wxALL, 5);

  run_button_ = new wxButton(main_panel_, wxID_ANY, _T("Run"),
                             wxDefaultPosition, FromDIP(wxSize(70, 70)), 0);
  middle_sizer->Add(run_button_, 0, wxALIGN_CENTER | wxALL, 5);

  reset_button_ = new wxButton(main_panel_, wxID_ANY, _T("Reset all"),
                               wxDefaultPosition, FromDIP(wxSize(80, 30)), 0);
  middle_sizer->Add(reset_button_, 0, wxALIGN_CENTER | wxALL, 10);

  inner_sizer->Add(middle_sizer, 1, wxALIGN_CENTER | wxALL, 5);

  log_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(-1, 230)), wxTE_MULTILINE | wxTE_READONLY);
  inner_sizer->Add(log_text_, 0, wxEXPAND, 5);

  main_panel_->SetSizer(inner_sizer);
  main_panel_->Layout();
  inner_sizer->Fit(main_panel_);

  this->Layout();

  this->Centre(wxBOTH);

  input_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnFindInput,
                      this);
  output_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnFindOutput,
                       this);
  font_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnFindFont, this);
  db_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnFindDB, this);

  input_clean_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                            [&](wxCommandEvent&) { input_text_->Clear(); });
  output_clean_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                             [&](wxCommandEvent&) { output_text_->Clear(); });
  font_clean_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                           [&](wxCommandEvent&) { font_text_->Clear(); });
  db_clean_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                         [&](wxCommandEvent&) { db_text_->Clear(); });

  input_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropInput, this);
  output_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropOutput, this);
  font_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropFont, this);
  db_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropDB, this);

  run_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnRun, this);
  build_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnBuild, this);
  reset_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnReset, this);

  spdlog::init_thread_pool(512, 1);
  sink_ = std::make_shared<mylog::sinks::wxwidgets_sink_mt>(log_text_);
  logger_ = std::make_shared<spdlog::logger>("main", sink_);
  logger_->set_pattern("[%^%l%$] %v");
  spdlog::register_logger(logger_);

  logger_->info("assfonts-gui v{}.{}.{}", VERSION_MAX, VERSION_MID,
                VERSION_MIN);

  auto db_path = fs::path(db_text_->GetValue().ToStdString() + "/fonts.db");
  if (fs::is_regular_file(db_path)) {
    logger_->info("Found fonts database: \"{}\"", db_path.generic_string());
  } else {
    logger_->warn("Fonts database not found.");
  }
}

GuiFrame::~GuiFrame() {
  spdlog::shutdown();
}

void GuiFrame::OnFindInput(wxCommandEvent& WXUNUSED(event)) {
  wxFileDialog open_file_dialog(this, _T("Open ASS file"), "", "",
                                _T("ASS file (*.ass)|*.ass"),
                                wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (open_file_dialog.ShowModal() == wxID_CANCEL) {
    return;
  }
  wxString path = open_file_dialog.GetPath();
  wxString dir = open_file_dialog.GetDirectory();
  input_text_->ChangeValue(path);
  output_text_->ChangeValue(dir);
}

void GuiFrame::OnFindOutput(wxCommandEvent& WXUNUSED(event)) {
  wxDirDialog open_dir_dialog(this, _T("Select a directory"));
  if (open_dir_dialog.ShowModal() == wxID_CANCEL) {
    return;
  }
  wxString path = open_dir_dialog.GetPath();
  output_text_->ChangeValue(path);
}

void GuiFrame::OnFindFont(wxCommandEvent& WXUNUSED(event)) {
  wxDirDialog open_dir_dialog(this, _T("Select a directory"));
  if (open_dir_dialog.ShowModal() == wxID_CANCEL) {
    return;
  }
  wxString path = open_dir_dialog.GetPath();
  font_text_->ChangeValue(path);
}

void GuiFrame::OnFindDB(wxCommandEvent& WXUNUSED(event)) {
  wxDirDialog open_dir_dialog(this, _T("Select a directory"));
  if (open_dir_dialog.ShowModal() == wxID_CANCEL) {
    return;
  }
  wxString path = open_dir_dialog.GetPath();
  db_text_->ChangeValue(path);
  auto db_path = fs::path(db_text_->GetValue().ToStdString() + "/fonts.db");
  if (fs::is_regular_file(db_path)) {
    logger_->info("Found fonts database: \"{}\"", db_path.generic_string());
  } else {
    logger_->warn("Fonts database not found.");
  }
}

void GuiFrame::OnDropInput(wxDropFilesEvent& event) {
  wxString* path = event.GetFiles();
  if (!wxFileExists(path[0])) {
    return;
  }
  wxFileName first_filename = wxFileName(path[0]);
  wxString ext = first_filename.GetExt();
  if (!first_filename.GetExt().IsSameAs("ass", false)) {
    return;
  }
  input_text_->ChangeValue(path[0]);
  output_text_->ChangeValue(first_filename.GetPath());
}

void GuiFrame::OnDropOutput(wxDropFilesEvent& event) {
  wxString* path = event.GetFiles();
  if (!wxDirExists(path[0])) {
    return;
  }
  output_text_->ChangeValue(path[0]);
}

void GuiFrame::OnDropFont(wxDropFilesEvent& event) {
  wxString* path = event.GetFiles();
  if (!wxDirExists(path[0])) {
    return;
  }
  font_text_->ChangeValue(path[0]);
}

void GuiFrame::OnDropDB(wxDropFilesEvent& event) {
  wxString* path = event.GetFiles();
  if (!wxDirExists(path[0])) {
    return;
  }
  db_text_->ChangeValue(path[0]);
  auto db_path = fs::path(db_text_->GetValue().ToStdString() + "/fonts.db");
  if (fs::is_regular_file(db_path)) {
    logger_->info("Found fonts database: \"{}\"", db_path.generic_string());
  } else {
    logger_->warn("Fonts database not found.");
  }
}

void GuiFrame::OnRun(wxCommandEvent& WXUNUSED(event)) {
  if (is_running_) {
    return;
  }
  if (input_text_->IsEmpty()) {
    logger_->error("No Input ASS file.");
    return;
  }
  if (output_text_->IsEmpty()) {
    logger_->error("No Output directory.");
    return;
  }
  if (font_text_->IsEmpty() && db_text_->IsEmpty()) {
    logger_->error("No font directory.");
    return;
  }
  fs::path input_path =
      fs::system_complete(input_text_->GetValue().ToStdString());
  fs::path output_path =
      fs::system_complete(output_text_->GetValue().ToStdString());
  fs::path fonts_path =
      fs::system_complete(font_text_->GetValue().ToStdString());
  fs::path db_path = fs::system_complete(db_text_->GetValue().ToStdString());
  boost::thread thread([=] {
    is_running_ = true;
    Run(input_path, output_path, fonts_path, db_path, subset_check_->GetValue(),
        embed_check_->GetValue(), sink_);
    is_running_ = false;
  });
  thread.detach();
}

void GuiFrame::OnBuild(wxCommandEvent& WXUNUSED(event)) {
  if (is_running_) {
    return;
  }
  if (font_text_->IsEmpty()) {
    logger_->error("No Font directory.");
    return;
  }
  if (db_text_->IsEmpty()) {
    logger_->error("No Database directory.");
    return;
  }
  fs::path fonts_path =
      fs::system_complete(font_text_->GetValue().ToStdString());
  fs::path db_path = fs::system_complete(db_text_->GetValue().ToStdString());
  logger_->info("Building fonts database.");
  boost::thread thread([=] {
    is_running_ = true;
    BuildDB(fonts_path, db_path, sink_);
    is_running_ = false;
  });
  thread.detach();
}

void GuiFrame::OnReset(wxCommandEvent& WXUNUSED(event)) {
  subset_check_->SetValue(false);
  embed_check_->SetValue(false);
  input_text_->Clear();
  output_text_->Clear();
  font_text_->Clear();
  db_text_->ChangeValue(app_path_);
  log_text_->Clear();
  logger_->info("assfonts-gui v{}.{}.{}", VERSION_MAX, VERSION_MID,
                VERSION_MIN);
  auto db_path = fs::path(db_text_->GetValue().ToStdString() + "/fonts.db");
  if (fs::is_regular_file(db_path)) {
    logger_->info("Found fonts database: \"{}\"", db_path.generic_string());
  } else {
    logger_->warn("Fonts database not found.");
  }
}