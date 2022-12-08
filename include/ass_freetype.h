#ifndef ASSFONTS_ASSFREETYPE_H_
#define ASSFONTS_ASSFREETYPE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef __cplusplus
}
#endif

#include <ass_string.h>

namespace ass {

FT_Error NewOpenArgs(const AString& filepathname, FT_StreamRec& stream,
                     FT_Open_Args& args);

}

#endif