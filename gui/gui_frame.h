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

#ifndef ASSFONTS_ASSFONTSGUI_H_
#define ASSFONTS_ASSFONTSGUI_H_

#include <memory>

#include <spdlog/spdlog.h>
#include <wx/wx.h>

#include "wxwidgets_sink.h"

class GuiFrame : public wxFrame {
 public:
  GuiFrame(wxWindow* parent, wxWindowID id = wxID_ANY,
           const wxString& title = wxEmptyString,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxSize(800, 600),
           long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

  ~GuiFrame();

 private:
  wxPanel* main_panel_;
  wxStaticText* input_label_;
  wxTextCtrl* input_text_;
  wxButton* input_button_;
  wxButton* input_clean_button_;
  wxStaticText* output_label_;
  wxTextCtrl* output_text_;
  wxButton* output_button_;
  wxButton* output_clean_button_;
  wxStaticText* font_label_;
  wxTextCtrl* font_text_;
  wxButton* font_button_;
  wxButton* font_clean_button_;
  wxStaticText* db_label_;
  wxTextCtrl* db_text_;
  wxButton* db_button_;
  wxButton* db_clean_button_;
  wxCheckBox* subset_check_;
  wxCheckBox* embed_check_;
  wxButton* build_button_;
  wxButton* run_button_;
  wxButton* reset_button_;
  wxTextCtrl* log_text_;

  std::shared_ptr<mylog::sinks::wxwidgets_sink_mt> sink_;
  std::shared_ptr<spdlog::logger> logger_;
  bool is_running_ = false;
  wxString app_path_;

  void OnFindInput(wxCommandEvent& WXUNUSED(event));
  void OnFindOutput(wxCommandEvent& WXUNUSED(event));
  void OnFindFont(wxCommandEvent& WXUNUSED(event));
  void OnFindDB(wxCommandEvent& WXUNUSED(event));
  void OnDropInput(wxDropFilesEvent& event);
  void OnDropOutput(wxDropFilesEvent& event);
  void OnDropFont(wxDropFilesEvent& event);
  void OnDropDB(wxDropFilesEvent& event);
  void OnRun(wxCommandEvent& WXUNUSED(event));
  void OnBuild(wxCommandEvent& WXUNUSED(event));
  void OnReset(wxCommandEvent& WXUNUSED(event));
};

#endif