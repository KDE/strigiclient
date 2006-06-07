#include "cluceneindexreader.h"
#include "cluceneindexmanager.h"
#include <CLucene/clucene-config.h>
#include <CLucene.h>

using lucene::search::Hits;
using lucene::search::IndexSearcher;
using lucene::document::Document;
using lucene::document::Field;
using lucene::index::Term;
using lucene::search::TermQuery;
using lucene::search::BooleanQuery;
using namespace jstreams;

CLuceneIndexReader::~CLuceneIndexReader() {
}

const char*
CLuceneIndexReader::mapId(const std::string& id) {
    if (id == "") return "content";
    return id.c_str();
}
Term*
CLuceneIndexReader::createTerm(const string& name, const string& value) {
#ifndef _CL_HAVE_WCSLEN
    return  Term(name.c_str(), value.c_str());
#else 
#endif
    TCHAR* n = new TCHAR[name.length()+1];
    STRCPY_AtoT(n, name.c_str(), name.length()+1);
    TCHAR* v = new TCHAR[value.length()+1];
    STRCPY_AtoT(v, value.c_str(), value.length()+1);
    Term* t = _CLNEW Term(n, v);
    delete [] n;
    delete [] v;
    return t;
}
void
CLuceneIndexReader::createBooleanQuery(const Query& query, BooleanQuery& bq) {
    // add the attributes
    const map<string, set<string> >& includes = query.getIncludes();
    map<string, set<string> >::const_iterator i;
    set<string>::const_iterator j;
    for (i = includes.begin(); i != includes.end(); ++i) {
        string id = mapId(i->first);
        for (j = i->second.begin(); j != i->second.end(); ++j) {
            Term* t = createTerm(mapId(i->first), *j);
            TermQuery* tq = _CLNEW TermQuery(t);
            _CLDECDELETE(t);
            bq.add(tq, true, true, false);
        }
    }
    const map<string, set<string> >& excludes = query.getExcludes();
    for (i = excludes.begin(); i != excludes.end(); ++i) {
        string id = mapId(i->first);
        for (j = i->second.begin(); j != i->second.end(); ++j) {
            Term* t = createTerm(mapId(i->first), *j);
            TermQuery* tq = _CLNEW TermQuery(t);
            _CLDECDELETE(t);
            bq.add(tq, true, false, true);
        }
    }
}

std::vector<IndexedDocument>
CLuceneIndexReader::query(const Query& q) {
    BooleanQuery bq;
    createBooleanQuery(q, bq);
    lucene::index::IndexReader* reader = manager->refReader();
    IndexSearcher searcher(reader);
    std::vector<IndexedDocument> results;
    TCHAR tf[CL_MAX_DIR];
    char path[CL_MAX_DIR];
    Hits *hits = searcher.search(&bq);
    int s = hits->length();
    STRCPY_AtoT(tf, "path", CL_MAX_DIR);
    for (int i = 0; i < s; ++i) {
        Document *d = &hits->doc(i);
        const TCHAR* v = d->get(tf);
        STRCPY_TtoA(path, v, CL_MAX_DIR);
        IndexedDocument doc;
        doc.filepath = path;
        doc.score = hits->score(i);
        Field* f = d->getField(_T("content"));
        if (f) {
            v = f->stringValue();
            STRCPY_TtoA(path, v, CL_MAX_DIR);
            // remove newlines
            char *p = path;
            while (true) {
                if (*p == '\0') break;
                if (*p == '\n' || *p == '\r') *p = ' ';
                p++;
            }
            doc.fragment = path;
        }
        results.push_back(doc);
    }
    delete hits;
    searcher.close();
    manager->derefReader();
    return results;
}
std::map<std::string, time_t>
CLuceneIndexReader::getFiles(char depth) {
    std::map<std::string, time_t> files;
    lucene::index::IndexReader* reader = manager->refReader();
    IndexSearcher searcher(reader);
    TCHAR tstr[CL_MAX_DIR];
    char cstr[CL_MAX_DIR];
    snprintf(cstr, CL_MAX_DIR, "%i", depth);
    STRCPY_AtoT(tstr, cstr, CL_MAX_DIR);
    Term* term = _CLNEW Term(_T("depth"), tstr);
    TermQuery* termquery = _CLNEW TermQuery(term);
    Hits *hits = searcher.search(termquery);
    int s = hits->length();
    STRCPY_AtoT(tstr, "path", CL_MAX_DIR);
    TCHAR tstr2[CL_MAX_DIR];
    STRCPY_AtoT(tstr2, "mtime", CL_MAX_DIR);
    for (int i = 0; i < s; ++i) {
        Document *d = &hits->doc(i);
        const TCHAR* v = d->get(tstr2);
        STRCPY_TtoA(cstr, v, CL_MAX_DIR);
        time_t mtime = atoi(cstr);
        v = d->get(tstr);
        STRCPY_TtoA(cstr, v, CL_MAX_DIR);
        files[cstr] = mtime;
    }
    searcher.close();
    _CLDELETE(hits);
    _CLDELETE(termquery);
    _CLDECDELETE(term);
    manager->derefReader();
    return files;
}
int
CLuceneIndexReader::countDocuments() {
    return manager->docCount();
}
int
CLuceneIndexReader::countWords() {
    return -1;
}
int
CLuceneIndexReader::getIndexSize() {
    return manager->getIndexSize();
}
