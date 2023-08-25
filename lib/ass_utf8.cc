/*  This file is part of assfonts.
 *
 *  assfonts is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 3 of the License,
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

#include "ass_utf8.h"

#include <cstring>

namespace ass {

unsigned char* StrToLwrExt(unsigned char* pString) {
  /* https://stackoverflow.com/a/53920363 */

  unsigned char* p = pString;
  unsigned char* pExtChar = 0;

  if (pString && *pString) {
    while (*p) {
      if ((*p >= 0x41) && (*p <= 0x5a)) /* US ASCII */
        (*p) += 0x20;
      else if (*p > 0xc0) {
        pExtChar = p;
        p++;
        switch (*pExtChar) {
          case 0xc3: /* Latin 1 */
            if ((*p >= 0x80) && (*p <= 0x9e) && (*p != 0x97))
              (*p) += 0x20; /* US ASCII shift */
            break;
          case 0xc4: /* Latin ext */
            if (((*p >= 0x80) && (*p <= 0xb7) && (*p != 0xb0)) &&
                (!(*p % 2))) /* Even */
              (*p)++;        /* Next char is lwr */
            else if ((*p >= 0xb9) && (*p <= 0xbe) && (*p % 2)) /* Odd */
              (*p)++; /* Next char is lwr */
            else if (*p == 0xbf) {
              *pExtChar = 0xc5;
              (*p) = 0x80;
            }
            break;
          case 0xc5:                                      /* Latin ext */
            if ((*p >= 0x81) && (*p <= 0x88) && (*p % 2)) /* Odd */
              (*p)++;                                     /* Next char is lwr */
            else if ((*p >= 0x8a) && (*p <= 0xb7) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            else if (*p == 0xb8) {
              *pExtChar = 0xc3;
              (*p) = 0xbf;
            } else if ((*p >= 0xb9) && (*p <= 0xbe) && (*p % 2)) /* Odd */
              (*p)++; /* Next char is lwr */
            break;
          case 0xc6: /* Latin ext */
            switch (*p) {
              case 0x81:
                *pExtChar = 0xc9;
                (*p) = 0x93;
                break;
              case 0x86:
                *pExtChar = 0xc9;
                (*p) = 0x94;
                break;
              case 0x89:
                *pExtChar = 0xc9;
                (*p) = 0x96;
                break;
              case 0x8a:
                *pExtChar = 0xc9;
                (*p) = 0x97;
                break;
              case 0x8e:
                *pExtChar = 0xc9;
                (*p) = 0x98;
                break;
              case 0x8f:
                *pExtChar = 0xc9;
                (*p) = 0x99;
                break;
              case 0x90:
                *pExtChar = 0xc9;
                (*p) = 0x9b;
                break;
              case 0x93:
                *pExtChar = 0xc9;
                (*p) = 0xa0;
                break;
              case 0x94:
                *pExtChar = 0xc9;
                (*p) = 0xa3;
                break;
              case 0x96:
                *pExtChar = 0xc9;
                (*p) = 0xa9;
                break;
              case 0x97:
                *pExtChar = 0xc9;
                (*p) = 0xa8;
                break;
              case 0x9c:
                *pExtChar = 0xc9;
                (*p) = 0xaf;
                break;
              case 0x9d:
                *pExtChar = 0xc9;
                (*p) = 0xb2;
                break;
              case 0x9f:
                *pExtChar = 0xc9;
                (*p) = 0xb5;
                break;
              case 0xa9:
                *pExtChar = 0xca;
                (*p) = 0x83;
                break;
              case 0xae:
                *pExtChar = 0xca;
                (*p) = 0x88;
                break;
              case 0xb1:
                *pExtChar = 0xca;
                (*p) = 0x8a;
                break;
              case 0xb2:
                *pExtChar = 0xca;
                (*p) = 0x8b;
                break;
              case 0xb7:
                *pExtChar = 0xca;
                (*p) = 0x92;
                break;
              case 0x82:
              case 0x84:
              case 0x87:
              case 0x8b:
              case 0x91:
              case 0x98:
              case 0xa0:
              case 0xa2:
              case 0xa4:
              case 0xa7:
              case 0xac:
              case 0xaf:
              case 0xb3:
              case 0xb5:
              case 0xb8:
              case 0xbc:
                (*p)++; /* Next char is lwr */
                break;
              default:
                break;
            }
            break;
          case 0xc7: /* Latin ext */
            if (*p == 0x84)
              (*p) = 0x86;
            else if (*p == 0x85)
              (*p)++; /* Next char is lwr */
            else if (*p == 0x87)
              (*p) = 0x89;
            else if (*p == 0x88)
              (*p)++; /* Next char is lwr */
            else if (*p == 0x8a)
              (*p) = 0x8c;
            else if (*p == 0x8b)
              (*p)++; /* Next char is lwr */
            else if ((*p >= 0x8d) && (*p <= 0x9c) && (*p % 2)) /* Odd */
              (*p)++; /* Next char is lwr */
            else if ((*p >= 0x9e) && (*p <= 0xaf) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            else if (*p == 0xb1)
              (*p) = 0xb3;
            else if (*p == 0xb2)
              (*p)++; /* Next char is lwr */
            else if (*p == 0xb4)
              (*p)++; /* Next char is lwr */
            else if (*p == 0xb6) {
              *pExtChar = 0xc6;
              (*p) = 0x95;
            } else if (*p == 0xb7) {
              *pExtChar = 0xc6;
              (*p) = 0xbf;
            } else if ((*p >= 0xb8) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            break;
          case 0xc8:                                         /* Latin ext */
            if ((*p >= 0x80) && (*p <= 0x9f) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            else if (*p == 0xa0) {
              *pExtChar = 0xc6;
              (*p) = 0x9e;
            } else if ((*p >= 0xa2) && (*p <= 0xb3) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            else if (*p == 0xbb)
              (*p)++; /* Next char is lwr */
            else if (*p == 0xbd) {
              *pExtChar = 0xc6;
              (*p) = 0x9a;
            }
            /* 0xba three byte small 0xe2 0xb1 0xa5 */
            /* 0xbe three byte small 0xe2 0xb1 0xa6 */
            break;
          case 0xc9: /* Latin ext */
            if (*p == 0x81)
              (*p)++; /* Next char is lwr */
            else if (*p == 0x83) {
              *pExtChar = 0xc6;
              (*p) = 0x80;
            } else if (*p == 0x84) {
              *pExtChar = 0xca;
              (*p) = 0x89;
            } else if (*p == 0x85) {
              *pExtChar = 0xca;
              (*p) = 0x8c;
            } else if ((*p >= 0x86) && (*p <= 0x8f) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            break;
          case 0xcd: /* Greek & Coptic */
            switch (*p) {
              case 0xb0:
              case 0xb2:
              case 0xb6:
                (*p)++; /* Next char is lwr */
                break;
              case 0xbf:
                *pExtChar = 0xcf;
                (*p) = 0xb3;
                break;
              default:
                break;
            }
            break;
          case 0xce: /* Greek & Coptic */
            if (*p == 0x86)
              (*p) = 0xac;
            else if (*p == 0x88)
              (*p) = 0xad;
            else if (*p == 0x89)
              (*p) = 0xae;
            else if (*p == 0x8a)
              (*p) = 0xaf;
            else if (*p == 0x8c) {
              *pExtChar = 0xcf;
              (*p) = 0x8c;
            } else if (*p == 0x8e) {
              *pExtChar = 0xcf;
              (*p) = 0x8d;
            } else if (*p == 0x8f) {
              *pExtChar = 0xcf;
              (*p) = 0x8e;
            } else if ((*p >= 0x91) && (*p <= 0x9f))
              (*p) += 0x20; /* US ASCII shift */
            else if ((*p >= 0xa0) && (*p <= 0xab) && (*p != 0xa2)) {
              *pExtChar = 0xcf;
              (*p) -= 0x20;
            }
            break;
          case 0xcf: /* Greek & Coptic */
            if (*p == 0x8f)
              (*p) = 0x97;
            else if ((*p >= 0x98) && (*p <= 0xaf) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            else if (*p == 0xb4) {
              (*p) = 0x91;
            } else if (*p == 0xb7)
              (*p)++; /* Next char is lwr */
            else if (*p == 0xb9)
              (*p) = 0xb2;
            else if (*p == 0xba)
              (*p)++; /* Next char is lwr */
            else if (*p == 0xbd) {
              *pExtChar = 0xcd;
              (*p) = 0xbb;
            } else if (*p == 0xbe) {
              *pExtChar = 0xcd;
              (*p) = 0xbc;
            } else if (*p == 0xbf) {
              *pExtChar = 0xcd;
              (*p) = 0xbd;
            }
            break;
          case 0xd0: /* Cyrillic */
            if ((*p >= 0x80) && (*p <= 0x8f)) {
              *pExtChar = 0xd1;
              (*p) += 0x10;
            } else if ((*p >= 0x90) && (*p <= 0x9f))
              (*p) += 0x20; /* US ASCII shift */
            else if ((*p >= 0xa0) && (*p <= 0xaf)) {
              *pExtChar = 0xd1;
              (*p) -= 0x20;
            }
            break;
          case 0xd1: /* Cyrillic supplement */
            if ((*p >= 0xa0) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            break;
          case 0xd2: /* Cyrillic supplement */
            if (*p == 0x80)
              (*p)++; /* Next char is lwr */
            else if ((*p >= 0x8a) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            break;
          case 0xd3: /* Cyrillic supplement */
            if (*p == 0x80)
              (*p) = 0x8f;
            else if ((*p >= 0x81) && (*p <= 0x8e) && (*p % 2)) /* Odd */
              (*p)++; /* Next char is lwr */
            else if ((*p >= 0x90) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            break;
          case 0xd4: /* Cyrillic supplement & Armenian */
            if ((*p >= 0x80) && (*p <= 0xaf) && (!(*p % 2))) /* Even */
              (*p)++; /* Next char is lwr */
            else if ((*p >= 0xb1) && (*p <= 0xbf)) {
              *pExtChar = 0xd5;
              (*p) -= 0x10;
            }
            break;
          case 0xd5: /* Armenian */
            if ((*p >= 0x80) && (*p <= 0x8f)) {
              (*p) += 0x30;
            } else if ((*p >= 0x90) && (*p <= 0x96)) {
              *pExtChar = 0xd6;
              (*p) -= 0x10;
            }
            break;
          case 0xe1: /* Three byte code */
            pExtChar = p;
            p++;
            switch (*pExtChar) {
              case 0x82: /* Georgian asomtavruli */
                if ((*p >= 0xa0) && (*p <= 0xbf)) {
                  *pExtChar = 0x83;
                  (*p) -= 0x10;
                }
                break;
              case 0x83: /* Georgian asomtavruli */
                if (((*p >= 0x80) && (*p <= 0x85)) || (*p == 0x87) ||
                    (*p == 0x8d))
                  (*p) += 0x30;
                break;
              case 0x8e: /* Cherokee */
                if ((*p >= 0xa0) && (*p <= 0xaf)) {
                  *(p - 2) = 0xea;
                  *pExtChar = 0xad;
                  (*p) += 0x10;
                } else if ((*p >= 0xb0) && (*p <= 0xbf)) {
                  *(p - 2) = 0xea;
                  *pExtChar = 0xae;
                  (*p) -= 0x30;
                }
                break;
              case 0x8f: /* Cherokee */
                if ((*p >= 0x80) && (*p <= 0xaf)) {
                  *(p - 2) = 0xea;
                  *pExtChar = 0xae;
                  (*p) += 0x10;
                } else if ((*p >= 0xb0) && (*p <= 0xb5)) {
                  (*p) += 0x08;
                }
                /* 0xbe three byte small 0xe2 0xb1 0xa6 */
                break;
              case 0xb2: /* Georgian mtavruli */
                if (((*p >= 0x90) && (*p <= 0xba)) || (*p == 0xbd) ||
                    (*p == 0xbe) || (*p == 0xbf))
                  *pExtChar = 0x83;
                break;
              case 0xb8:                                         /* Latin ext */
                if ((*p >= 0x80) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                break;
              case 0xb9:                                         /* Latin ext */
                if ((*p >= 0x80) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                break;
              case 0xba:                                         /* Latin ext */
                if ((*p >= 0x80) && (*p <= 0x94) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                else if ((*p >= 0xa0) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                /* 0x9e Two byte small 0xc3 0x9f */
                break;
              case 0xbb:                                         /* Latin ext */
                if ((*p >= 0x80) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                break;
              case 0xbc: /* Greek ex */
                if ((*p >= 0x88) && (*p <= 0x8f))
                  (*p) -= 0x08;
                else if ((*p >= 0x98) && (*p <= 0x9d))
                  (*p) -= 0x08;
                else if ((*p >= 0xa8) && (*p <= 0xaf))
                  (*p) -= 0x08;
                else if ((*p >= 0xb8) && (*p <= 0xbf))
                  (*p) -= 0x08;
                break;
              case 0xbd: /* Greek ex */
                if ((*p >= 0x88) && (*p <= 0x8d))
                  (*p) -= 0x08;
                else if ((*p == 0x99) || (*p == 0x9b) || (*p == 0x9d) ||
                         (*p == 0x9f))
                  (*p) -= 0x08;
                else if ((*p >= 0xa8) && (*p <= 0xaf))
                  (*p) -= 0x08;
                break;
              case 0xbe: /* Greek ex */
                if ((*p >= 0x88) && (*p <= 0x8f))
                  (*p) -= 0x08;
                else if ((*p >= 0x98) && (*p <= 0x9f))
                  (*p) -= 0x08;
                else if ((*p >= 0xa8) && (*p <= 0xaf))
                  (*p) -= 0x08;
                else if ((*p >= 0xb8) && (*p <= 0xb9))
                  (*p) -= 0x08;
                else if ((*p >= 0xba) && (*p <= 0xbb)) {
                  *(p - 1) = 0xbd;
                  (*p) -= 0x0a;
                } else if (*p == 0xbc)
                  (*p) -= 0x09;
                break;
              case 0xbf: /* Greek ex */
                if ((*p >= 0x88) && (*p <= 0x8b)) {
                  *(p - 1) = 0xbd;
                  (*p) += 0x2a;
                } else if (*p == 0x8c)
                  (*p) -= 0x09;
                else if ((*p >= 0x98) && (*p <= 0x99))
                  (*p) -= 0x08;
                else if ((*p >= 0x9a) && (*p <= 0x9b)) {
                  *(p - 1) = 0xbd;
                  (*p) += 0x1c;
                } else if ((*p >= 0xa8) && (*p <= 0xa9))
                  (*p) -= 0x08;
                else if ((*p >= 0xaa) && (*p <= 0xab)) {
                  *(p - 1) = 0xbd;
                  (*p) += 0x10;
                } else if (*p == 0xac)
                  (*p) -= 0x07;
                else if ((*p >= 0xb8) && (*p <= 0xb9)) {
                  *(p - 1) = 0xbd;
                } else if ((*p >= 0xba) && (*p <= 0xbb)) {
                  *(p - 1) = 0xbd;
                  (*p) += 0x02;
                } else if (*p == 0xbc)
                  (*p) -= 0x09;
                break;
              default:
                break;
            }
            break;
          case 0xe2: /* Three byte code */
            pExtChar = p;
            p++;
            switch (*pExtChar) {
              case 0xb0: /* Glagolitic */
                if ((*p >= 0x80) && (*p <= 0x8f)) {
                  (*p) += 0x30;
                } else if ((*p >= 0x90) && (*p <= 0xae)) {
                  *pExtChar = 0xb1;
                  (*p) -= 0x10;
                }
                break;
              case 0xb1: /* Latin ext */
                switch (*p) {
                  case 0xa0:
                  case 0xa7:
                  case 0xa9:
                  case 0xab:
                  case 0xb2:
                  case 0xb5:
                    (*p)++; /* Next char is lwr */
                    break;
                  case 0xa2: /* Two byte small 0xc9 0xab */
                  case 0xa4: /* Two byte small 0xc9 0xbd */
                  case 0xad: /* Two byte small 0xc9 0x91 */
                  case 0xae: /* Two byte small 0xc9 0xb1 */
                  case 0xaf: /* Two byte small 0xc9 0x90 */
                  case 0xb0: /* Two byte small 0xc9 0x92 */
                  case 0xbe: /* Two byte small 0xc8 0xbf */
                  case 0xbf: /* Two byte small 0xc9 0x80 */
                    break;
                  case 0xa3:
                    *(p - 2) = 0xe1;
                    *(p - 1) = 0xb5;
                    *(p) = 0xbd;
                    break;
                  default:
                    break;
                }
                break;
              case 0xb2:                                         /* Coptic */
                if ((*p >= 0x80) && (*p <= 0xbf) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                break;
              case 0xb3:                                          /* Coptic */
                if (((*p >= 0x80) && (*p <= 0xa3) && (!(*p % 2))) /* Even */
                    || (*p == 0xab) || (*p == 0xad) || (*p == 0xb2))
                  (*p)++; /* Next char is lwr */
                break;
              case 0xb4: /* Georgian nuskhuri */
                if (((*p >= 0x80) && (*p <= 0xa5)) || (*p == 0xa7) ||
                    (*p == 0xad)) {
                  *(p - 2) = 0xe1;
                  *(p - 1) = 0x83;
                  (*p) += 0x10;
                }
                break;
              default:
                break;
            }
            break;
          case 0xea: /* Three byte code */
            pExtChar = p;
            p++;
            switch (*pExtChar) {
              case 0x99:                                         /* Cyrillic */
                if ((*p >= 0x80) && (*p <= 0xad) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                break;
              case 0x9a:                                         /* Cyrillic */
                if ((*p >= 0x80) && (*p <= 0x9b) && (!(*p % 2))) /* Even */
                  (*p)++; /* Next char is lwr */
                break;
              case 0x9c: /* Latin ext */
                if ((((*p >= 0xa2) && (*p <= 0xaf)) ||
                     ((*p >= 0xb2) && (*p <= 0xbf))) &&
                    (!(*p % 2))) /* Even */
                  (*p)++;        /* Next char is lwr */
                break;
              case 0x9d: /* Latin ext */
                if ((((*p >= 0x80) && (*p <= 0xaf)) && (!(*p % 2))) /* Even */
                    || (*p == 0xb9) || (*p == 0xbb) || (*p == 0xbe))
                  (*p)++; /* Next char is lwr */
                else if (*p == 0xbd) {
                  *(p - 2) = 0xe1;
                  *(p - 1) = 0xb5;
                  *(p) = 0xb9;
                }
                break;
              case 0x9e: /* Latin ext */
                if (((((*p >= 0x80) && (*p <= 0x87)) ||
                      ((*p >= 0x96) && (*p <= 0xa9)) ||
                      ((*p >= 0xb4) && (*p <= 0xbf))) &&
                     (!(*p % 2))) /* Even */
                    || (*p == 0x8b) || (*p == 0x90) || (*p == 0x92))
                  (*p)++; /* Next char is lwr */
                else if (*p == 0xb3) {
                  *(p - 2) = 0xea;
                  *(p - 1) = 0xad;
                  *(p) = 0x93;
                }
                /* case 0x8d: // Two byte small 0xc9 0xa5 */
                /* case 0xaa: // Two byte small 0xc9 0xa6 */
                /* case 0xab: // Two byte small 0xc9 0x9c */
                /* case 0xac: // Two byte small 0xc9 0xa1 */
                /* case 0xad: // Two byte small 0xc9 0xac */
                /* case 0xae: // Two byte small 0xc9 0xaa */
                /* case 0xb0: // Two byte small 0xca 0x9e */
                /* case 0xb1: // Two byte small 0xca 0x87 */
                /* case 0xb2: // Two byte small 0xca 0x9d */
                break;
              case 0x9f: /* Latin ext */
                if ((*p == 0x82) || (*p == 0x87) || (*p == 0x89) ||
                    (*p == 0xb5))
                  (*p)++; /* Next char is lwr */
                else if (*p == 0x84) {
                  *(p - 2) = 0xea;
                  *(p - 1) = 0x9e;
                  *(p) = 0x94;
                } else if (*p == 0x86) {
                  *(p - 2) = 0xe1;
                  *(p - 1) = 0xb6;
                  *(p) = 0x8e;
                }
                /* case 0x85: // Two byte small 0xca 0x82 */
                break;
              default:
                break;
            }
            break;
          case 0xef: /* Three byte code */
            pExtChar = p;
            p++;
            switch (*pExtChar) {
              case 0xbc: /* Latin fullwidth */
                if ((*p >= 0xa1) && (*p <= 0xba)) {
                  *pExtChar = 0xbd;
                  (*p) -= 0x20;
                }
                break;
              default:
                break;
            }
            break;
          case 0xf0: /* Four byte code */
            pExtChar = p;
            p++;
            switch (*pExtChar) {
              case 0x90:
                pExtChar = p;
                p++;
                switch (*pExtChar) {
                  case 0x90: /* Deseret */
                    if ((*p >= 0x80) && (*p <= 0x97)) {
                      (*p) += 0x28;
                    } else if ((*p >= 0x98) && (*p <= 0xa7)) {
                      *pExtChar = 0x91;
                      (*p) -= 0x18;
                    }
                    break;
                  case 0x92: /* Osage  */
                    if ((*p >= 0xb0) && (*p <= 0xbf)) {
                      *pExtChar = 0x93;
                      (*p) -= 0x18;
                    }
                    break;
                  case 0x93: /* Osage  */
                    if ((*p >= 0x80) && (*p <= 0x93))
                      (*p) += 0x28;
                    break;
                  case 0xb2: /* Old hungarian */
                    if ((*p >= 0x80) && (*p <= 0xb2))
                      *pExtChar = 0xb3;
                    break;
                  default:
                    break;
                }
                break;
              case 0x91:
                pExtChar = p;
                p++;
                switch (*pExtChar) {
                  case 0xa2: /* Warang citi */
                    if ((*p >= 0xa0) && (*p <= 0xbf)) {
                      *pExtChar = 0xa3;
                      (*p) -= 0x20;
                    }
                    break;
                  default:
                    break;
                }
                break;
              case 0x96:
                pExtChar = p;
                p++;
                switch (*pExtChar) {
                  case 0xb9: /* Medefaidrin */
                    if ((*p >= 0x80) && (*p <= 0x9f)) {
                      (*p) += 0x20;
                    }
                    break;
                  default:
                    break;
                }
                break;
              case 0x9E:
                pExtChar = p;
                p++;
                switch (*pExtChar) {
                  case 0xA4: /* Adlam */
                    if ((*p >= 0x80) && (*p <= 0x9d))
                      (*p) += 0x22;
                    else if ((*p >= 0x9e) && (*p <= 0xa1)) {
                      *(pExtChar) = 0xa5;
                      (*p) -= 0x1e;
                    }
                    break;
                  default:
                    break;
                }
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }
        pExtChar = 0;
      }
      p++;
    }
  }
  return pString;
}

}  // namespace ass