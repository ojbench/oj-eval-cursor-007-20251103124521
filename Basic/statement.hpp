/*
 * File: statement.h
 * -----------------
 * This file defines the Statement abstract type.  In
 * the finished version, this file will also specify subclasses
 * for each of the statement types.  As you design your own
 * version of this class, you should pay careful attention to
 * the exp.h interface, which is an excellent model for
 * the Statement class hierarchy.
 */

#ifndef _statement_h
#define _statement_h

#include <string>
#include <sstream>
#include "evalstate.hpp"
#include "exp.hpp"
#include "Utils/tokenScanner.hpp"
#include "program.hpp"
#include "parser.hpp"
#include "Utils/error.hpp"
#include "Utils/strlib.hpp"

class Program;

/*
 * Class: Statement
 * ----------------
 * This class is used to represent a statement in a program.
 * The model for this class is Expression in the exp.h interface.
 * Like Expression, Statement is an abstract class with subclasses
 * for each of the statement and command types required for the
 * BASIC interpreter.
 */

class Statement {

public:

/*
 * Constructor: Statement
 * ----------------------
 * The base class constructor is empty.  Each subclass must provide
 * its own constructor.
 */

    Statement();

/*
 * Destructor: ~Statement
 * Usage: delete stmt;
 * -------------------
 * The destructor deallocates the storage for this expression.
 * It must be declared virtual to ensure that the correct subclass
 * destructor is called when deleting a statement.
 */

    virtual ~Statement();

/*
 * Method: execute
 * Usage: stmt->execute(state);
 * ----------------------------
 * This method executes a BASIC statement.  Each of the subclasses
 * defines its own execute method that implements the necessary
 * operations.  As was true for the expression evaluator, this
 * method takes an EvalState object for looking up variables or
 * controlling the operation of the interpreter.
 */

    virtual void execute(EvalState &state, Program &program) = 0;

};


// REM statement: comment (no-op)
class RemStatement : public Statement {
public:
    explicit RemStatement(TokenScanner &scanner);
    ~RemStatement() override = default;
    void execute(EvalState &state, Program &program) override;
private:
    std::string comment;
};

// LET statement: assignment expression
class LetStatement : public Statement {
public:
    explicit LetStatement(TokenScanner &scanner);
    ~LetStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp = nullptr;
};

// PRINT statement
class PrintStatement : public Statement {
public:
    explicit PrintStatement(TokenScanner &scanner);
    ~PrintStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    Expression *exp = nullptr;
};

// INPUT statement
class InputStatement : public Statement {
public:
    explicit InputStatement(TokenScanner &scanner);
    ~InputStatement() override = default;
    void execute(EvalState &state, Program &program) override;
private:
    std::string varName;
};

// END statement
class EndStatement : public Statement {
public:
    EndStatement() = default;
    ~EndStatement() override = default;
    void execute(EvalState &state, Program &program) override;
};

// GOTO statement
class GotoStatement : public Statement {
public:
    explicit GotoStatement(TokenScanner &scanner);
    ~GotoStatement() override = default;
    void execute(EvalState &state, Program &program) override;
private:
    int targetLine = -1;
};

// IF ... THEN ... statement
class IfStatement : public Statement {
public:
    explicit IfStatement(TokenScanner &scanner);
    ~IfStatement() override;
    void execute(EvalState &state, Program &program) override;
private:
    std::string lhsStr;
    std::string rhsStr;
    std::string op; // one of "<", ">", "="
    int targetLine = -1;
};

#endif
