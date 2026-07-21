#!/usr/bin/env python
##
# Doxygen filter for Google Protocol Buffers .proto files.
# This script converts .proto files into C++ style ones
# and prints the output to standard output.
#
# version 0.8-beta OSI
#
# How to enable this filter in Doxygen:
#   1. Generate Doxygen configuration file with command 'doxygen -g <filename>'
#        e.g.  doxygen -g doxyfile
#   2. In the Doxygen configuration file, find FILE_PATTERNS and add *.proto
#        FILE_PATTERNS          = *.proto
#   3. In the Doxygen configuration file, find EXTENSION_MAPPING and add proto=C++
#        EXTENSION_MAPPING      = proto=C++
#   4. In the Doxygen configuration file, find INPUT_FILTER and add this script
#        INPUT_FILTER           = "python proto2cpp.py"
#   5. Run Doxygen with the modified configuration
#        doxygen doxyfile
#
#
# Version 0.8 2018 Bugfix regarding long comments, remove typo
# Version 0.7 2018 Bugfix and extensions have been made by Open Simulation Interface (OSI) Carsten Kuebler https://github.com/OpenSimulationInterface,
# Copyright (C) 2016 Regents of the University of California https://github.com/vgteam/vg
# Copyright (C) 2012-2015 Timo Marjoniemi https://sourceforge.net/p/proto2cpp/wiki/Home/
# All rights reserved.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
##

import fnmatch
import inspect
import os
import re
import sys


