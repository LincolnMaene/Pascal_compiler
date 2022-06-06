#pragma once
#include <string>
using namespace std;



typedef struct CToken {

    string tokenType;//type of oken
    string tokenValue;
    int current_state;
    int prev_state;

}CToken;


typedef struct Parser_Token {

    string tokenType;//type of oken
    string tokenValue;
    int consumed = 0;
    int not_ended = 0;

}Parser_Token;

//typedef struct CToken;

//typedef struct CLex;
