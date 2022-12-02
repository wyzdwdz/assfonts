#include <wx/wx.h>

#include "gui_frame.h"

class GuiApp : public wxApp {
 public:
  virtual bool OnInit();
};

wxIMPLEMENT_APP(GuiApp);

bool GuiApp::OnInit()
{
  GuiFrame* gui_frame = new GuiFrame(nullptr);
  gui_frame->Show(true);
  return true;
}