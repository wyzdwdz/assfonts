#ifndef ASSFONTS_ASSFONTSGUI_H_
#define ASSFONTS_ASSFONTSGUI_H_

#include <wx/wx.h>

class GuiFrame : public wxFrame {
 public:
  GuiFrame(wxWindow* parent, wxWindowID id = wxID_ANY,
           const wxString& title = wxEmptyString,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxSize(800, 600),
           long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

  ~GuiFrame();

 protected:
  wxPanel* main_panel;
  wxStaticText* input_label;
  wxTextCtrl* input_text;
  wxButton* input_button;
  wxStaticText* output_label;
  wxTextCtrl* output_text;
  wxButton* output_button;
  wxStaticText* font_label;
  wxTextCtrl* font_text;
  wxButton* font_button;
  wxStaticText* db_label;
  wxTextCtrl* db_text;
  wxButton* db_button;
  wxCheckBox* subset_check;
  wxCheckBox* embed_check;
  wxButton* build_button;
  wxButton* run_button;
  wxTextCtrl* log_text;
};

#endif