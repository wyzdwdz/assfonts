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

#include <wx/setup.h>
#include <wx/wx.h>

constexpr int VERSION_MAX = 0;
constexpr int VERSION_MID = 2;
constexpr int VERSION_MIN = 5;

class GuiApp : public wxApp {
 public:
  virtual bool OnInit();
};

#endif