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
#include "xmlstream.h"
#include <map>
#include <list>
#include <vector>
#include <stack>
#include <expat.h>
using namespace std;

class SimpleNode {
friend class SimpleNodeParser;
private:
    SimpleNode() {};
public:
    string tagname;
    map<string, string> atts;
    list<SimpleNode> nodes;
    string text;

    SimpleNode(const string& xml);
};
class SimpleNodeParser {
    int depth;
    stack<SimpleNode*> nodes;
    XML_Parser parser;
    static void charactersSAXFunc(void* ctx, const char * ch, int len);
    static void startElementSAXFunc(void * ctx, const char * name,
        const char ** atts);
    static void endElementSAXFunc(void * ctx, const char * name);
public:
    SimpleNodeParser() {
        parser = XML_ParserCreate(NULL);
    }
    ~SimpleNodeParser() {
        XML_ParserFree(parser);
    }
    void parse(const string& xml, SimpleNode& node);
};
void
SimpleNodeParser::parse(const string& xml, SimpleNode& node) {
    depth = 0;
    nodes.push(&node);
    XML_ParserReset(parser, 0);
    XML_SetElementHandler(this->parser, startElementSAXFunc, endElementSAXFunc);
    XML_SetCharacterDataHandler(this->parser, charactersSAXFunc);
    XML_SetUserData(parser, this);
    if (XML_Parse(parser, xml.c_str(), xml.size(), true) == 0) {
        XML_Error e = XML_GetErrorCode(parser);
        printf("parsing error: %s\n", XML_ErrorString(e));
        // handle the error unless it is a tag mismatch in html
//        errorstring = XML_ErrorString(e);
//        error = stop = true;
//        wellformed = false;
    }
}
void
SimpleNodeParser::charactersSAXFunc(void* ctx, const char* ch, int len) {
    SimpleNodeParser* p = static_cast<SimpleNodeParser*>(ctx);
    p->nodes.top()->text.append(ch, len);
}
void
SimpleNodeParser::startElementSAXFunc(void* ctx, const char* name,
        const char** atts) {
    SimpleNodeParser* p = static_cast<SimpleNodeParser*>(ctx);
    SimpleNode* node = p->nodes.top();

    // if this is not the root node, add it to the stack and to the parent node
    if (p->depth > 0) {
        SimpleNode emptynode;
        node->nodes.push_back(emptynode);
        node = &*(node->nodes.rbegin());
        p->nodes.push(node);
    }

    node->tagname = name;
    printf("%s\n", name);
    while (*atts) {
        node->atts[*atts] = *(atts+1);
        atts += 2;
    }
    p->depth++;
}
void
SimpleNodeParser::endElementSAXFunc(void* ctx, const char* name) {
    SimpleNodeParser* p = static_cast<SimpleNodeParser*>(ctx);
    p->nodes.pop();
    p->depth--;
}
SimpleNode::SimpleNode(const string& xml) {
    SimpleNodeParser parser;
    parser.parse(xml, *this);
}

class XMLStream::Private {
private:
public:
    const SimpleNode root;
    const SimpleNode* activeNode;
    Private(const string& xml) :root(xml), activeNode(&root) {
    }
};
XMLStream::XMLStream(const string& xml) {
    p = new Private(xml);
}
XMLStream::~XMLStream() {
    delete p;
}
void
XMLStream::setFromAttribute(bool& v, const char* name) {
    map<string, string>::const_iterator i = p->activeNode->atts.find(name);
    if (i != p->activeNode->atts.end()
            && (i->second == "true" || i->second == "1")) {
        v = true;
    } else {
        v = false;
    }
}
void
XMLStream::setFromAttribute(int& v, const char* name) {
    map<string, string>::const_iterator i = p->activeNode->atts.find(name);
    if (i == p->activeNode->atts.end()) {
        v = 0;
    } else {
        v = atoi(i->second.c_str());
    }
}
void
XMLStream::setFromAttribute(string& v, const char* name) {
    map<string, string>::const_iterator i = p->activeNode->atts.find(name);
    if (i == p->activeNode->atts.end()) {
        v = "";
    } else {
        v = i->second;
    }
}
const string&
XMLStream::getTagName() const {
    return activeNode().tagname;
}
XMLStream&
operator>>(XMLStream& in, bool& e) {
    e = in.activeNode().text == "true" || in.activeNode().text == "1";
    return in;
}
XMLStream&
operator>>(XMLStream& in, int& e) {
    e = atoi(in.activeNode().text.c_str());
    return in;
}
XMLStream&
operator>>(XMLStream& in, string& e) {
    e = in.activeNode().text;
    return in;
}
const SimpleNode&
XMLStream::activeNode() const {
    return *p->activeNode;
}
