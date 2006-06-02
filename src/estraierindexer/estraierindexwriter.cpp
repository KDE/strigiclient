#include "estraierindexwriter.h"
#include "estraierindexmanager.h"
#include <vector>
#include <sstream>
using namespace std;
using namespace jstreams;

struct EstraierData {
    ESTDOC* doc;
    int nwords;
};

EstraierIndexWriter::EstraierIndexWriter(EstraierIndexManager *m, ESTDB* d)
        : manager(m), db(d) {
}
EstraierIndexWriter::~EstraierIndexWriter() {
    // make sure the cache is empty
    commit();
}
void
EstraierIndexWriter::addStream(const Indexable* idx, const string& fieldname,
        StreamBase<wchar_t>* datastream) {
}
void
EstraierIndexWriter::addField(const Indexable* idx, const string& name,
        const string& value) {
    EstraierData* data = static_cast<EstraierData*>(idx->getWriterData());
    ESTDOC* doc = data->doc;
    if (name == "content") {
        if (data->nwords < 10000) {
            est_doc_add_text(doc, value.c_str());
            data->nwords++;
        }
    } else {
        est_doc_add_attr(doc, name.c_str(), value.c_str());
    }
}
void
EstraierIndexWriter::setField(const Indexable* idx, const std::string& name,
        int64_t value) {
}

void
EstraierIndexWriter::startIndexable(Indexable* idx) {
    // allocate a new estraier document
    EstraierData* data = new EstraierData();
    data->doc = est_doc_new();
    data->nwords = 0;
    idx->setWriterData(data);
}
/*
    Close all left open indexwriters for this path.
*/
void
EstraierIndexWriter::finishIndexable(const Indexable* idx) {
    EstraierData* data = static_cast<EstraierData*>(idx->getWriterData());
    ESTDOC* doc = data->doc;
    // add required url field

    est_doc_add_attr(doc, "@uri", idx->getName().c_str());
    char numbuf[64];
    sprintf(numbuf, "%lli", idx->getMTime());
    est_doc_add_attr(doc, "@mdate", numbuf);
    sprintf(numbuf, "%i", idx->getDepth());
    est_doc_add_attr(doc, "depth", numbuf);

    manager->ref();
    int ok = est_db_put_doc(db, doc, 0);
    if (!ok) printf("could not write document\n");
    // deallocate the estraier document
    est_doc_delete(doc);
    delete data;
    manager->deref();
}
void
EstraierIndexWriter::commit() {
    manager->ref();
    //est_db_optimize(db, 0);
    est_db_sync(db);
    manager->deref();
}
/**
 * Delete all files that start with the specified path.
 **/
void
EstraierIndexWriter::deleteEntries(const std::vector<std::string>& entries) {
    manager->ref();

    // retrieve the ids of all documents
    int n;
    int* all;
    ESTCOND* c = est_cond_new();
    est_cond_add_attr(c, "@id NUMGE 0");
    all = est_db_search(db, c, &n, NULL);
    est_cond_delete(c);

    // loop over all documents and check if they should be deleted
    vector<string>::const_iterator j;
    for (int i=0; i<n; ++i) {
        int id = all[i];
        char* uri = est_db_get_doc_attr(db, id, "@uri");
        int len = strlen(uri);
        for (j = entries.begin(); j != entries.end(); ++j) {
            if (j->length() <= len
                    && strncmp(j->c_str(), uri, j->length()) == 0) {
                est_db_out_doc(db, id, 0);
                break;
            }
        }
        free(uri);
    }
    free(all);
    manager->deref();
}