## Class for converting Google Protocol Buffers .proto files into C++ style output to enable Doxygen usage.
##
## The C++ style output is printed into standard output.<br />
## There are three different logging levels for the class:
## <ul><li>#logNone: do not log anything</li>
## <li>#logErrors: log errors only</li>
## <li>#logAll: log everything</li></ul>
## Logging level is determined by \c #logLevel.<br />
## Error logs are written to file determined by \c #errorLogFile.<br />
## Debug logs are written to file determined by \c #logFile.
#
class proto2cpp:

    ## Logging level: do not log anything.
    logNone   = 0
    ## Logging level: log errors only.
    logErrors = 1
    ## Logging level: log everything.
    logAll    = 2

    ## Constructor
    #
    def __init__(self):
        ## Debug log file name.
        self.logFile = "proto2cpp.log"
        ## Error log file name.
        self.errorLogFile = "proto2cpp.error.log"
        ## Logging level.
        self.logLevel = self.logNone

    ## Handles a file.
    ##
    ## If @p fileName has .proto suffix, it is processed through parseFile().
    ## Otherwise it is printed to stdout as is except for file \c proto2cpp.py without
    ## path since it's the script given to python for processing.
    ##
    ## @param fileName Name of the file to be handled.
    #
    def handleFile(self, fileName):
        if fnmatch.fnmatch(filename, '*.proto'):
            self.log('\nXXXXXXXXXX\nXX ' + filename + '\nXXXXXXXXXX\n\n')
            # Open the file. Use try to detect whether or not we have an actual file.
            if (sys.version_info >= (3, 0)):
                try:
                    with open(filename, 'r', encoding='utf8') as inputFile:
                        self.parseFile(inputFile)
                    pass
                except IOError as e:
                    self.logError('the file ' + filename + ' could not be opened for reading')
            else:
                # Python 2 code in this block
                try:
                    with open(filename, 'r') as inputFile:
                        self.parseFile(inputFile)
                    pass
                except IOError as e:
                    self.logError('the file ' + filename + ' could not be opened for reading')

        elif not fnmatch.fnmatch(filename, os.path.basename(inspect.getfile(inspect.currentframe()))):
            self.log('\nXXXXXXXXXX\nXX ' + filename + '\nXXXXXXXXXX\n\n')
            if (sys.version_info > (3, 0)):
                try:
                    with open(filename, 'r', encoding='utf8') as theFile:
                        output = ''
                        for theLine in theFile:
                            output += theLine
                        print(output)
                        self.log(output)
                    pass
                except IOError as e:
                    self.logError('the file ' + filename + ' could not be opened for reading')
            else:
                # Python 2 code in this block
                try:
                    with open(filename, 'r') as theFile:
                        output = ''
                        for theLine in theFile:
                            output += theLine
                        print(output)
                        self.log(output)
                    pass
                except IOError as e:
                    self.logError('the file ' + filename + ' could not be opened for reading')

        else:
            self.log('\nXXXXXXXXXX\nXX ' + filename + ' --skipped--\nXXXXXXXXXX\n\n')

    ## Parser function.
    ##
    ## The function takes a .proto file object as input
    ## parameter and modifies the contents into C++ style.
    ## The modified data is printed into standard output.
    ##
    ## @param inputFile Input file object
    #
    def parseFile(self, inputFile):
        # Go through the input file line by line.
        isEnum = False
        isPackage = False
        isMultilineComment = False
        # This variable is here as a workaround for not getting extra line breaks (each line
        # ends with a line separator and print() method will add another one).
        # We will be adding lines into this var and then print the var out at the end.
        theOutput = ''
        for line in inputFile:
            # Search for comment ("//") and add one more slash character ("/") to the comment
            # block to make Doxygen detect it.
            matchComment = re.search("//", line)
            # Search for semicolon and if one is found before comment, add a third slash character
            # ("/") and a smaller than ("<") character to the comment to make Doxygen detect it.
            matchSemicolon = re.search(";", line)
            if matchSemicolon is not None and (matchComment is not None and matchSemicolon.start() < matchComment.start()):
                comment = "///<" + line[matchComment.end():]
                # Replace '.' in nested message references with '::'
                # don't work for multi-nested references and generates problems with URLs and acronyms
                #comment = re.sub(r'\s(\w+)\.(\w+)\s', r' \1::\2 ', comment)
                line = line[:matchComment.start()]
            elif matchComment is not None:
                if isMultilineComment:
                    comment = " * " + line[matchComment.end():]
                else:
                    comment = "/** " + line[matchComment.end():]
                    isMultilineComment = True
                # replace '.' in nested message references with '::'
                # don't work for multi-nested references and generates problems with URLs and acronyms
                #comment = re.sub(r'\s(\w+)\.(\w+)\s', r' \1::\2 ', comment)
                line = line[:matchComment.start()]
            else:
                comment = ""

            # End multiline comment, if there is no comment or if there are some chars before the comment.
            if (matchComment is None or len(line.strip())>0) and isMultilineComment:
                theOutput += " */\n"
                isMultilineComment = False

            # line = line.replace(".", "::") but not in quoted strings (Necessary for import statement)
            line = re.sub(r'\.(?=(?:[^"]*"[^"]*")*[^"]*$)',r'::',line)

            # Search for " option ...;", remove it
            line = re.sub(r'\boption\b[^;]+;', r'', line)

            # Search for " package ", make a namespace
            matchPackage = re.search(r"\bpackage\b", line)
            if matchPackage is not None:
                isPackage = True
                # Convert to C++-style separator and block instead of statement
                line = "namespace" + line[:matchPackage.start()] + line[matchPackage.end():].replace(";", " {")

            # Search for " repeated " fields and make them ...
            #matchRepeated = re.search(r"\brepeated\b", line)
            #if matchRepeated is not None:
            #  # Convert
            #  line = re.sub(r'\brepeated\s+(\S+)', r' repeated \1', line)

            # Search for "enum", start changing all semicolons (";") to commas (",").
            matchEnum = re.search(r"\benum\b", line)
            if matchEnum is not None:
                isEnum = True

            # Search semicolon if we have detected an enum, and replace semicolon with comma.
            if isEnum is True and matchSemicolon is not None:
                line = line.replace(";", ",")

            # Search for a closing brace.
            matchClosingBrace = re.search("}", line)
            if isEnum is True and matchClosingBrace is not None:
                line = line[:matchClosingBrace.start()] + "};" + line[matchClosingBrace.end():]
                isEnum = False
            elif isEnum is False and matchClosingBrace is not None:
                # Message (to be struct) ends => add semicolon so that it'll
                # be a proper C(++) struct and Doxygen will handle it correctly.
                line = line[:matchClosingBrace.start()] + "};" + line[matchClosingBrace.end():]

            # Replacements change start of comment...
            matchMsg = re.search(r"\bmessage\b", line)
            if matchMsg is not None:
                line = line[:matchMsg.start()] + "struct" + line[matchMsg.end():]

            # Replacements change start of comment...
            matchExt = re.search(r"\bextend\b", line)
            if matchExt is not None:
                a_extend = line[matchExt.end():]
                matchName = re.search(r"\b\w[\S:]*\b", a_extend)
                if matchName is not None:
                    name = a_extend[matchName.start():matchName.end()]
                    name = re.sub(r'\w+::',r'',name)
                    a_extend = a_extend[:matchName.start()] + name + ": public " + a_extend[matchName.start():]
                else:
                    a_extend = "_Dummy: public " + a_extend;
                line = line[:matchExt.start()] + "struct " + a_extend

            theOutput += (line.strip() + ' ' + comment.strip()).strip() + '\n'

        if isPackage:
            # Close the package namespace
            theOutput += "}"
            isPackage = False

        # Now that we've got all lines in the string let's split the lines and print out
        # one by one.
        # This is a workaround to get rid of extra empty line at the end which print() method adds.
        lines = theOutput.splitlines()
        for line in lines:
            if len(line) > 0:
                print(line) # Add linebreak to generate documentation correct.
                self.log(line + '\n')

    ## Writes @p string to log file.
    ##
    ## #logLevel must be #logAll or otherwise the logging is skipped.
    ##
    ## @param string String to be written to log file.
    #
    def log(self, string):
        if self.logLevel >= self.logAll:
            with open(self.logFile, 'a') as theFile:
                theFile.write(string)

    ## Writes @p string to error log file.
    ##
    ## #logLevel must be #logError or #logAll or otherwise the logging is skipped.
    ##
    ## @param string String to be written to error log file.
    #
    def logError(self, string):
        if self.logLevel >= self.logError:
            with open(self.errorLogFile, 'a') as theFile:
                theFile.write(string)

converter = proto2cpp()
# Doxygen will give us the file names
for filename in sys.argv[1:]:
    converter.handleFile(filename)

# end of file