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

#include <array>
#include <cmath>
#include <filesystem>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <wx/filename.h>
#include <wx/stdpaths.h>

#ifndef _WIN32
#include "icon.xpm"
#endif
#include "ass_string.h"
#include "assfonts_gui.h"
#include "run.h"
#include "wxwidgets_sink.h"

#include "ver.h"

#ifdef _WIN32
#define ToAString() ToStdWstring()
#else
#define ToAString() ToStdString(wxConvUTF8)
#endif

namespace fs = std::filesystem;

GuiFrame::GuiFrame(wxWindow* parent, wxWindowID id, const wxString& title,
                   const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style) {
  wxFileName f(wxStandardPaths::Get().GetExecutablePath());
  app_path_ = f.GetPath();

  this->SetTitle(_T("assfonts"));
  this->SetSize(FromDIP(wxSize(800, 700)));
  this->SetMinSize(FromDIP(wxSize(800, 700)));
  this->SetMaxSize(FromDIP(wxSize(800, -1)));
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

  input_label_ = new wxStaticText(main_panel_, wxID_ANY, _T("Input\nASS files"),
                                  wxDefaultPosition, wxDefaultSize,
                                  wxALIGN_CENTER_HORIZONTAL);
  input_label_->Wrap(-1);
  wxFont font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  input_label_->SetFont(font);
  top_sizer->Add(input_label_, 0, wxALIGN_CENTER | wxALL, FromDIP(10));

  input_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(580, 50)), wxTE_MULTILINE | wxTE_READONLY);
  input_text_->DragAcceptFiles(true);
  top_sizer->Add(input_text_, 0, wxALL, FromDIP(5));

  input_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."),
                               wxDefaultPosition, FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(input_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  input_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("\u2715"), wxDefaultPosition,
                   FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(input_clean_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  output_label_ = new wxStaticText(main_panel_, wxID_ANY,
                                   _T("Output\ndirectory"), wxDefaultPosition,
                                   wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
  output_label_->Wrap(-1);
  font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  output_label_->SetFont(font);
  top_sizer->Add(output_label_, 0, wxALIGN_CENTER | wxALL, FromDIP(10));

  output_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(580, 50)), wxTE_MULTILINE | wxTE_READONLY);
  output_text_->DragAcceptFiles(true);
  top_sizer->Add(output_text_, 0, wxALL, FromDIP(5));

  output_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."),
                                wxDefaultPosition, FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(output_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  output_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("\u2715"), wxDefaultPosition,
                   FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(output_clean_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  font_label_ = new wxStaticText(main_panel_, wxID_ANY, _T("Font\ndirectory"),
                                 wxDefaultPosition, wxDefaultSize,
                                 wxALIGN_CENTER_HORIZONTAL);
  font_label_->Wrap(-1);
  font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  font_label_->SetFont(font);
  top_sizer->Add(font_label_, 0, wxALIGN_CENTER | wxALL, FromDIP(10));

  font_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(580, 50)), wxTE_MULTILINE | wxTE_READONLY);
  font_text_->DragAcceptFiles(true);
  top_sizer->Add(font_text_, 0, wxALL, FromDIP(5));

  font_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."),
                              wxDefaultPosition, FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(font_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  font_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("\u2715"), wxDefaultPosition,
                   FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(font_clean_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  db_label_ = new wxStaticText(main_panel_, wxID_ANY, _T("Database\ndirectory"),
                               wxDefaultPosition, wxDefaultSize,
                               wxALIGN_CENTER_HORIZONTAL);
  db_label_->Wrap(-1);
  font = input_label_->GetFont();
  font.SetWeight(wxFONTWEIGHT_BOLD);
  db_label_->SetFont(font);
  top_sizer->Add(db_label_, 0, wxALIGN_CENTER | wxALL, FromDIP(10));

  db_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     FromDIP(wxSize(580, 50)), wxTE_MULTILINE | wxTE_READONLY);
  db_text_->DragAcceptFiles(true);
  db_text_->SetValue(app_path_);
  top_sizer->Add(db_text_, 0, wxALL, FromDIP(5));

  db_button_ = new wxButton(main_panel_, wxID_ANY, _T("..."), wxDefaultPosition,
                            FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(db_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  db_clean_button_ =
      new wxButton(main_panel_, wxID_ANY, _T("\u2715"), wxDefaultPosition,
                   FromDIP(wxSize(35, 35)), 0);
  top_sizer->Add(db_clean_button_, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

  inner_sizer->Add(top_sizer, 0, wxALIGN_CENTER | wxTOP, FromDIP(10));

  wxBoxSizer* middle_sizer;
  middle_sizer = new wxBoxSizer(wxHORIZONTAL);

  wxFlexGridSizer* checkbox_sizer;
  checkbox_sizer = new wxFlexGridSizer(2, 2, 0, 0);

  hdr_high_check_ = new wxCheckBox(main_panel_, wxID_ANY, _T("HDR High"),
                                   wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(hdr_high_check_, 1, wxALL, FromDIP(10));

  subset_check_ = new wxCheckBox(main_panel_, wxID_ANY, _T("Subset only"),
                                 wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(subset_check_, 1, wxALL, FromDIP(10));

  hdr_low_check_ = new wxCheckBox(main_panel_, wxID_ANY, _T("HDR Low"),
                                  wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(hdr_low_check_, 1, wxALL, FromDIP(10));

  embed_check_ = new wxCheckBox(main_panel_, wxID_ANY, _T("Embed only"),
                                wxDefaultPosition, wxDefaultSize, 0);
  checkbox_sizer->Add(embed_check_, 1, wxALL, FromDIP(10));

  middle_sizer->Add(checkbox_sizer, 0, wxALIGN_CENTER | wxALL, FromDIP(10));

  run_button_ = new wxButton(main_panel_, wxID_ANY, _T("RUN"),
                             wxDefaultPosition, FromDIP(wxSize(70, 70)), 0);
  font = run_button_->GetFont();
  font.SetPointSize(11);
  font.SetWeight(wxFONTWEIGHT_BOLD);
  run_button_->SetFont(font);
  middle_sizer->Add(run_button_, 1, wxALIGN_CENTER | wxALL, FromDIP(5));

  wxBoxSizer* button_sizer;
  button_sizer = new wxBoxSizer(wxVERTICAL);

  build_button_ = new wxButton(main_panel_, wxID_ANY, _T("Build database"),
                               wxDefaultPosition, FromDIP(wxSize(-1, 30)), 0);
  button_sizer->Add(build_button_, 1, wxALL | wxEXPAND, FromDIP(5));

  reset_button_ = new wxButton(main_panel_, wxID_ANY, _T("Reset all"),
                               wxDefaultPosition, FromDIP(wxSize(-1, 30)), 0);
  button_sizer->Add(reset_button_, 1, wxALL | wxEXPAND, FromDIP(5));

  middle_sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxLEFT, FromDIP(20));

  middle_sizer->Add(0, 0, 1, wxALIGN_CENTER | wxALL, FromDIP(5));

  inner_sizer->Add(middle_sizer, 0, wxALIGN_CENTER | wxALL, FromDIP(15));

  log_text_ =
      new wxTextCtrl(main_panel_, wxID_ANY, wxEmptyString, wxDefaultPosition,
                     wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
  inner_sizer->Add(log_text_, 1, wxEXPAND, 0);

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
                            [this](wxCommandEvent&) { input_text_->Clear(); });
  output_clean_button_->Bind(
      wxEVT_COMMAND_BUTTON_CLICKED,
      [this](wxCommandEvent&) { output_text_->Clear(); });
  font_clean_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                           [this](wxCommandEvent&) { font_text_->Clear(); });
  db_clean_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                         [this](wxCommandEvent&) { db_text_->Clear(); });

  input_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropInput, this);
  output_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropOutput, this);
  font_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropFont, this);
  db_text_->Bind(wxEVT_DROP_FILES, &GuiFrame::OnDropDB, this);

  run_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnRun, this);
  build_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnBuild, this);
  reset_button_->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GuiFrame::OnReset, this);

  hdr_high_check_->Bind(
      wxEVT_COMMAND_CHECKBOX_CLICKED,
      [this](wxCommandEvent&) { hdr_low_check_->SetValue(false); });
  hdr_low_check_->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, [this](wxCommandEvent&) {
    hdr_high_check_->SetValue(false);
  });

  SetColours();
  log_text_->Bind(SPDLOG_EVT, &GuiFrame::OnAppendLog, this);

  spdlog::init_thread_pool(512, 1);
  sink_ = std::make_shared<mylog::sinks::wxwidgets_sink_mt>(log_text_);
  logger_ = std::make_shared<spdlog::async_logger>("main", sink_,
                                                   spdlog::thread_pool());
  logger_->set_pattern("[%^%l%$] %v");
  spdlog::register_logger(logger_);

  logger_->info("assfonts-gui v{}.{}.{}", VERSION_MAJOR, VERSION_MINOR,
                VERSION_PATCH);

  auto db_path = fs::path(db_text_->GetValue().ToAString() +
                          fs::path::preferred_separator + _ST("fonts.json"));
  if (fs::is_regular_file(db_path)) {
    logger_->info(_ST("Found fonts database: \"{}\""), db_path.native());
  } else {
    logger_->warn(_ST("Fonts database not found."));
  }
}

