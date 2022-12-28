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

#include "ass_freetype.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ass {

#ifdef _WIN32

void* ft_alloc(FT_Memory memory, long size) {
  return HeapAlloc(memory->user, 0, size);
}

void ft_free(FT_Memory memory, void* block) {
  if (memory != nullptr) {
    HeapFree(memory->user, 0, block);
  }
}

void ft_close_stream_by_munmap(FT_Stream stream) {
  UnmapViewOfFile(static_cast<LPCVOID>(stream->descriptor.pointer));
  stream->descriptor.pointer = NULL;
  stream->size = 0;
  stream->base = NULL;
}

void ft_close_stream_by_free(FT_Stream stream) {
  ft_free(stream->memory, stream->descriptor.pointer);
  stream->descriptor.pointer = NULL;
  stream->size = 0;
  stream->base = NULL;
}

#endif

FT_Error NewOpenArgs(const AString& filepathname, FT_StreamRec& stream,
                     FT_Open_Args& args) {
#ifdef _WIN32
  /****************************************************************************
 *
 * ftsystem.c
 *
 *   Windows-specific FreeType low-level system interface (body).
 *
 * Copyright (C) 2021-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

  HANDLE file;
  HANDLE fm;
  LARGE_INTEGER size;
  ft_free(stream.memory, stream.descriptor.pointer);
  stream.descriptor.pointer = NULL;
  stream.size = 0;
  stream.base = NULL;
  file = CreateFileW(static_cast<LPCWSTR>(filepathname.c_str()), GENERIC_READ,
                     FILE_SHARE_READ, NULL, OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL, 0);
  if (file == INVALID_HANDLE_VALUE) {
    return FT_Err_Cannot_Open_Resource;
  }
  if (GetFileSizeEx(file, &size) == FALSE) {
    goto Fail_Open;
  }
  if (size.QuadPart > LONG_MAX) {
    goto Fail_Open;
  } else if (size.QuadPart == 0) {
    goto Fail_Open;
  }
  fm = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
  if (fm == NULL) {
    goto Fail_Open;
  }
  stream.size = size.LowPart;
  stream.pos = 0;
  stream.base =
      static_cast<unsigned char*>(MapViewOfFile(fm, FILE_MAP_READ, 0, 0, 0));
  CloseHandle(fm);
  if (stream.base != NULL)
    stream.close = ft_close_stream_by_munmap;
  else {
    DWORD total_read_count;
    stream.base =
        static_cast<unsigned char*>(ft_alloc(stream.memory, stream.size));
    if (!stream.base) {
      goto Fail_Open;
    }
    total_read_count = 0;
    do {
      DWORD read_count;
      if (ReadFile(file, stream.base + total_read_count,
                   stream.size - total_read_count, &read_count,
                   NULL) == FALSE) {
        goto Fail_Read;
      }
      total_read_count += read_count;
    } while (total_read_count != stream.size);
    stream.close = ft_close_stream_by_free;
  }
  CloseHandle(file);
  stream.descriptor.pointer = stream.base;
  stream.pathname.pointer = NULL;
  stream.read = NULL;

  args.flags = FT_OPEN_STREAM;
  args.memory_base = NULL;
  args.memory_size = 0;
  args.pathname = NULL;
  args.stream = &stream;
  args.driver = NULL;
  args.num_params = 0;
  args.params = NULL;
  return FT_Err_Ok;

Fail_Read:
  ft_free(stream.memory, stream.base);
Fail_Open:
  CloseHandle(file);
  stream.base = NULL;
  stream.size = 0;
  stream.pos = 0;
  return FT_Err_Cannot_Open_Stream;

#else

  args.flags = FT_OPEN_PATHNAME;
  args.memory_base = NULL;
  args.memory_size = 0;
  args.pathname = const_cast<FT_String*>(filepathname.c_str());
  args.stream = NULL;
  args.driver = NULL;
  args.num_params = 0;
  args.params = NULL;
  return FT_Err_Ok;

#endif
}

}  // namespace ass