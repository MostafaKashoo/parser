#include <iostream>
#include <fstream>
#include <cctype>
#include <vector>
#include <string>
#include <unordered_set>
#include <cstdlib>

using namespace std;

enum TokenType { KEYWORD, IDENTIFIER, NUMBER, OPERATOR, SPECIAL_SYMBOL, END_OF_FILE };

struct Token {
    TokenType type;
    string value;
};

vector<Token> tokenList;

unordered_set<string> keywords = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "int", "long",
    "register", "return", "short", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

bool isKeyword(const string& word) {
    return keywords.find(word) != keywords.end();
}

bool isSpecialSymbol(char ch) {
    string special = "();{}[],";
    return special.find(ch) != string::npos;
}

bool isOperator(char ch) {
    string ops = "+-*/%=<>!";
    return ops.find(ch) != string::npos;
}

void saveToken(TokenType t, string v) {
    tokenList.push_back({ t, v });
}

class Parser {
    vector<Token> tokens;
    int current = 0;
public:
    Parser(vector<Token> t) : tokens(t) {}

    Token peek() { return tokens[current]; }

    bool match(string val) {
        if (peek().value == val) { current++; return true; }
        return false;
    }

    bool matchType(TokenType type) {
        if (peek().type == type) { current++; return true; }
        return false;
    }

    void error(string msg) {
        cout << "\n Syntax Error: " << msg << " at '" << peek().value << "'" << endl;
        exit(1);
    }

    // Grammar
    void parseProgram() {
        if (!match("int")) error("Expected 'int'");
        if (!match("main")) error("Expected 'main'");
        if (!match("(")) error("Expected '('");
        if (!match(")")) error("Expected ')'");
        if (!match("{")) error("Expected '{'");

        parseStatementList();

        if (!match("}")) error("Expected '}' at end of main");
        cout << "\n Parsed Successfully! Syntax is Correct.\n";
    }

    void parseStatementList() {
        while (peek().value != "}" && peek().type != END_OF_FILE) {
            string val = peek().value;
            if (val == "int" || val == "float") parseDeclaration();
            else if (val == "if") parseIf();
            else if (peek().type == IDENTIFIER) parseAssignment();
            else if (val == "return") parseReturn();
            else error("Unknown statement");
        }
    }

    void parseDeclaration() {
        current++; 
        if (!matchType(IDENTIFIER)) error("Expected variable name");
        while (peek().value == ",") {
            match(",");
            if (!matchType(IDENTIFIER)) error("Expected variable name after ','");
        }
        if (!match(";")) error("Expected ';'");
    }

    void parseAssignment() {
        current++; 
        if (!match("=")) error("Expected '='");
        parseExpression();
        if (!match(";")) error("Expected ';'");
    }

    void parseIf() {
        match("if");
        if (!match("(")) error("Expected '('");
        parseCondition();
        if (!match(")")) error("Expected ')'");

        if (!match("{")) error("Expected '{' inside if");
        parseStatementList();
        if (!match("}")) error("Expected '}' end of if");

        if (peek().value == "else") {
            match("else");
            if (!match("{")) error("Expected '{' inside else");
            parseStatementList();
            if (!match("}")) error("Expected '}' end of else");
        }
    }

    void parseReturn() {
        match("return");
        parseExpression();
        if (!match(";")) error("Expected ';'");
    }

    void parseCondition() {
        parseExpression();
        if (peek().value == "==") {
            match("==");
            parseExpression();
        }
    }

    void parseExpression() {
        parseTerm();
        while (peek().value == "-" || peek().value == "+") {  
            current++;
            parseTerm();
        }
    }

    void parseTerm() {
        if (peek().type == IDENTIFIER || peek().type == NUMBER) current++;
        else error("Expected number or variable");
    }
};

int main() {
    string code;
    cout << "Enter C code below (end with # on a new line):\n";
    string line;
    while (getline(cin, line)) {
        if (line == "#") break;
        code += line + '\n';
    }

    cout << "\nTOKENS:\n\n";

    string token;
    for (size_t i = 0; i < code.size(); ++i) {
        char ch = code[i];
        if (isspace(ch)) continue;

        if (ch == '/' && i + 1 < code.size()) {
            if (code[i + 1] == '/') {
                while (i < code.size() && code[i] != '\n') i++;
                continue;
            }
            else if (code[i + 1] == '*') {
                i += 2;
                while (i + 1 < code.size() && !(code[i] == '*' && code[i + 1] == '/')) i++;
                i++; continue;
            }
        }

        if (isalpha(ch) || ch == '_') {
            token.clear();
            while (isalnum(code[i]) || code[i] == '_') {
                token += code[i++];
            }
            --i;
            if (isKeyword(token)) {
                cout << "Keyword\t\t" << token << "\n";
                saveToken(KEYWORD, token);
            }
            else {
                cout << "Identifier\t" << token << "\n";
                saveToken(IDENTIFIER, token);
            }
        }
        else if (isdigit(ch)) {
            token.clear();
            while (isdigit(code[i]) || code[i] == '.') {
                token += code[i++];
            }
            --i;
            cout << "Number\t\t" << token << "\n";
            saveToken(NUMBER, token);
        }
        else if (isOperator(ch)) {
            token.clear();
            token += ch;
            if (i + 1 < code.size()) {
                string two = token + code[i + 1];
                if (two == "==" || two == "!=" || two == "<=" || two == ">=" || two == "++" || two == "--") {
                    token = two;
                    i++;
                }
            }
            cout << "Operator\t" << token << "\n";
            saveToken(OPERATOR, token);
        }
        else if (isSpecialSymbol(ch)) {
            cout << "Special Symbol\t" << ch << "\n";
            string s(1, ch);
            saveToken(SPECIAL_SYMBOL, s); 
        }
    }

    saveToken(END_OF_FILE, "");

    cout << "\n-----------------------------------\n";
    cout << "Running Parser Check...\n";

    Parser parser(tokenList);
    parser.parseProgram();

    return 0;
}