GuiFrame::~GuiFrame() {
  spdlog::shutdown();
}

void GuiFrame::OnFindInput(wxCommandEvent& WXUNUSED(event)) {
  wxFileDialog open_file_dialog(
      this, _T("Open ASS files"), "", "",
      _T("ASS files (*.ass *.ssa)|")
      _T("*.ass;*.Ass;*.aSs;*.asS;*.ASs;*.AsS;*.aSS;*.ASS;")
      _T("*.ssa;*.Ssa;*.sSa;*.ssA;*.SSa;*.SsA;*.sSA;*.SSA"),
      wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
  if (open_file_dialog.ShowModal() == wxID_CANCEL) {
    return;
  }
  input_paths_.clear();
  open_file_dialog.GetPaths(input_paths_);
  wxString dir = open_file_dialog.GetDirectory();
  input_text_->Clear();
  input_paths_.Sort();
  size_t cnt = input_paths_.GetCount();
  for (size_t i = 0; i < cnt - 1; ++i) {
    *input_text_ << input_paths_[i] << _T('\n');
  }
  *input_text_ << input_paths_[cnt - 1];
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
  auto db_path = fs::path(db_text_->GetValue().ToAString() +
                          fs::path::preferred_separator + _ST("fonts.json"));
  if (fs::is_regular_file(db_path)) {
    logger_->info(_ST("Found fonts database: \"{}\""), db_path.native());
  } else {
    logger_->warn(_ST("Fonts database not found."));
  }
}

void GuiFrame::OnDropInput(wxDropFilesEvent& event) {
  wxString* path = event.GetFiles();
  size_t cnt = event.GetNumberOfFiles();
  wxArrayString input_paths_backup = input_paths_;
  input_paths_.clear();
  for (size_t i = 0; i < cnt; ++i) {
    if (!wxFileExists(path[i])) {
      continue;
    }
    wxFileName filename = wxFileName(path[i]);
    wxString ext = filename.GetExt();
    if (!filename.GetExt().IsSameAs("ass", false) &&
        !filename.GetExt().IsSameAs("ssa", false)) {
      continue;
    }
    input_paths_.push_back(path[i]);
  }
  if (input_paths_.IsEmpty()) {
    input_paths_ = input_paths_backup;
    return;
  }
  input_text_->Clear();
  input_paths_.Sort();
  cnt = input_paths_.GetCount();
  for (size_t i = 0; i < cnt - 1; ++i) {
    *input_text_ << input_paths_[i] << _T('\n');
  }
  *input_text_ << input_paths_[cnt - 1];
  wxFileName filename = wxFileName(input_paths_[0]);
  output_text_->ChangeValue(filename.GetPath());
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
  auto db_path = fs::path(db_text_->GetValue().ToAString() +
                          fs::path::preferred_separator + _ST("fonts.json"));
  if (fs::is_regular_file(db_path)) {
    logger_->info(_ST("Found fonts database: \"{}\""), db_path.native());
  } else {
    logger_->warn(_ST("Fonts database not found."));
  }
}

void GuiFrame::OnRun(wxCommandEvent& WXUNUSED(event)) {
  if (is_running_) {
    return;
  }
  if (input_text_->IsEmpty() || input_paths_.IsEmpty()) {
    logger_->error(_ST("No Input ASS file."));
    return;
  }
  if (output_text_->IsEmpty()) {
    logger_->error(_ST("No Output directory."));
    return;
  }
  if (font_text_->IsEmpty() && db_text_->IsEmpty()) {
    logger_->error(_ST("No font directory."));
    return;
  }
  std::vector<fs::path> input_paths;
  std::error_code ec;
  for (const auto& input_path : input_paths_) {
    input_paths.emplace_back(fs::absolute(input_path.ToAString(), ec));
  }
  fs::path output_path = fs::absolute(output_text_->GetValue().ToAString(), ec);
  fs::path fonts_path = fs::absolute(font_text_->GetValue().ToAString(), ec);
  fs::path db_path = fs::absolute(db_text_->GetValue().ToAString(), ec);
  std::thread thread([this, input_paths, output_path, fonts_path, db_path] {
    is_running_ = true;
    unsigned int brightness = 0;
    if (hdr_high_check_->GetValue()) {
      brightness = 203;
    } else if (hdr_low_check_->GetValue()) {
      brightness = 100;
    }
    bool is_subset_only = subset_check_->GetValue();
    bool is_embed_only = subset_check_->GetValue();
    Run(input_paths, output_path, fonts_path, db_path, brightness,
        is_subset_only, is_embed_only, sink_);
    is_running_ = false;
  });
  thread.detach();
}

void GuiFrame::OnBuild(wxCommandEvent& WXUNUSED(event)) {
  if (is_running_) {
    return;
  }
  if (font_text_->IsEmpty()) {
    logger_->error(_ST("No Font directory."));
    return;
  }
  if (db_text_->IsEmpty()) {
    logger_->error(_ST("No Database directory."));
    return;
  }
  std::error_code ec;
  fs::path fonts_path = fs::absolute(font_text_->GetValue().ToAString(), ec);
  fs::path db_path = fs::absolute(db_text_->GetValue().ToAString(), ec);
  logger_->info(_ST("Building fonts database."));
  std::thread thread([this, fonts_path, db_path]() {
    is_running_ = true;
    BuildDB(fonts_path, db_path, sink_);
    font_text_->Clear();
    is_running_ = false;
  });
  thread.detach();
}

void GuiFrame::OnReset(wxCommandEvent& WXUNUSED(event)) {
  if (is_running_) {
    return;
  }
  subset_check_->SetValue(false);
  embed_check_->SetValue(false);
  hdr_high_check_->SetValue(false);
  hdr_low_check_->SetValue(false);
  input_text_->Clear();
  output_text_->Clear();
  font_text_->Clear();
  db_text_->ChangeValue(app_path_);
  log_text_->Clear();
  logger_->info(_ST("assfonts-gui v{}.{}.{}"), VERSION_MAJOR, VERSION_MINOR,
                VERSION_PATCH);
  auto db_path = fs::path(db_text_->GetValue().ToAString() +
                          fs::path::preferred_separator + _ST("fonts.json"));
  if (fs::is_regular_file(db_path)) {
    logger_->info(_ST("Found fonts database: \"{}\""), db_path.native());
  } else {
    logger_->warn(_ST("Fonts database not found."));
  }
}

void GuiFrame::OnAppendLog(wxCommandEvent& event) {
  if (event.GetInt() == 10) {
    log_text_->Clear();
    return;
  }
  if (event.GetInt() == 0) {
    log_text_->SetDefaultStyle(wxTextAttr(wxNullColour));
  } else if (event.GetInt() == 1) {
    log_text_->SetDefaultStyle(wxTextAttr(warn_colour_));
  } else if (event.GetInt() == 2) {
    log_text_->SetDefaultStyle(wxTextAttr(err_colour_));
  } else {
    log_text_->SetDefaultStyle(wxTextAttr(wxNullColour));
  }
  log_text_->AppendText(event.GetString());
}

void GuiFrame::SetColours() {
  wxColour background_colour = log_text_->GetBackgroundColour();
  std::array<double, 3> rgb;
  rgb[0] = background_colour.GetRed() / 255.0;
  rgb[1] = background_colour.GetGreen() / 255.0;
  rgb[2] = background_colour.GetBlue() / 255.0;
  for (double& v : rgb) {
    if (v <= 0.04045) {
      v = v / 12.92;
    } else {
      v = std::pow(((v + 0.055) / 1.055), 2.4);
    }
  }
  double Y = 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2];
  double L;
  if (Y <= (216.0 / 24389.0)) {
    L = Y * (24389.0 / 27.0);
  } else {
    L = std::pow(Y, (1.0 / 3.0)) * 116.0 - 16.0;
  }
  if (L > 50.0) {
    warn_colour_ = *wxBLUE;
    err_colour_ = *wxRED;
  } else {
    warn_colour_ = *wxYELLOW;
    err_colour_ = *wxCYAN;
  }
}