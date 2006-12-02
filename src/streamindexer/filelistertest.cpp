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
#include "jstreamsconfig.h"
#include "filelister.h"
#include "indexerconfiguration.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace jstreams;

/**
 * This test file can be used to measure the performance of the filelister
 * class. The speed can be compared to e.g. "find [path] -printf ''".
 **/

vector<string> files;
vector<string> dirs;

void fileCallback(const char* path, uint dirlen, uint len, time_t mtime) {
    // stupid callback, used to test a situation similar to inotify's memory leak
    string file (path, len);
    files.push_back(file);
}

void dirCallback(const char* path, uint len)
{
    // stupid callback, used to test a situation similar to inotify's memory leak
    string dir (path, len);
    dirs.push_back(dir);
}

int
main(int argc, char** argv) {
    if (argc != 2) {
        return -1;
    }

//  TODO add the rules to the indexerconfiguration

    IndexerConfiguration ic;
    FileLister lister (ic);
    lister.setFileCallbackFunction(&fileCallback);
    lister.setDirCallbackFunction(&dirCallback);

    lister.listFiles(argv[1]);

    printf ("files:\n");
    for (unsigned int i = 0; i < files.size(); i++)
        cout <<"\t|"<< files[i] << "|\n";

    printf ("dirs:\n");
    for (unsigned int i = 0; i < dirs.size(); i++)
        cout <<"\t|"<< dirs[i] << "|\n";

    return 0;
}
