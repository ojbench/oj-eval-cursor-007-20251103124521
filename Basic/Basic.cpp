/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            if (!std::getline(std::cin, input)) break;
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    std::string first = scanner.nextToken();
    if (first.empty()) return;
    TokenType firstType = scanner.getTokenType(first);

    auto toUpper = [](const std::string &s) {
        return toUpperCase(s);
    };

    if (firstType == NUMBER) {
        // Program line
        int lineNumber = stringToInteger(first);
        if (!scanner.hasMoreTokens()) {
            program.removeSourceLine(lineNumber);
            return;
        }
        std::string keyword = scanner.nextToken();
        std::string upper = toUpper(keyword);
        Statement *stmt = nullptr;
        if (upper == "REM") stmt = new RemStatement(scanner);
        else if (upper == "LET") stmt = new LetStatement(scanner);
        else if (upper == "PRINT") stmt = new PrintStatement(scanner);
        else if (upper == "INPUT") stmt = new InputStatement(scanner);
        else if (upper == "END") {
            if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
            stmt = new EndStatement();
        } else if (upper == "GOTO") stmt = new GotoStatement(scanner);
        else if (upper == "IF") stmt = new IfStatement(scanner);
        else {
            error("SYNTAX ERROR");
        }
        // Store line and parsed statement
        program.addSourceLine(lineNumber, line);
        program.setParsedStatement(lineNumber, stmt);
        return;
    }

    // Immediate mode commands
    std::string cmd = toUpper(first);
    if (cmd == "REM") {
        // ignore rest
        return;
    }
    if (cmd == "LET") {
        Statement *stmt = new LetStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
        return;
    }
    if (cmd == "PRINT") {
        Statement *stmt = new PrintStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
        return;
    }
    if (cmd == "INPUT") {
        Statement *stmt = new InputStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
        return;
    }
    if (cmd == "LIST") {
        int ln = program.getFirstLineNumber();
        while (ln != -1) {
            std::cout << program.getSourceLine(ln) << std::endl;
            ln = program.getNextLineNumber(ln);
        }
        return;
    }
    if (cmd == "CLEAR") {
        program.clear();
        state.Clear();
        return;
    }
    if (cmd == "RUN") {
        // Run stored program
        int current = program.getFirstLineNumber();
        program.clearPendingJump();
        program.clearStop();
        while (current != -1) {
            Statement *stmt = program.getParsedStatement(current);
            if (stmt == nullptr) error("SYNTAX ERROR");
            stmt->execute(state, program);
            if (program.shouldStop()) break;
            int nextLine = -1;
            if (program.hasPendingJump()) {
                nextLine = program.getPendingJump();
                program.clearPendingJump();
            } else {
                nextLine = program.getNextLineNumber(current);
            }
            current = nextLine;
        }
        return;
    }
    if (cmd == "QUIT") {
        exit(0);
    }
    if (cmd == "END" || cmd == "GOTO" || cmd == "IF") {
        error("SYNTAX ERROR");
    }

    // Unrecognized
    error("SYNTAX ERROR");
}

