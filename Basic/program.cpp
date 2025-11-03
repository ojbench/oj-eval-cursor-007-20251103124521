/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"



Program::Program() = default;

Program::~Program() {
    clear();
}

void Program::clear() {
    // Delete all parsed statements
    for (auto &entry : parsedByNumber) {
        delete entry.second;
    }
    parsedByNumber.clear();
    sourceLinesByNumber.clear();
    clearPendingJump();
    clearStop();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    // Replace or insert source line; delete old parsed statement if replacing
    if (sourceLinesByNumber.find(lineNumber) != sourceLinesByNumber.end()) {
        // replacing existing line: drop old parsed statement
        auto it = parsedByNumber.find(lineNumber);
        if (it != parsedByNumber.end()) {
            delete it->second;
            parsedByNumber.erase(it);
        }
    }
    sourceLinesByNumber[lineNumber] = line;
}

void Program::removeSourceLine(int lineNumber) {
    auto it = parsedByNumber.find(lineNumber);
    if (it != parsedByNumber.end()) {
        delete it->second;
        parsedByNumber.erase(it);
    }
    sourceLinesByNumber.erase(lineNumber);
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLinesByNumber.find(lineNumber);
    if (it == sourceLinesByNumber.end()) return "";
    return it->second;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    if (sourceLinesByNumber.find(lineNumber) == sourceLinesByNumber.end()) {
        // No such line
        delete stmt;
        error("SYNTAX ERROR");
    }
    auto it = parsedByNumber.find(lineNumber);
    if (it != parsedByNumber.end()) {
        delete it->second;
        it->second = stmt;
    } else {
        parsedByNumber.emplace(lineNumber, stmt);
    }
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
   auto it = parsedByNumber.find(lineNumber);
   if (it == parsedByNumber.end()) return nullptr;
   return it->second;
}

int Program::getFirstLineNumber() {
    if (sourceLinesByNumber.empty()) return -1;
    return sourceLinesByNumber.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = sourceLinesByNumber.upper_bound(lineNumber);
    if (it == sourceLinesByNumber.end()) return -1;
    return it->first;
}

bool Program::hasLine(int lineNumber) const {
    return sourceLinesByNumber.find(lineNumber) != sourceLinesByNumber.end();
}

void Program::requestJump(int lineNumber) {
    jumpPending = true;
    jumpTarget = lineNumber;
}

bool Program::hasPendingJump() const {
    return jumpPending;
}

int Program::getPendingJump() const {
    return jumpTarget;
}

void Program::clearPendingJump() {
    jumpPending = false;
    jumpTarget = -1;
}

void Program::requestStop() {
    stopRequested = true;
}

bool Program::shouldStop() const {
    return stopRequested;
}

void Program::clearStop() {
    stopRequested = false;
}


