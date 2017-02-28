// A Bison parser, made by GNU Bison 3.0.4.

// Locations for Bison parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

/**
 ** \file ./src/assembler/location.hh
 ** Define the Assembler::location class.
 */

#pragma once

#define YY_NULLPTR nullptr

#define TRACK_LOCATIONS 0

#if TRACK_LOCATIONS
#define LOCATION_DEBUG(x, y...) printf(x, y);
#else
#define LOCATION_DEBUG(x...) do { } while(false)
#endif

#include "utils.h"

namespace Assembler {
  
  class position
  {
  public:
    std::string* filename;
    u32 line;
    u32 column;
    
    explicit position (std::string* f = YY_NULLPTR, u32 l = 1, u32 c = 1) : filename(f), line(l), column(c)
    {
      LOCATION_DEBUG("Built position %u,%u\n", l, c);
    }
    
    void initialize (std::string* fn = YY_NULLPTR, u32 l = 1u, u32 c = 1u)
    {
      LOCATION_DEBUG("initialized position %u,%u\n", l, c);

      filename = fn;
      line = l;
      column = c;
    }
    
    /// (line related) Advance to the COUNT next lines.
    void lines (int count = 1)
    {
      if (count)
      {
        column = 1u;
        line = add_ (line, count, 1);
      }
    }
    
    /// (column related) Advance to the COUNT next columns.
    void columns (int count = 1)
    {
      column = add_ (column, count, 1);
    }
    
    inline position& operator+= (int width)
    {
      columns(width);
      return *this;
    }
    
    inline position operator+ (int width)
    {
      return *this += width;
    }
  
  private:
    /// Compute max(min, lhs+rhs) (provided min <= lhs).
    static unsigned int add_ (unsigned int lhs, int rhs, unsigned int min)
    {
      return (0 < rhs || -static_cast<unsigned int>(rhs) < lhs
              ? rhs + lhs
              : min);
    }
  };

  /// Subtract \a width columns, in place.
  inline position&
  operator-= (position& res, int width)
  {
    return res += -width;
  }
  
  /// Subtract \a width columns.
  inline position
  operator- (position res, int width)
  {
    return res -= width;
  }
  
  /// Compare two position objects.
  inline bool
  operator== (const position& pos1, const position& pos2)
  {
    return (pos1.line == pos2.line
            && pos1.column == pos2.column
            && (pos1.filename == pos2.filename
                || (pos1.filename && pos2.filename
                    && *pos1.filename == *pos2.filename)));
  }
  
  /// Compare two position objects.
  inline bool
  operator!= (const position& pos1, const position& pos2)
  {
    return !(pos1 == pos2);
  }
  
  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param pos a reference to the position to redirect
   */
  template <typename YYChar>
  inline std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const position& pos)
  {
    if (pos.filename)
      ostr << *pos.filename << ':';
    return ostr << pos.line << '.' << pos.column;
  }

  
  /// Abstract a location.
  class location
  {
  public:
    
    /// Construct a location from \a b to \a e.
    location (const position& b, const position& e)
    : begin (b)
    , end (e)
    {
      LOCATION_DEBUG("Built location %u,%u  %u,%u\n", b.line, b.column, e.line, e.column);
    }
    
    /// Construct a 0-width location in \a p.
    explicit location (const position& p = position ())
    : begin (p)
    , end (p)
    {
      LOCATION_DEBUG("Built zero width location %u,%u\n", p.line, p.column);
    }
    
    /// Construct a 0-width location in \a f, \a l, \a c.
    explicit location (std::string* f,
                       unsigned int l = 1u,
                       unsigned int c = 1u)
    : begin (f, l, c)
    , end (f, l, c)
    {
    }
    
    
    /// Initialization.
    void initialize (std::string* f = YY_NULLPTR,
                     unsigned int l = 1u,
                     unsigned int c = 1u)
    {
      begin.initialize (f, l, c);
      end = begin;
    }
    
    /** \name Line and Column related manipulators
     ** \{ */
  public:
    /// Reset initial location to final location.
    void step ()
    {
      begin = end;
    }
    
    /// Extend the current location to the COUNT next columns.
    void columns (int count = 1)
    {
      end += count;
    }
    
    /// Extend the current location to the COUNT next lines.
    void lines (int count = 1)
    {
      LOCATION_DEBUG("Extended location lines %u ", end.line);
      end.lines (count);
      LOCATION_DEBUG("to %u\n", end.line);
    }
    /** \} */
    
    
  public:
    /// Beginning of the located region.
    position begin;
    /// End of the located region.
    position end;
  };
  
  /// Join two locations, in place.
  inline location& operator+= (location& res, const location& end)
  {
    res.end = end.end;
    return res;
  }
  
  /// Join two locations.
  inline location operator+ (location res, const location& end)
  {
    return res += end;
  }
  
  /// Add \a width columns to the end position, in place.
  inline location& operator+= (location& res, int width)
  {
    res.columns (width);
    return res;
  }
  
  /// Add \a width columns to the end position.
  inline location operator+ (location res, int width)
  {
    return res += width;
  }
  
  /// Subtract \a width columns to the end position, in place.
  inline location& operator-= (location& res, int width)
  {
    return res += -width;
  }
  
  /// Subtract \a width columns to the end position.
  inline location operator- (location res, int width)
  {
    return res -= width;
  }
  
  /// Compare two location objects.
  inline bool
  operator== (const location& loc1, const location& loc2)
  {
    return loc1.begin == loc2.begin && loc1.end == loc2.end;
  }
  
  /// Compare two location objects.
  inline bool
  operator!= (const location& loc1, const location& loc2)
  {
    return !(loc1 == loc2);
  }
  
  /** \brief Intercept output stream redirection.
   ** \param ostr the destination output stream
   ** \param loc a reference to the location to redirect
   **
   ** Avoid duplicate information.
   */
  template <typename YYChar>
  inline std::basic_ostream<YYChar>&
  operator<< (std::basic_ostream<YYChar>& ostr, const location& loc)
  {
    unsigned int end_col = 0 < loc.end.column ? loc.end.column - 1 : 0;
    ostr << loc.begin;
    if (loc.end.filename
        && (!loc.begin.filename
            || *loc.begin.filename != *loc.end.filename))
      ostr << '-' << loc.end.filename << ':' << loc.end.line << '.' << end_col;
    else if (loc.begin.line < loc.end.line)
      ostr << '-' << loc.end.line << '.' << end_col;
    else if (loc.begin.column < end_col)
      ostr << '-' << end_col;
    return ostr;
  }
  
} // Assembler
