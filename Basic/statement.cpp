/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"
#include <climits>


/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// Helper: check if token is a reserved keyword
static bool isKeyword(const std::string &tok) {
    std::string up = toUpperCase(tok);
    return up == "REM" || up == "LET" || up == "PRINT" || up == "INPUT" ||
           up == "END" || up == "GOTO" || up == "IF" || up == "THEN" ||
           up == "RUN" || up == "LIST" || up == "CLEAR" || up == "QUIT" || up == "HELP";
}

// RemStatement
RemStatement::RemStatement(TokenScanner &scanner) {
    // Collect the rest of the line as a comment (optional)
    std::ostringstream oss;
    while (scanner.hasMoreTokens()) {
        if (oss.tellp() > 0) oss << ' ';
        oss << scanner.nextToken();
    }
    comment = oss.str();
}

void RemStatement::execute(EvalState &state, Program &program) {
    (void) state; (void) program; // no-op
}

// LetStatement
LetStatement::LetStatement(TokenScanner &scanner) {
    // Parse assignment expression after LET
    exp = parseExp(scanner);
    // Ensure it is an assignment expression (contains '=')
    if (exp->getType() != COMPOUND || static_cast<CompoundExp *>(exp)->getOp() != "=") {
        delete exp;
        exp = nullptr;
        error("SYNTAX ERROR");
    }
}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    (void) program;
    (void) state;
    // Evaluate assignment
    (void) exp->eval(state);
}

// PrintStatement
PrintStatement::PrintStatement(TokenScanner &scanner) {
    exp = parseExp(scanner);
}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    (void) program;
    int value = exp->eval(state);
    std::cout << value << std::endl;
}

// InputStatement
InputStatement::InputStatement(TokenScanner &scanner) {
    std::string tok = scanner.nextToken();
    if (tok.empty()) error("SYNTAX ERROR");
    TokenType tp = scanner.getTokenType(tok);
    if (tp != WORD && tp != NUMBER) error("SYNTAX ERROR");
    if (isKeyword(tok)) error("SYNTAX ERROR");
    varName = tok;
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}

static bool tryParseInt(const std::string &s, int &out) {
    if (s.empty()) return false;
    size_t idx = 0;
    bool neg = false;
    if (s[0] == '+' || s[0] == '-') {
        neg = s[0] == '-';
        idx = 1;
    }
    if (idx >= s.size()) return false;
    long long val = 0;
    for (; idx < s.size(); ++idx) {
        if (s[idx] < '0' || s[idx] > '9') return false;
        val = val * 10 + (s[idx] - '0');
        if (val > 2147483648LL) return false; // guard overflow when neg
    }
    if (neg) val = -val;
    if (val < INT_MIN || val > INT_MAX) return false;
    out = static_cast<int>(val);
    return true;
}

void InputStatement::execute(EvalState &state, Program &program) {
    (void) program;
    while (true) {
        std::cout << " ? ";
        std::cout.flush();
        std::string line;
        if (!std::getline(std::cin, line)) {
            // EOF treated as 0
            state.setValue(varName, 0);
            return;
        }
        line = trim(line);
        int value = 0;
        if (!tryParseInt(line, value)) {
            std::cout << "INVALID NUMBER" << std::endl;
            continue;
        }
        state.setValue(varName, value);
        return;
    }
}

// EndStatement
void EndStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.requestStop();
}

// GotoStatement
GotoStatement::GotoStatement(TokenScanner &scanner) {
    std::string tok = scanner.nextToken();
    if (scanner.getTokenType(tok) != NUMBER) error("SYNTAX ERROR");
    targetLine = stringToInteger(tok);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}

void GotoStatement::execute(EvalState &state, Program &program) {
    (void) state;
    if (!program.hasLine(targetLine)) error("LINE NUMBER ERROR");
    program.requestJump(targetLine);
}

// IfStatement
IfStatement::IfStatement(TokenScanner &scanner) {
    // Parse: IF <exp> <op> <exp> THEN <line>
    // Collect tokens for lhs until relational op at top level
    std::ostringstream lhsBuf, rhsBuf;
    int depth = 0;
    bool foundOp = false;
    while (scanner.hasMoreTokens()) {
        std::string tok = scanner.nextToken();
        if (tok == "(") depth++;
        if (tok == ")") depth--;
        if (depth == 0 && (tok == "<" || tok == ">" || tok == "=")) {
            op = tok;
            foundOp = true;
            break;
        }
        if (lhsBuf.tellp() > 0) lhsBuf << ' ';
        lhsBuf << tok;
    }
    if (!foundOp) error("SYNTAX ERROR");
    // Collect rhs until THEN at top level
    depth = 0;
    bool foundThen = false;
    while (scanner.hasMoreTokens()) {
        std::string tok = scanner.nextToken();
        if (tok == "(") depth++;
        if (tok == ")") depth--;
        if (depth == 0 && toUpperCase(tok) == "THEN") {
            foundThen = true;
            break;
        }
        if (rhsBuf.tellp() > 0) rhsBuf << ' ';
        rhsBuf << tok;
    }
    if (!foundThen) error("SYNTAX ERROR");
    std::string lineTok = scanner.nextToken();
    if (scanner.getTokenType(lineTok) != NUMBER) error("SYNTAX ERROR");
    targetLine = stringToInteger(lineTok);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");

    // Store raw expression strings for evaluation at runtime
    lhsStr = lhsBuf.str();
    rhsStr = rhsBuf.str();
}

IfStatement::~IfStatement() {
    // nothing to free
}

void IfStatement::execute(EvalState &state, Program &program) {
    // Parse and evaluate expressions at runtime
    TokenScanner ls(lhsStr);
    ls.ignoreWhitespace();
    ls.scanNumbers();
    Expression *l = readE(ls, 0);
    TokenScanner rs(rhsStr);
    rs.ignoreWhitespace();
    rs.scanNumbers();
    Expression *r = readE(rs, 0);
    int lv = l->eval(state);
    int rv = r->eval(state);
    delete l;
    delete r;
    bool cond = false;
    if (op == "=") cond = (lv == rv);
    else if (op == "<") cond = (lv < rv);
    else if (op == ">") cond = (lv > rv);
    if (cond) {
        if (!program.hasLine(targetLine)) error("LINE NUMBER ERROR");
        program.requestJump(targetLine);
    }
}
