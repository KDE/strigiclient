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
#include "combinedindexmanager.h"
#include "strigiconfig.h"

#include "grepindexmanager.h"
#ifdef HAVE_CLUCENE
#include "cluceneindexmanager.h"
#endif
#ifdef HAVE_XAPIAN
#include "xapianindexmanager.h"
#endif
#ifdef HAVE_ESTRAIER
#include "estraierindexmanager.h"
#endif
#ifdef HAVE_SQLITE
#include "sqliteindexmanager.h"
#endif

#include "tssptr.h"
#include "query.h"
#include "indexeddocument.h"
#include "indexreader.h"
#include <string>
#include <map>
using namespace std;
using namespace Strigi;

map<string, IndexManager*(*)(const char*)>
CombinedIndexManager::factories() {
    map<string, IndexManager*(*)(const char*)> factories;
#ifdef HAVE_ESTRAIER
    factories["estraier"] = createEstraierIndexManager;
#endif
#ifdef HAVE_CLUCENE
    factories["clucene"] = createCLuceneIndexManager;
#endif
#ifdef HAVE_XAPIAN
    factories["xapian"] = createXapianIndexManager;
#endif
#ifdef HAVE_SQLITE
    factories["sqlite"] = createSqliteIndexManager;
#endif
    factories["grep"] = createGrepIndexManager;
    return factories;
}

vector<string>
CombinedIndexManager::backEnds() {
    vector<string> v;
    map<string, IndexManager*(*)(const char*)> f = factories();
    map<string, IndexManager*(*)(const char*)>::const_iterator i;
    for (i=f.begin(); i!=f.end(); ++i) {
        v.push_back(i->first);
    }
    return v;
}

class CombinedIndexReader : public IndexReader {
private:
    CombinedIndexManager* m;
public:
    CombinedIndexReader(CombinedIndexManager* manager) :m(manager){}
    int32_t countHits(const Query& query);
    vector<IndexedDocument> query(const Query& q);
    map<string, time_t> files(char depth);
    int32_t countDocuments();
    int32_t countWords();
    int64_t indexSize();
    int64_t documentId(const string& uri);
    time_t mTime(int64_t docid);
    vector<pair<string,uint32_t> > histogram(
            const string& query, const string& fieldname,
            const string& labeltype);
    vector<string> fieldNames();
    int32_t countKeywords(const string& keywordprefix,
        const vector<string>& fieldnames);
    vector<string> keywords(const string& keywordmatch,
        const vector<string>& fieldnames,
        uint32_t max, uint32_t offset);
};

class CombinedIndexManager::Private {
public:
    StrigiMutex lock;
    CombinedIndexReader reader;
    string writermanagerdir;
    IndexManager* writermanager;
    map<string, TSSPtr<IndexManager> > readmanagers;

    Private(CombinedIndexManager* m);
    ~Private();
};
CombinedIndexManager::Private::Private(CombinedIndexManager* m) :reader(m) {
    writermanager = 0;
}
CombinedIndexManager::Private::~Private() {
    if (writermanager) {
        delete writermanager;
    }
}
CombinedIndexManager::CombinedIndexManager(const string& type,
        const string& indexdir) :p(new CombinedIndexManager::Private(this)) {
    // determine the right index manager
    map<string, IndexManager*(*)(const char*)> l_factories = factories();
    map<string, IndexManager*(*)(const char*)>::const_iterator f
        = l_factories.find(type);
    if (f == l_factories.end()) {
        f = l_factories.begin();
    }
    p->writermanager = f->second(indexdir.c_str());
}
CombinedIndexManager::~CombinedIndexManager() {
    delete p;
}
IndexReader*
CombinedIndexManager::indexReader() {
    return &(p->reader);
}
IndexWriter*
CombinedIndexManager::indexWriter() {
    return p->writermanager->indexWriter();
}
void
CombinedIndexManager::addReadonlyIndex(const string& indexdir,
        const string& type) {
    removeReadonlyIndex(indexdir);
    // determine the right index manager
    map<string, IndexManager*(*)(const char*)> l_factories = factories();
    map<string, IndexManager*(*)(const char*)>::const_iterator f
        = l_factories.find(type);
    if (f == l_factories.end()) {
        return;
    }
    IndexManager* im = f->second(indexdir.c_str());
    STRIGI_MUTEX_LOCK(&p->lock.lock);
    p->readmanagers[indexdir] = im;
    STRIGI_MUTEX_UNLOCK(&p->lock.lock);
}
void
CombinedIndexManager::removeReadonlyIndex(const string& indexdir) {
    STRIGI_MUTEX_LOCK(&p->lock.lock);
    p->readmanagers.erase(indexdir);
    STRIGI_MUTEX_UNLOCK(&p->lock.lock);
}
int32_t
CombinedIndexReader::countHits(const Query& query) {
    int32_t c = m->p->writermanager->indexReader()->countHits(query);
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->indexReader()->countHits(query);
    }
    return c;
}
vector<IndexedDocument>
CombinedIndexReader::query(const Query& q) {
    /** TODO merge the result documents by score **/
    vector<IndexedDocument> v = m->p->writermanager->indexReader()
        ->query(q);
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        v = i->second->indexReader()->query(q);
    }
    return v;
}
map<string, time_t>
CombinedIndexReader::files(char depth) {
    map<string, time_t> files = m->p->writermanager->indexReader()
        ->files(depth);
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        map<string, time_t> af(i->second->indexReader()->files(depth));
        map<string, time_t>::const_iterator j;
        for (j = af.begin(); j != af.end(); ++j) {
            // insert entry if it does not yet occur in the map
            files.insert(*j);
        }
    }
    return files;
}
int32_t
CombinedIndexReader::countDocuments() {
    int32_t c = m->p->writermanager->indexReader()->countDocuments();
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->indexReader()->countDocuments();
    }
    return c;
}
int32_t
CombinedIndexReader::countWords() {
    int32_t c = m->p->writermanager->indexReader()->countWords();
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->indexReader()->countWords();
    }
    return c;
}
int64_t
CombinedIndexReader::indexSize() {
    int64_t c = m->p->writermanager->indexReader()->indexSize();
    STRIGI_MUTEX_LOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> > f = m->p->readmanagers;
    STRIGI_MUTEX_UNLOCK(&m->p->lock.lock);
    map<string, TSSPtr<IndexManager> >::const_iterator i;
    for (i = f.begin(); i != f.end(); ++i) {
        c += i->second->indexReader()->indexSize();
    }
    return c;
}
vector<pair<string,uint32_t> >
CombinedIndexReader::histogram(const string& query, const string& fieldname,
        const string& labeltype){
    return m->p->writermanager->indexReader()->histogram(query,
        fieldname, labeltype);
}
int64_t
CombinedIndexReader::documentId(const string& uri) {
    return m->p->writermanager->indexReader()->documentId(uri);
}
time_t
CombinedIndexReader::mTime(int64_t docid) {
    return m->p->writermanager->indexReader()->mTime(docid);
}
vector<string>
CombinedIndexReader::fieldNames() {
    return m->p->writermanager->indexReader()->fieldNames();
}
int32_t
CombinedIndexReader::countKeywords(const string& keywordprefix,
        const vector<string>& fieldnames) {
    return m->p->writermanager->indexReader()->countKeywords(keywordprefix,
        fieldnames);
}
vector<string>
CombinedIndexReader::keywords(const string& keywordmatch,
        const vector<string>& fieldnames, uint32_t max, uint32_t offset) {
    return m->p->writermanager->indexReader()->keywords(keywordmatch,
        fieldnames, max, offset);
}
