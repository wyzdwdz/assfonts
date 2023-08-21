//
// One header file iconv wrapper for C++11 2023-01-11.12
// https://github.com/trueroad/iconv_wrapper
//
// Copyright (C) 2016, 2019, 2022, 2023 Masamichi Hosoda. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#ifndef INCLUDE_GUARD_ICONV_WRAPPER_H
#define INCLUDE_GUARD_ICONV_WRAPPER_H

#include <iconv.h>
#include <string>
#include <system_error>

namespace iconv_wrapper
{
  class iconv
  {
  public:
    // Open
    void open (const std::string &fromcode, const std::string &tocode);

    // Close
    void close () noexcept;

    // Convert
    std::string convert (const std::string &in);

    // Convert (with getting the incomplete input / output
    // when an exception occurs)
    std::string &convert (const std::string &in,
                          std::string::size_type *pinpos,
                          std::string *pout);

    // Get initial sequence
    std::string get_initial_sequence (void);

    // Get initial sequence
    std::string &get_initial_sequence (std::string *pout);

    // Reset conversion state
    void reset (void) const noexcept;

    // Constructor / destructor
    iconv () = default;

    iconv (const std::string &fromcode, const std::string &tocode)
    {
      open (fromcode, tocode);
    }

    ~iconv () noexcept
    {
      close ();
    }

    // Enable move
    iconv (iconv &&) = default;
    iconv& operator = (iconv &&) = default;

  private:
    // Internal class
    // There is two different implements for the second argument of iconv ().
    //   `const char**' and `char **'
    // This class is for automatic type cast switching.
    class iconv_const_cast
    {
    public:
      iconv_const_cast (const char **in) noexcept:
        t (in)
      {
      }
      operator char** () const noexcept
      {
        return const_cast<char**>(t);
      }
      operator const char ** () const noexcept
      {
        return t;
      }
    private:
      const char ** t;
    };

    // Internal function
    void do_iconv (std::string *pout, const char *inbuf, size_t *pinleft,
                   std::string::size_type *pinpos = nullptr);

    // Disable copy
    // Since iconv_t cannot duplicate.
    iconv (iconv const &) = delete;
    iconv& operator = (iconv const &) = delete;

    // Const
    // Here is C cast instead of C++ cast.
    // C++ reinterpret_cast (pointer) and static_cast (integer) cannot be used.
    // Since it depends on the implementation of iconv_t.
    const iconv_t invalid_cd = (iconv_t)-1;

    // Conversion descriptor
    iconv_t convdesc = invalid_cd;
  };

  inline void iconv::open (const std::string &fromcode,
                           const std::string &tocode)
  {
    if (convdesc != invalid_cd)
      {
        close ();
      }
    convdesc = iconv_open (tocode.c_str (), fromcode.c_str ());
    if (convdesc == invalid_cd)
      {
        throw std::system_error (errno, std::system_category ());
      }
  }

  inline void iconv::close () noexcept
  {
    if (convdesc != invalid_cd)
      {
        iconv_close (convdesc);
        convdesc = invalid_cd;
      }
  }

  inline std::string iconv::convert (const std::string &in)
  {
    std::string out (in.size (), '\0');
    convert (in, nullptr, &out);
    return out;
  }

  inline std::string &iconv::convert (const std::string &in,
                                      std::string::size_type *pinpos,
                                      std::string *pout)
  {
    size_t inleft {in.size ()};
    if (inleft)
      do_iconv (pout, &in.at (0), &inleft, pinpos);
    else
      pout->clear();
    return *pout;
  }

  inline std::string iconv::get_initial_sequence (void)
  {
    std::string out (1, '\0');
    get_initial_sequence (&out);
    return out;
  }

  inline std::string &iconv::get_initial_sequence (std::string *pout)
  {
    do_iconv (pout, nullptr, nullptr);
    return *pout;
  }

  inline void iconv::reset (void) const noexcept
  {
    ::iconv (convdesc, nullptr, nullptr, nullptr, nullptr);
  }

  inline void iconv::do_iconv (std::string *pout,
                               const char *inbuf, size_t *pinleft,
                               std::string::size_type *pinpos)
  {
    if (pout->empty ())
      {
        pout->resize (1);
      }
    const char *inbuf_tmp {inbuf};
    char *outbuf {&pout->at (0)};
    size_t outleft {pout->size ()};
    size_t s;

    while ((s = ::iconv (convdesc,
                         iconv_const_cast(&inbuf_tmp), pinleft,
                         &outbuf, &outleft)) == static_cast<size_t>(-1))
      {
        if (errno != E2BIG)
          {
            if (pinpos && inbuf)
              {
                *pinpos = (inbuf_tmp - inbuf);
              }
            pout->resize (outbuf - &pout->at (0));
            throw std::system_error (errno, std::system_category ());
          }
        std::string::size_type pos = (outbuf - &pout->at (0));
        pout->resize (pout->size () * 2);
        outbuf = &pout->at (pos);
        outleft = pout->size () - pos;
      }
    pout->resize (outbuf - &pout->at (0));
  }
}

#endif  // INCLUDE_GUARD_ICONV_WRAPPER_H
