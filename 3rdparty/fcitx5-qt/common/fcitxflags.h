/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef _COMMON_FCITXFLAGS_H_
#define _COMMON_FCITXFLAGS_H_

#include <cstdint>

namespace fcitx {

// This need to keep sync with fcitx5.
enum FcitxCapabilityFlag : uint64_t {
    FcitxCapabilityFlag_Preedit = (1 << 1),
    FcitxCapabilityFlag_Password = (1 << 3),
    FcitxCapabilityFlag_FormattedPreedit = (1 << 4),
    FcitxCapabilityFlag_ClientUnfocusCommit = (1 << 5),
    FcitxCapabilityFlag_SurroundingText = (1 << 6),
    FcitxCapabilityFlag_Email = (1 << 7),
    FcitxCapabilityFlag_Digit = (1 << 8),
    FcitxCapabilityFlag_Uppercase = (1 << 9),
    FcitxCapabilityFlag_Lowercase = (1 << 10),
    FcitxCapabilityFlag_NoAutoUpperCase = (1 << 11),
    FcitxCapabilityFlag_Url = (1 << 12),
    FcitxCapabilityFlag_Dialable = (1 << 13),
    FcitxCapabilityFlag_Number = (1 << 14),
    FcitxCapabilityFlag_NoSpellCheck = (1 << 17),
    FcitxCapabilityFlag_Alpha = (1 << 21),
    FcitxCapabilityFlag_GetIMInfoOnFocus = (1 << 23),
    FcitxCapabilityFlag_RelativeRect = (1 << 24),

    FcitxCapabilityFlag_Multiline = (1ull << 35),
    FcitxCapabilityFlag_Sensitive = (1ull << 36),
    FcitxCapabilityFlag_KeyEventOrderFix = (1ull << 37),
    FcitxCapabilityFlag_ReportKeyRepeat = (1ull << 38),
    FcitxCapabilityFlag_ClientSideInputPanel = (1ull << 39),
    FcitxCapabilityFlag_Disable = (1ull << 40),
};

enum FcitxTextFormatFlag : int {
    FcitxTextFormatFlag_Underline = (1 << 3), /**< underline is a flag */
    FcitxTextFormatFlag_HighLight = (1 << 4), /**< highlight the preedit */
    FcitxTextFormatFlag_DontCommit = (1 << 5),
    FcitxTextFormatFlag_Bold = (1 << 6),
    FcitxTextFormatFlag_Strike = (1 << 7),
    FcitxTextFormatFlag_Italic = (1 << 8),
    FcitxTextFormatFlag_None = 0,
};

enum FcitxKeyStates {
    FcitxKeyState_Alt = 1 << 3,
    FcitxKeyState_Shift = 1 << 0,
    FcitxKeyState_Ctrl = 1 << 2,
};

enum class FcitxCandidateLayoutHint { NotSet, Vertical, Horizontal };

} // namespace fcitx

#endif // _COMMON_FCITXFLAGS_H_
