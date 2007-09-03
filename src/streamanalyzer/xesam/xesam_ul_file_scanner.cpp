/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Flavio Castelli <flavio.castelli@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "xesam_ul_file_scanner.h"
#include "xesam_ul_parser.hh"

using namespace std;

XesamUlFileScanner::XesamUlFileScanner(string filename)
{
  m_ifstream.open(filename.c_str());
  m_modifier = false;
  m_quotmarkClosed = false;
  m_quotmarkCount = 0;
}

XesamUlFileScanner::~XesamUlFileScanner()
{
  if (m_ifstream.is_open())
    m_ifstream.close();
}

int XesamUlFileScanner::yylex(YYSTYPE* yylval)
{
  string read;
  char ch;
  
  if (!m_ifstream.is_open())
    return -1;
  else if (m_ifstream.eof()) {
    return yy::xesam_ul_parser::token::END;
  }
  
  do {
    m_ifstream.get(ch);

    if (m_ifstream.eof()) {
      m_modifier = false;
      return yy::xesam_ul_parser::token::END;
    }

    if ((m_modifier) && (isspace(ch) != 0))
      m_modifier = false;
    
  } while (isspace (ch) != 0);
  
  if (isalnum(ch) != 0) {

    // quotation mark closed and no space after it--> read modifiers
    if ((m_quotmarkClosed) && (m_modifier)) {
      if ( (ch == 'b') /*Boost*/ ||
           (ch == 'c') /*Case sensitive*/ ||
           (ch == 'C') /*Case insensitive*/ ||
           (ch == 'd') /*Diacritic sensitive*/ ||
           (ch == 'D') /*Diacritic insensitive*/ ||
           (ch == 'e') /*Exact match. Short for cdl*/ ||
           (ch == 'f') /*Fuzzy search*/ ||
           (ch == 'l') /*Don't do stemming*/ ||
           (ch == 'L') /*Do stemming*/ ||
           (ch == 'o') /*Ordered words*/ ||
           (ch == 'p') /*Proximity search (suggested default: 10)*/ ||
           (ch == 'r') /*The phrase is a regular expression*/ ||
           (ch == 's') /*Sloppy search*/ ||
           (ch == 'w') /*Word based matching*/ )
      {
        *yylval = ch;
        return yy::xesam_ul_parser::token::MODIFIER;
      }
      else {
        //error!
        m_modifier = false;
        std::cerr << "Error: unknown modifier: |" << ch << "|" << endl;

        return -1;
      }
    }
    

    read = ch;
    while (isalnum (m_ifstream.peek()) != 0) {
      m_ifstream.get(ch);
      read += ch;
    }

    //yy::xesam_ul_parser::token::KEYWORD (type)|(format)|(creator)|(tag)|(size)
    if ( (read.compare("type") == 0) ||
         (read.compare("format") == 0) ||
         (read.compare("creator") == 0) ||
         (read.compare("tag") == 0) ||
         (read.compare("size") == 0))
    {
      // set yylval
      *yylval = read;
      return yy::xesam_ul_parser::token::KEYWORD;
    }
    else if ((read.compare("and") == 0) ||
              (read.compare("AND") == 0))
    {
      //yy::xesam_ul_parser::token::AND (and)|(AND)
      // set yylval
           *yylval = read;
           return yy::xesam_ul_parser::token::AND;
    }
    else if ((read.compare("or") == 0) ||
              (read.compare("OR") == 0))
    {
      //yy::xesam_ul_parser::token::OR (or)|(OR)
      // set yylval
      *yylval = read;
      return yy::xesam_ul_parser::token::OR;
    }
    else {
      // read also symbols composing a WORD (except for ")
      char next = m_ifstream.peek();
      while ((isspace (next) == 0) && (next != '"')) {
        m_ifstream.get(ch);
        read += ch;
        next = m_ifstream.peek();
      }
      
      *yylval = read;
      return yy::xesam_ul_parser::token::WORD;
    }
  }
  else if ((ch == '=') || (ch == '<') || (ch == '>') || (ch == ':')) {
    // yy::xesam_ul_parser::token::RELATION (=)|(<=)|(>=)|(<)|(>)|(:)
    read = ch;

    // we've to look after <= and >=
    if (((ch == '<') || (ch == '>')) && (m_ifstream.peek() == '=')) {
      m_ifstream.get(ch);
      read += ch;
    }

    // set yylval
    *yylval = read;
    return yy::xesam_ul_parser::token::RELATION;
  }
  else if ((ch == '"')) {
    // yy::xesam_ul_parser::token::QUOTMARK (")
    read = ch;
    // set yylval
    *yylval = "";
    m_quotmarkCount++;
    if (m_quotmarkCount %2 == 0) {
      m_quotmarkClosed = true;
      m_modifier = true;
      m_quotmarkCount = 0;
      return yy::xesam_ul_parser::token::QUOTMARKCLOSE;
    }
    else {
      m_quotmarkClosed = false;
      m_modifier = false;
      return yy::xesam_ul_parser::token::QUOTMARKOPEN;
    }
  }
  else if ((ch == '-')) {
    // yy::xesam_ul_parser::token::MINUS (-)
    read = ch;
    // set yylval
    *yylval = read;
    return yy::xesam_ul_parser::token::MINUS;
  }
  else if ((ch == '&') && (m_ifstream.peek() == '&')) {
    // yy::xesam_ul_parser::token::AND (&&)
    m_ifstream.get(ch);
    read = ch + ch;
    // set yylval
    *yylval = read;
    return yy::xesam_ul_parser::token::AND;
  }
  else if ((ch == '|') && (m_ifstream.peek() == '|')) {
    // yy::xesam_ul_parser::token::OR (||)
    m_ifstream.get(ch);
    read = ch + ch;
    // set yylval
    *yylval = read;
    return yy::xesam_ul_parser::token::OR;
  }

  //unknown char!
  //TODO yyerror?
  return -1;
}