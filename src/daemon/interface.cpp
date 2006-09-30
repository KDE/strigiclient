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
#include "interface.h"
#include "indexreader.h"
#include "indexmanager.h"
#include "indexwriter.h"
#include "indexscheduler.h"
#include "eventlistener.h"
#include "filtermanager.h"
#include "streamindexer.h"
#include "stringreader.h"
#include "query.h"
#include <sstream>
using namespace std;
using namespace jstreams;

Interface::Interface(IndexManager& m, IndexScheduler& s)
    :manager(m),
     scheduler(s)
{
    eventListener = NULL;
    filterManager = NULL;
}

void Interface::setEventListener (EventListener* eListener) {
    eventListener = eListener;
}

void Interface::setFilterManager (FilterManager* fManager) {
    filterManager = fManager;
}

int
Interface::countHits(const string& query) {
    Query q(query, -1, 0);
    int count = manager.getIndexReader()->countHits(q);
    return count;
}
ClientInterface::Hits
Interface::getHits(const string& query, int max, int off) {
    Query q(query, max, off);
    Hits hits;
    hits.hits = manager.getIndexReader()->query(q);
    // highlight the hits in the results
    vector<IndexedDocument>::iterator i;
    for (i = hits.hits.begin(); i != hits.hits.end(); ++i) {
        i->fragment = q.highlight(i->fragment);
    }
    return hits;
}
map<string, string>
Interface::getStatus() {
    map<string,string> status;
    status["Status"]=scheduler.getStateString();
    ostringstream out;
    out << scheduler.getQueueSize();
    status["Documents in queue"]= out.str();
    out.str("");
    IndexReader* reader = manager.getIndexReader();
    out << reader->countDocuments();
    status["Documents indexed"]= out.str();
    out.str("");
    out << reader->countWords();
    status["Unique words indexed"] = out.str();
    out.str("");
    out << reader->getIndexSize()/1024/1024;
    status["Index size"] = out.str()+" MB";
    return status;
}
string
Interface::stopDaemon() {
    active = false;
    scheduler.stop();
    return "";
}
string
Interface::startIndexing() {
    scheduler.startIndexing();
    return "";
}
string
Interface::stopIndexing() {
    scheduler.stopIndexing();
    return "";
}
set<string>
Interface::getIndexedDirectories() {
    return scheduler.getIndexedDirectories();
}
string
Interface::setIndexedDirectories(set<string> dirs) {
    if (eventListener != NULL)
        eventListener->setIndexedDirectories( dirs);

    scheduler.setIndexedDirectories(dirs);
    return "";
}
multimap<int,string>
Interface::getFilteringRules() {
    if (filterManager != NULL)
        return filterManager->getFilteringRules();
    else
    {
        multimap<int,string> empty;
        return empty;
    }
}
void
Interface::setFilteringRules(const multimap<int,string>& rules) {
    if (filterManager != NULL)
        filterManager->setFilteringRules (rules);
}
set<string>
Interface::getIndexedFiles() {
    map <string, time_t> indexedfiles = manager.getIndexReader()->getFiles(0);
    set <string> r;
    
    for (map<string, time_t>::iterator iter = indexedfiles.begin(); iter != indexedfiles.end(); iter++)
        r.insert (iter->first);
    
    return r;
}
void
Interface::indexFile(const std::string &path, time_t mtime,
        const std::vector<char>& content) {
    // TODO if the file is already there, remove it first
    IndexWriter* writer = manager.getIndexWriter();
    vector<string> paths;
    paths.push_back(path);
    writer->deleteEntries(paths);
    StreamIndexer streamindexer(writer);
    StringReader<char> sr(&content[0], content.size(), false);
    char r = streamindexer.analyze(path, mtime, &sr, 0);
}
