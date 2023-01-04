/*  This file is part of asshdr.
 *
 *  asshdr is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  asshdr is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with asshdr. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  written by wyzdwdz (https://github.com/wyzdwdz)
 */

#ifndef ASSHDR_RECOLORIZE_H_
#define ASSHDR_RECOLORIZE_H_

namespace asshdr {

bool AssRecolor(const char* input_str, const unsigned int& input_size,
                char* output_str, unsigned int& output_size,
                const unsigned int input_brightness);

}

#endif