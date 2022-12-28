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

#ifndef ASSFONTS_ASSFREETYPE_H_
#define ASSFONTS_ASSFREETYPE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#ifdef __cplusplus
}
#endif

#include "ass_string.h"

namespace ass {

FT_Error NewOpenArgs(const AString& filepathname, FT_StreamRec& stream,
                     FT_Open_Args& args);

}

#endif