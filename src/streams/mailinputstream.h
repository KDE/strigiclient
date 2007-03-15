/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Jos van den Oever <jos@vandenoever.info>
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
#ifndef MAILINPUTSTREAM_H
#define MAILINPUTSTREAM_H

#include "streams_export.h"
#include "substreamprovider.h"
#include <stack>

namespace jstreams {
class SubInputStream;
class StringTerminatedSubStream;

/**
 * This is an implementation for handling email streams as
 * archives. It allows one to read the email body and email attachements as
 * streams.
 **/
class STREAMS_EXPORT MailInputStream : public SubStreamProvider {
private:
    int64_t nextLineStartPosition;
    // variables that record the current read state
    int32_t entrynumber;
    int linenum;
    int maxlinesize;
    const char* linestart;
    const char* lineend;

    StringTerminatedSubStream* substream;
    std::string subject;
    std::string contenttype;
    std::string contenttransferencoding;
    std::string contentdisposition;

    std::stack<std::string> boundary;

    void readHeaderLine();
    void readHeader();
    void scanBody();
    void handleHeaderLine();
    bool handleBodyLine();
    bool lineIsEndOfBlock();
    bool checkHeaderLine() const;
    void clearHeaders();
    void ensureFileName();
    std::string getValue(const char* n, const std::string& headerline) const;
public:
    explicit MailInputStream(StreamBase<char>* input);
    ~MailInputStream();
    StreamBase<char>* nextEntry();
    static bool checkHeader(const char* data, int32_t datasize);
    static SubStreamProvider* factory(StreamBase<char>* input) {
        return new MailInputStream(input);
    }
    const std::string& getSubject() { return subject; }
    const std::string& getContentType() { return contenttype; }
};

} // end namespace jstreams

#endif
