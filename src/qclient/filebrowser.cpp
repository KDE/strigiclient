#include "filebrowser.h"
#include <QtGui/QTreeView>
#include <QtGui/QDirModel>
#include <QtGui/QTextBrowser>
#include <QtGui/QHeaderView>

FileBrowser::FileBrowser() : QSplitter(Qt::Vertical) {
    model = new QDirModel();
    view = new QTreeView();
    view->setModel(model);
    QModelIndex index = model->index("/home/oever/tmp/ziptest");
    view->setRootIndex(index);

    browser = new QTextBrowser();

    addWidget(view);
    addWidget(browser);

    connect(view, SIGNAL(clicked(const QModelIndex&)),
        SLOT(clicked(const QModelIndex&)));

    int cols = model->columnCount(index);
    for (int i=0; i<cols; ++i) {
        view->resizeColumnToContents(i);
    }
}
void
FileBrowser::clicked(const QModelIndex& index) {
    int cols = model->columnCount(index);
    for (int i=0; i<cols; ++i) {
        view->resizeColumnToContents(i);
    }
    QString s = model->filePath(index);
    browser->setSource(s);
}

#include "filebrowser.moc"
