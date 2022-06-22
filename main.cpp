
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"
#include "Token/Token.hpp"

static void HandleDefinition() {
    if (Parser::ParseFuncDefinition()) {
        printf("Parsed a function definition.\n");
    } else {
        // Skip token for error recovery.
        Parser::getNextToken();
    }
}

static void HandleExtern() {
    if (Parser::ParseExtern()) {
        printf("Parsed an extern\n");
    } else {
        // Skip token for error recovery.
        Parser::getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (Parser::ParseTopLevelExpr()) {
        printf("Parsed a top-level expr\n");
    } else {
        // Skip token for error recovery.
        Parser::getNextToken();
    }
}

static void MainLoop() {
    while (true) {
        switch (Parser::CurrentToken.type) {
            case Token::type::tok_eof:
                return;
            case Token::type::tok_func:
                HandleDefinition();
                break;
            case Token::type::tok_extern:
                HandleExtern();
                break;
            default:
                if (Parser::CurrentToken.value == ";")
                    Parser::getNextToken();
                else {
                    HandleTopLevelExpression();
                }
                break;
        }
        fflush(stdout);
    }
}


int main() {
    Lexer::Source = "extern sin(a);";
    Token currentTok;
    while ((currentTok = Lexer::getTok()).type != Token::type::tok_eof) {
        printf("%3d:%-3d %10s %10d\n",
               currentTok.position.row,
               currentTok.position.column,
               currentTok.value.c_str(),
               currentTok.type
        );
    }
    printf("\n");
    fflush(stdout);

    Lexer::CharIdx = 0; // reset

    // set parser precedences
    Parser();

    // Prime the first token.
    Parser::getNextToken();

    MainLoop();
}