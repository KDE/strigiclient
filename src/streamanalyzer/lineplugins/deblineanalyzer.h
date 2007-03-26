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
#ifndef DEBLINEANALYZER
#define DEBLINEANALYZER

#include "analysisresult.h"
#include "analyzerplugin.h"
#include "streamlineanalyzer.h"
#include "cnstr.h"


class DebLineAnalyzerFactory;
class DebLineAnalyzer : public Strigi::StreamLineAnalyzer {
private:
    const DebLineAnalyzerFactory* factory;
    virtual void startAnalysis(Strigi::AnalysisResult*);
    virtual void handleLine(const char* data, uint32_t length);
    virtual bool isReadyWithStream();
    const char* getName() const { return "DebLineAnalyzer"; }
    unsigned int finished;
    Strigi::AnalysisResult* result;
public:
    DebLineAnalyzer(const DebLineAnalyzerFactory* f) : factory(f) {}
};

class DebLineAnalyzerFactory : public Strigi::StreamLineAnalyzerFactory {
public:
    static const cnstr nameFieldName;
    static const cnstr versionFieldName;
    static const cnstr summaryFieldName;
    static const cnstr maintainerFieldName;
    static const cnstr sectionFieldName;
    static const cnstr dependsFieldName;
    const Strigi::RegisteredField* nameField;
    const Strigi::RegisteredField* versionField;
    const Strigi::RegisteredField* summaryField;
    const Strigi::RegisteredField* maintainerField;
    const Strigi::RegisteredField* sectionField;
    const Strigi::RegisteredField* dependsField;
    const char* getName() const {
        return "DebLineAnalyzer";
    }
    Strigi::StreamLineAnalyzer* newInstance() const {
        return new DebLineAnalyzer(this);
    }
    void registerFields(Strigi::FieldRegister&);
};

#endif