/* This file is part of Strigi Desktop Search
 *
 * Copyright (C) 2006 Flavio Castelli <flavio.castelli@gmail.com>
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
 

#ifndef STRIGI_LOGGING_H
#define STRIGI_LOGGING_H

// log4cxx libraries
#include <logger.h>
#include <basicconfigurator.h>


#define STRIGI_LOG_INIT() \
{ \
    log4cxx::BasicConfigurator::configure();\
}

#define STRIGI_LOG_DEBUG(loggerName, message) \
{ \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(loggerName); \
    LOG4CXX_DEBUG(logger, message) \
}

#define STRIGI_LOG_INFO(loggerName, message) \
{ \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(loggerName); \
    LOG4CXX_INFO(logger, message) \
}

#define STRIGI_LOG_WARNING(loggerName, message) \
{ \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(loggerName); \
    LOG4CXX_WARN(logger, message) \
}

#define STRIGI_LOG_ERROR(loggerName, message) \
{ \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(loggerName); \
    LOG4CXX_ERROR(logger, message) \
}

#define STRIGI_LOG_FATAL(loggerName, message) \
{ \
    log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(loggerName); \
    LOG4CXX_FATAL(logger, message) \
}

#endif
