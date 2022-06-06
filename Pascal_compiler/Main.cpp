#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

#include <map>
#include <stack>
#include "CToken.h"
#include "CLex.h"
#include "CParser.h"
#include "Registers.h"


#define INTEGER "integer" 
#define WORD "word"
#define CHAR "char"
#define BOOLEAN "boolean"


using namespace std;
char opc_table[15][15];

map<string, int> productions;//map for productions
map<string, int> keys;//map for productions




/*

void load_production_map();
void  load_keys();
void  load_OPC_table();
*/


/* Bellow is the list of functions called by language parse*/




/*End of list of functions*/



bool run_fails() {

    bool test_success = true;

    string file_name = "C:\\Users\\Lincoln Maene\\source\\repos\\Pascal_compiler\\Pascal_compiler\\Test data\\fail_";
    //int index = 0;

    for (int i = 0; i < 8; i++) {

        string string_dex = to_string(i);

        file_name = file_name.append(string_dex);
        file_name = file_name.append(".txt");
        //cout << file_name << endl;
        CParser fail_parser(file_name);
        bool bad_flag = fail_parser.get_bad_flag();

        if (bad_flag == false)
            test_success = false;
    }
    
    return test_success;

}
bool run_success() {
    
    bool test_success = true;

    string file_name = "C:\\Users\\Lincoln Maene\\source\\repos\\Pascal_compiler\\Pascal_compiler\\Test data\\success_";
    //int index = 0;

    for (int i = 0; i < 15; i++) {

        string string_dex = to_string(i);

        file_name = file_name.append(string_dex);
        file_name = file_name.append(".txt");
        //cout << file_name << endl;
        CParser sucess_parser(file_name);
        bool bad_flag = sucess_parser.get_bad_flag();

        if (bad_flag == true)
            test_success = false;
    }

    return test_success;

}

bool run_tests() {

    bool tests_run=true;

    bool test_success = run_fails();
    //bool test_success = run_success();
    
    return test_success;
}



RegisterManager test_reg_manager;
int main()
{

    //string register_name = test_reg_manager.get_registers()[0].get_name();
    //cout << endl <<register_name << endl;

   

   
    /*

    CLex LEX("code.txt");

    CToken TOKEN;
    while (LEX.GetToken(TOKEN))
    {
        cout << TOKEN.tokenType << " " << TOKEN.tokenValue << endl;


    }

    */
   
    //run_tests();


    
   
    string file_name;
  
    printf("\nPlease enter file name: ");

    cin >> file_name;

    CParser parser(file_name);
    
    vector<string> var_vector = parser.get_var_vector();

    SymbolTable test_table = parser.get_symb_table();

    stack<string> datatype_stack = parser.get_stack();
    
    system("pause");
    return 0;

}

