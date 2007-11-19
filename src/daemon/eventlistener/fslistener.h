/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2007 Flavio Castelli <flavio.castelli@gmail.com>
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
#ifndef FSLISTENER_H
#define FSLISTENER_H

#include "eventlistener.h"
#include "strigi_thread.h"
#include <map>
#include <vector>

class Event;
class PollingListener;

//TODO: update documentation

class FsEvent
{
    public:
        FsEvent () {};
        virtual ~FsEvent() {};

        enum TYPE {CREATE, UPDATE, DELETE};

        virtual const std::string description() = 0;
        TYPE type() { return m_type; }
        const std::string file() { return m_file;}
        virtual bool regardsDir() = 0;

    protected:
        std::string m_file;
        TYPE m_type;
};


/*!
 * @class FsListener
 * @brief Interacts with kernel inotify monitoring recursively all changes over indexed directories
 */

class FsListener : public EventListener
{
    public:
        explicit FsListener(const char* name,
                            std::set<std::string>& indexedDirs);

        virtual ~FsListener();

        virtual bool init() = 0;

        void setIndexedDirectories (const std::set<std::string>& dirs);

        void* run(void*);

    protected:
        void setInitialized () { m_bInitialized = true; }

        /*!
         * @param event the inotify event to analyze
         * returns true if event is to process (ergo is interesting), false otherwise
         */
        virtual bool isEventInteresting (FsEvent * event) = 0;

        /*!
         * main FsListener thread
         */
        void watch ();

        void bootstrap();
        void reindex();
        bool reindexReq();
        void getReindexDirs(std::set<std::string>&);

        // event methods
        virtual bool pendingEvent() = 0;
        virtual FsEvent* retrieveEvent() = 0;
        virtual bool isEventValid(FsEvent* event) = 0;

        // dir methods
        
        /*!
         * @param dir removed dir
         * Removes all db entries of files contained into the removed dir.
         * Removes also all inotify watches related to removed dir (including watches over subdirs), there's <b>no need</b> to call rmWatch after invoking that method
         * Updates also m_watches
         */
        virtual void dirRemoved (std::string dir,
                                 std::vector<Event*>& events) = 0;

        /*!
         * @param dirs removed dirs
         * Removes all db entries of files contained into the removed dirs.
         * Removes also all inotify watches related to removed dirs (including watches over subdirs), there's <b>no need</b> to call rmWatch after invoking that method
         * Optimized for large removal
         * Updates also m_watches
         */
        void dirsRemoved (std::set<std::string> dirs,
                          std::vector<Event*>& events);

        void recursivelyMonitor (const std::string dir,
                                 std::vector<Event*>& events);

        // watches methods
        virtual void addWatches (const std::set<std::string>& watches) = 0;

        /*!
         * removes and release all inotify watches
         */
        virtual void clearWatches() {};

        bool m_bMonitor;
        bool m_bInitialized;
        bool m_bBootstrapped;

        std::set<std::string> m_indexedDirs;
        
        bool m_bReindexReq;
        std::set<std::string> m_reindexDirs;
        pthread_mutex_t m_reindexLock;
        std::vector<FsEvent*> m_events;
};

#endif