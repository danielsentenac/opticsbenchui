/**
 * @file QsLogDest.h
 * @brief Qs Log Dest logging support.
 */
// Copyright (c) 2010, Razvan Petru
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// * The name of the contributors may not be used to endorse or promote products
//   derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSLOGDEST_H
#define QSLOGDEST_H

#include <memory>
class QString;

/// \ingroup utils
/// Logging destination interfaces and factory helpers.
namespace QsLogging
{

/// Abstract log destination.
class Destination
{
public:
   /// Destructor.
   virtual ~Destination(){}
   /// Write a log message to the destination.
   /// \param message Log message.
   virtual void write(const QString& message) = 0;
};
/// Destination smart pointer type.
typedef std::auto_ptr<Destination> DestinationPtr;

/// Creates logging destinations/sinks. The caller owns the result.
class DestinationFactory
{
public:
   /// Create a file destination.
   /// \param filePath Output file path.
   static DestinationPtr MakeFileDestination(const QString& filePath);
   /// Create a debug output destination.
   static DestinationPtr MakeDebugOutputDestination();
};

} // end namespace

#endif // QSLOGDEST_H
