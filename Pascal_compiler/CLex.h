#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

#include <map>
#include <stack>

#define INTEGER "integer" 
#define WORD "word"
#define CHAR "char"
#define BOOLEAN "boolean"

using namespace std;
void load_DFA(ifstream&);//load dfa
void preset_dfa();
bool empty_file(ifstream& pFile);
ifstream code_file;
const int rows = 13;//this constant variable will change in the future
int DFA[128][rows];//our dfa


typedef struct CLex {

    CLex() {}//default constructor would go here

    CLex(string filename) {//constructor

        ifstream filestream;
        load_DFA(filestream);//load dfa

        code_file.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    }

    void get_file_name(string file_name) {

        ifstream filestream;
        load_DFA(filestream);//load dfa

        code_file.open(file_name, std::fstream::in | std::fstream::out | std::fstream::app);

    }


    bool GetToken(CToken& token) {


        token.current_state = 0;
        token.prev_state = 0;

        token.tokenType = "";
        token.tokenValue = "";

        char character;//character from file

        bool continue_loop = true;

        do {
            if (code_file.peek() != EOF)
                code_file.get(character);//get character from file
            else {
                character = 32;
                continue_loop = false;
            }



            token.prev_state = token.current_state;//set prev state to current state
            int current_state = token.current_state;

            int current_state_test = DFA[character][current_state];//test that we got correct current state

            token.current_state = current_state_test;//set current state correctly

            if ((token.current_state != 0) && (token.current_state != 55) && (token.current_state != 99))//if the current state is not equal to either 0,55,99
            {
                token.tokenValue.push_back(character);//append character to token value
            }
            else if (token.current_state == 55 && character != 32 && character != 9 && character != 10 && character != 11
                && character != 12 && character != 13) {//if we're in state 55 and character is not a whitespace

                code_file.unget();//move back one step in file
            }

            if (token.current_state == 55) {

                for_each(token.tokenValue.begin(), token.tokenValue.end(), [](char& c) {
                    c = ::tolower(c);
                    });

                if (token.prev_state == 1) {



                    if (token.tokenValue._Equal("if"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("then"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("else"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("while"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("do"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("begin"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("end"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("or"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("and"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("program"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("var"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("integer"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("char"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("boolean"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("procedure"))
                        token.tokenType = "r_word";
                    else if (token.tokenValue._Equal("function"))
                        token.tokenType = "r_word";
                    else
                        token.tokenType = "word";


                }
                else if (token.prev_state == 2)
                    token.tokenType = "integer";
                else if (token.prev_state == 4)
                    token.tokenType = "real";
                else if (token.prev_state == 9)
                    token.tokenType = "real";
                else if (token.prev_state == 10)
                    token.tokenType = "special";
                else if (token.prev_state == 11)
                    token.tokenType = "special";
                else if (token.prev_state == 12)
                    token.tokenType = "assignment";

                return true;
            }
            else if (token.current_state == 99) {

                printf("Lex error\n");

                return false;
            }

        } while (continue_loop);


        if (token.prev_state == 1 || token.prev_state == 2 || token.prev_state == 3 || token.prev_state == 4 || token.prev_state == 9 || token.prev_state == 10) {//if previous state is a final state

            if (token.prev_state == 1)
                token.tokenType = "word";
            else if (token.prev_state == 2)
                token.tokenType = "integer";
            else if (token.prev_state == 4)
                token.tokenType = "real";
            else if (token.prev_state == 9)
                token.tokenType = "real";
            else if (token.prev_state == 10)
                token.tokenType = "special";
        }
        else {
            //printf("Lex error\n");

            return false;
        }
        return true;
    }

}CLex;

void load_DFA(ifstream& filestream) {//load the DFA


    //fstream filestream_loc = filestream;
    preset_dfa();
    //print_dfa();
    string file_name = "completeDFA.txt";


    filestream.open(file_name, std::fstream::in | std::fstream::out | std::fstream::app);

    if (filestream.is_open())
    {

        int test;

        for (int i = 0; i < 127; i++)
        {


            for (int j = 0; j < rows; j++) {

                filestream >> test;

                DFA[i][j] = test;
            }


        }
    }
    else {

        printf("File not open\n");

    }

}

void preset_dfa() {//set everything to 99

    for (int i = 0; i < 128; i++) {

        for (int j = 0; j < rows; j++) {

            DFA[i][j] = 99;
        }
    }
}


bool empty_file(ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}
