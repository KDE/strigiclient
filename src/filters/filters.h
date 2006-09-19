/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Flavio Castelli <flavio.castelli@gmail.com>
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
 
#include <string>

#ifndef FILTERS_H
#define FILTERS_H

class Filter
{
    public:
        Filter(const std::string& rule) :m_rule(rule) {}
        virtual ~Filter() {}
        
        virtual bool match(const std::string& text) = 0;
        const std::string& rule() const { return m_rule; }
        virtual int rtti() const = 0;
        
    protected:
        const std::string m_rule;
};

class PatternFilter : public Filter
{
    public:
        PatternFilter(const std::string& rule) : Filter (rule) {}
        ~PatternFilter() {}
        
        enum {RTTI = 1};
        
        bool match(const std::string& text);
        int rtti() const { return RTTI; }
};

class PathFilter : public Filter
{
    public:
        PathFilter(const std::string& rule) : Filter (rule) {}
        ~PathFilter() {}
        
        enum {RTTI = 2};
        
        bool match (const std::string& text);
        int rtti() const { return RTTI; }
};

#endif