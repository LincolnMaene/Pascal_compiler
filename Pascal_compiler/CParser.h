#pragma once
#include "CToken.h"
#include "CLex.h"
#include "Symbol_table.h"
#include "Registers.h"
#include "Conditional_assembler.h"

typedef struct CParser {

    CLex LEX;//the lexer
    Parser_Token parser_token;
    bool then_done_flag = false;
    RegisterManager register_manager;
    Conditional_assembler * cond_assembler;
    bool bad_code_flag = false;
    bool special_do_situaation;
    stack<string> curr_op;
    stack<string> param_assembly_instructs;//this stack stores assembler instructions for parameters
    vector<string> var_vector;//list of parameters
    string var_vector_type;//type of variable to add
    int assembler_add_sub;//necessary to prevent assembler crashes by keeping track of edi register
    int param_count;

    bool passed_by_ref = false;//lets the parser know if variables are passed by ref

    SymbolTable symbol_table;

    stack<string> datatype_stack;

    bool literal_added = 0;//lets us know a literal has been added

    string last_word = "";//the last word met by the compiler

    CParser(string file_name) {

        cond_assembler = new Conditional_assembler();

        parser_token.consumed = 1;//we must assume that the token was consumed 
        LEX.get_file_name(file_name);
        int not_empty_file = 1;//check for empty files


        code_file.seekg(0, ios::end);
        if (code_file.tellg() == 0) {
            not_empty_file = 0;
        }

        code_file.clear();
        code_file.seekg(0);

        if (not_empty_file && parse_code() && bad_code_flag == false) {

            bad_code_flag = false;//this helps automate tests
            printf("\n\nGood code\n\n");

        }
        else {

            bad_code_flag = true;//this helps automate tests
            printf("\n\nVery Bad code\n\n");
        }
    }

    void  carry_out_lit_op(string curr_op_str) {
        
        bool operation_carried = false;

        operation_carried = register_manager.carry_out_literral_operation(curr_op_str);
                
        if (operation_carried) {//we pop the operations stack if operation was carried out
            if(curr_op.empty()==false)
                curr_op.pop();
        }
        //curr_op = "";

    }
    
    void back_out_scope() {

        symbol_table.BackOut();
    }
    void create_assembler_code(string tokenType, string tokenValue, bool is_literal) {

        int integer_token=-1;
        int offset=-1;

        if (register_manager.get_curr_index() == 4) {

            bad_code();
        }

        
        
        if (symbol_table.is_global_var(tokenValue, offset)|| is_literal) {

            if (tokenType._Equal("integer") && is_literal) {//we convert to integer
                integer_token = stoi(tokenValue);
                string instruction = "\n\t\tmov ";
                string register_name = register_manager.get_register_name();
                string offset_str = to_string(offset);
                instruction = instruction.append(register_name);

                /*

                if (special case for comparisons) {


                }
                else{

                */
                if (offset != -1) {
                    instruction = instruction.append(",[ebp+");
                    instruction = instruction.append(offset_str);
                    instruction = instruction.append("]");
                }
                else {
                    instruction = instruction.append(",");
                    instruction = instruction.append(tokenValue);

                }

                register_manager.add_to_register(is_literal, integer_token);
                register_manager.push_to_insstruction_stack(instruction);



            }
            else if (tokenType._Equal("word") && is_literal == false) {

                string instruction = "\n\t\tmov ";
                string register_name = register_manager.get_register_name();
                instruction = instruction.append(register_name);
                string offset_str = to_string(offset);
                if (offset != -1) {
                    instruction = instruction.append(",[ebp+");
                    instruction = instruction.append(offset_str);
                    instruction = instruction.append("]");
                }
                else {
                    instruction = instruction.append(",");
                    instruction = instruction.append(tokenValue);

                }

                register_manager.add_to_register(is_literal, -1);
                register_manager.push_to_insstruction_stack(instruction);

            }

        }
        else if (symbol_table.is_param_copy(tokenValue, offset)) {

            string instruction = "\n\t\tmov ";
            string register_name = register_manager.get_register_name();
            instruction = instruction.append(register_name);
            string offset_str = to_string(offset);
            if (offset != -1) {
                instruction = instruction.append(",[edi+");
                instruction = instruction.append(offset_str);
                instruction = instruction.append("]");
            }
            else {
                instruction = instruction.append(",");
                instruction = instruction.append(tokenValue);

            }

            register_manager.add_to_register(is_literal, -1);
            register_manager.push_to_insstruction_stack(instruction);
        }
        else if (symbol_table.is_param_ref(tokenValue, offset)) {

            string instruction = "\n\t\tmov ";
            string register_name = register_manager.get_register_name();
            instruction = instruction.append(register_name);
            string offset_str = to_string(offset);
            if (offset != -1) {

                string pre_instruction = "\n\t\tmov esi, [edi + ";
                pre_instruction.append(offset_str);
                pre_instruction.append("]\n");

                instruction = instruction.append(",[esi]");

                instruction = pre_instruction.append(instruction);
                
            }
            else {
                instruction = instruction.append(",");
                instruction = instruction.append(tokenValue);

            }

            register_manager.add_to_register(is_literal, -1);
            register_manager.push_to_insstruction_stack(instruction);
        }
        else if (symbol_table.is_local_var(tokenValue, offset)) {

            offset = offset + 4;
            string instruction = "\n\t\tmov ";
            string register_name = register_manager.get_register_name();
            instruction = instruction.append(register_name);
            string offset_str = to_string(offset);
            if (offset != -1) {
                instruction = instruction.append(",[edi-");
                instruction = instruction.append(offset_str);
                instruction = instruction.append("]");
            }
            else {
                instruction = instruction.append(",");
                instruction = instruction.append(tokenValue);

            }

            register_manager.add_to_register(is_literal, -1);
            register_manager.push_to_insstruction_stack(instruction);

        }

    }
    stack<string> get_stack() {

        return this->datatype_stack;
    }
    SymbolTable get_symb_table() {

        return symbol_table;
    }
    void add_variables(string var_type) {

        int size = -1;

        if (var_type._Equal("integer"))
            size = 4;
        else
            size = 1;

        for (int i = 0; i < var_vector.size(); i++) {

            symbol_table.AddVariable(var_vector[i], var_type, size);
        }
        
        var_vector.clear();
    }

    void add_param(string var_type) {

        bool passed_by_val=true;


        if (passed_by_ref) {
            passed_by_val = false;
            passed_by_ref = false;

        }

       for (int i = 0; i < var_vector.size(); i++) {

           if (symbol_table.add_parameters(var_vector[i], var_type, passed_by_val) == false)
               bad_code();
        }

        var_vector.clear();

        passed_by_ref = false;

    }
    vector<string> get_var_vector() {

        return this->var_vector;
    }

    void set_ref_pass_true() {

        passed_by_ref = true;
    }

    void set_ref_pass_false() {

        passed_by_ref = false;
    }

    void number_5() {//adds parameters to symbol table


        if (var_vector.empty() == false) {

            if (parser_token.tokenType._Equal(INTEGER) || parser_token.tokenValue._Equal(INTEGER))
                add_param("integer");
            else if (parser_token.tokenType._Equal(CHAR) || parser_token.tokenValue._Equal(CHAR))
                add_param("char");
            else if(parser_token.tokenType._Equal(BOOLEAN) || parser_token.tokenValue._Equal(BOOLEAN))
                add_param("boolean");
            else {

                cout << "\n!!Datatype unknown!!!\n";

                bad_code();
            }
        }

        set_ref_pass_false();

       
    }

    void number_2() {//adds variables to symbol table

        if (var_vector.empty() == false) {

            if (parser_token.tokenType._Equal(INTEGER) || parser_token.tokenValue._Equal(INTEGER))
                add_variables("integer");
            else if (parser_token.tokenType._Equal(CHAR) || parser_token.tokenValue._Equal(CHAR))
                add_variables("char");
            else if (parser_token.tokenType._Equal(BOOLEAN) || parser_token.tokenValue._Equal(BOOLEAN))
                add_variables("boolean");
            else {
                cout << "\n!!!token type unknown!!!\n";

                bad_code();
            }
        }
    }

    void number_8() {//Check to see if the word is in the symbol table.  (FindVariable) Push datatype onto the stack.
        
        bool word_exists = false;

        int off_set = -1;

        string var_name = parser_token.tokenValue;
        string var_type = "";//doesn't work, need some way to find a var's datatype from symbol table
        
        if (this->symbol_table.FindVariable(var_name, off_set)) {

            word_exists = true;
            this->symbol_table.find_var_type(var_name, var_type);
        }
        

        if (word_exists == false) {
            
            //cout << "\nvar not found\n";

            cout << "\n!! identifier not found!!!\n";
            bad_code();
        }
        else {


            if(var_type._Equal("procedure")==false)//no point putting procedure on stack
                this->datatype_stack.push(var_type);
            //cout << "\nnum_8 not yet fully implemented!!!!\n";
        }
    }
    void print_to_file(string instruction1, string instruction2, string operation) {

        string* register_to_use = register_manager.get_last_2_registers();


        if (operation._Equal("*")) {

            string instruction3 = "imul ";
            if (instruction1._Equal("") == false) {
                //cout << instruction1 << endl;
                print_to_assembly_file(instruction1);
            }

            if (instruction2._Equal("") == false) {
                //cout << instruction2 << endl;
                print_to_assembly_file(instruction2);
            }
            instruction3 = instruction3.append(register_to_use[1]);
            instruction3 = instruction3.append(",");
            instruction3 = instruction3.append(register_to_use[0]);

            print_to_assembly_file(instruction3);

            //cout << "imul " << register_to_use[1] << "," << register_to_use[0] << endl;

        }
        else if (operation._Equal("+")) {
            string instruction3 = "add ";
            if (instruction1._Equal("") == false) {
                //cout << instruction1 << endl;
                print_to_assembly_file(instruction1);
            }

            if (instruction2._Equal("") == false) {
                //cout << instruction2 << endl;
                print_to_assembly_file(instruction2);
            }


            instruction3 = instruction3.append(register_to_use[1]);
            instruction3 = instruction3.append(",");
            instruction3 = instruction3.append(register_to_use[0]);

            print_to_assembly_file(instruction3);
            //cout << "add " << register_to_use[1] << "," << register_to_use[0] << endl;
        }
        else if (operation._Equal("-")) {

            string instruction3 = "sub ";
            if (instruction1._Equal("") == false) {
                //cout << instruction1 << endl;
                print_to_assembly_file(instruction1);
            }

            if (instruction2._Equal("") == false) {
                //cout << instruction2 << endl;
                print_to_assembly_file(instruction2);
            }

            instruction3 = instruction3.append(register_to_use[1]);
            instruction3 = instruction3.append(",");
            instruction3 = instruction3.append(register_to_use[0]);

            print_to_assembly_file(instruction3);
            //cout << "sub " << register_to_use[1] << "," << register_to_use[0] << endl;
        }
        else if (operation._Equal("/")) {


            string instruction3 = "\n\t\tpush edx\n\t\tcdq\n\t\tidiv ";

            if (instruction1._Equal("") == false) {
                //cout << instruction1 << endl;
                print_to_assembly_file(instruction1);
            }

            if (instruction2._Equal("") == false) {
                //cout << instruction2 << endl;
                print_to_assembly_file(instruction2);
            }

            instruction3 = instruction3.append(register_to_use[0]);
            instruction3 = instruction3.append("\n\t\tpop edx\n\t\t");
            //instruction3 = instruction3.append(register_to_use[0]);

            print_to_assembly_file(instruction3);

            //cout << "idiv " << register_to_use[1] << "," << register_to_use[0] << endl;
        }

    }
    void write_last_2_instruct(string operation) {

        bool write = true;
        int instruction_size = register_manager.get_instruction_stack().size();

        if (instruction_size < 2) {

            write = false;
        }

        if (write == true) {

            string instruction2 = register_manager.get_last_instruction();
            string instruction1 = "";
            if (!register_manager.pop_last_instruction()) {

                cout << "\n\n!!!Stack Error 0!!!\n\n";
            }
            instruction1 = register_manager.get_last_instruction();

            if (!register_manager.pop_last_instruction()) {

                cout << "\n\n!!!Stack Error 1!!!\n\n";
            }
           
            print_to_file(instruction1, instruction2, operation);

            register_manager.free_register();
            register_manager.free_register();
            register_manager.add_to_register(false, -1);
            register_manager.push_to_insstruction_stack("");

            if(curr_op.empty()==false)
                curr_op.pop();

        }
    }

    void number_9(string operation) { //(9) Pop two datatypes off of the stack.
                       //Make sure you can do the current operation on the two datatypes.Put the new datatype onto the stack.
        if (parser_token.tokenValue._Equal("then") && then_done_flag||special_do_situaation) {//prevents a glitch due to now likely unsolved structural code issues

            datatype_stack.push("boolean");
            datatype_stack.push("boolean");
                   
        }
        string datatype_0 = "";
        string datatype_1 = "";

       
        if (this->datatype_stack.empty() == false) {
            datatype_0 = this->datatype_stack.top();//essentially i need to remove both elements from stack
            this->datatype_stack.pop();
        }
        
        
        if (this->datatype_stack.empty() == false) {

            datatype_1 = this->datatype_stack.top();
            this->datatype_stack.pop();
        }

        if (operation._Equal("<") || operation._Equal(">") || operation._Equal("=")) {

            if (datatype_0._Equal(datatype_1)) {//if the two vars are same datatype

                if (literal_added == 0)
                    this->datatype_stack.push("boolean");
                else
                    literal_added = 0;

            }
            else {

                bad_code();
            }

        } 
        else  if (operation._Equal("+") || operation._Equal("-") || operation._Equal("*") || operation._Equal("/")) {

            if (datatype_0._Equal("integer") && datatype_1._Equal("integer")) {//if the two vars are integer

                string write_op = "";

                if (curr_op.empty() == false)
                    write_op = curr_op.top();

                write_last_2_instruct(write_op);
                this->datatype_stack.push("integer");

            }
            else {

                bad_code();
            }

        }
        else  if (operation._Equal("or") || operation._Equal("and")) {

            if (datatype_0._Equal("boolean") && datatype_1._Equal("boolean")) {//if the two vars are integer

                this->datatype_stack.push("boolean");

            }
            else {

                bad_code();
            }

        }
        else {

            bad_code();
        }

        if (parser_token.tokenValue._Equal("then") && then_done_flag) {//prevents a glitch due to now likely unsolved structural code issues

            while (datatype_stack.size() > 1) {
                datatype_stack.pop();
            }
            

        }
        

    }
    void write_assembly_prog_header() {

        ofstream out;

        out.open("assembler.cpp");


        out << "#include <iostream>" << endl;
        out << "using namespace std;" << endl;
        out << "char DataSegment[65536];" << endl;
        out << "int main()" << endl;
        out << "{" << endl;
        out << "	_asm{" << endl;
        out << "		pushad         // Assembler header for all programs (Top)" << endl;
      
        out << "		lea ebp, DataSegment" << endl;
        out << "		jmp kmain       // Assembler header for all programs (Bottom)" << endl << endl << endl;

        out.close();
    }

    void print_to_assembly_file(string line) {


        ofstream outfile;

        outfile.open("assembler.cpp", ios::out | ios::app);


        if (outfile) {

            outfile << "		"<<line << endl;

        }
        else {

            outfile << "\n\n!!!!assembler file not found!!!!!n\n";
        }



        outfile.close();



    }

    void write_assembly_prog_footer() {


        ofstream out;

        out.open("assembler.cpp", ios::out | ios::app);

        out << "		\n" << endl;
       
        out << "		popad    // Assembler footer for all programs (Bottom)" << endl;
        out << "	}" << endl;
        out << "return 0;" << endl;
        out << "}" << endl;

        out.close();

    }
    void write_assignment_assembly(string last_word) {

        int offset = -1;
        string instruction_check = "";
        bool did_redundancy_check = false;

        if (datatype_stack.empty()) {
           
            while (register_manager.get_instruction_stack().empty() == false) {
                
                instruction_check = register_manager.get_last_instruction();

                if (instruction_check._Equal("") == false) {

                    print_to_assembly_file(instruction_check);
                    did_redundancy_check = true;
                }

                register_manager.pop_last_instruction();
               

            }

            //register_manager.free_register_all();

        }

        if (symbol_table.is_global_var(last_word,offset)) {

            string instruction = "\n\t\tmov [ebp+";
            string offset_str = to_string(offset);
            string register_name = register_manager.get_last_register_name();
            if (register_name._Equal("") == false) {//we need a valid register name for all this to work
                bool reg_is_lit = register_manager.register_is_literal(register_name);


                instruction = instruction.append(offset_str);
                instruction = instruction.append("],");

                if (reg_is_lit == false || did_redundancy_check==true) {
                    instruction = instruction.append(register_name);
                    register_manager.add_to_register(false, -1);

                    //cout << instruction << endl;
                    print_to_assembly_file(instruction);
                    register_manager.free_register_all();
                }
                else {

                    int reg_value = register_manager.get_register_val(register_name);
                    string str_val = to_string(reg_value);
                    string pre_instruction = "\n\t\tmov ";
                    pre_instruction = pre_instruction.append(register_name);
                    pre_instruction = pre_instruction.append(",");
                    pre_instruction = pre_instruction.append(str_val);
                    //cout << pre_instruction << endl;
                    if (pre_instruction._Equal(instruction_check) == false) {//avoid duplicates
                        
                        print_to_assembly_file(pre_instruction);

                    }

                    if (register_manager.get_instruction_stack().empty() == false) {

                        if (pre_instruction._Equal(register_manager.get_instruction_stack().top())) {

                            register_manager.pop_last_instruction();
                        }
                    }
                    instruction = instruction.append(register_name);
                    //cout << instruction << endl;
                    print_to_assembly_file(instruction);

                    if (register_manager.get_instruction_stack().empty() == false) {

                        if (instruction._Equal(register_manager.get_instruction_stack().top())) {

                            register_manager.pop_last_instruction();
                        }
                    }

                    register_manager.free_register();

                }
            }
            
        }
        else if (symbol_table.is_param_copy(last_word, offset)) {


            string instruction = "\n\t\tmov [edi+";
            string offset_str = to_string(offset);
            string register_name = register_manager.get_last_register_name();
            if (register_name._Equal("") == false) {//we need a valid register name for all this to work
                bool reg_is_lit = register_manager.register_is_literal(register_name);


                instruction = instruction.append(offset_str);
                instruction = instruction.append("],");

                if (reg_is_lit == false || did_redundancy_check == true) {
                    instruction = instruction.append(register_name);
                    register_manager.add_to_register(false, -1);

                    //cout << instruction << endl;
                    print_to_assembly_file(instruction);
                }
                else {

                    int reg_value = register_manager.get_register_val(register_name);
                    string str_val = to_string(reg_value);
                    string pre_instruction = "\n\t\tmov ";
                    pre_instruction = pre_instruction.append(register_name);
                    pre_instruction = pre_instruction.append(",");
                    pre_instruction = pre_instruction.append(str_val);
                    //cout << pre_instruction << endl;

                    if(pre_instruction._Equal(instruction_check)==false)//trying not to repeat instructions pointlessly
                    print_to_assembly_file(pre_instruction);

                    if (register_manager.get_instruction_stack().empty() == false) {

                        if (pre_instruction._Equal(register_manager.get_instruction_stack().top())) {

                            register_manager.pop_last_instruction();
                        }
                    }
                    instruction = instruction.append(register_name);
                    //cout << instruction << endl;
                    print_to_assembly_file(instruction);

                    if (register_manager.get_instruction_stack().empty() == false) {

                        if (instruction._Equal(register_manager.get_instruction_stack().top())) {

                            register_manager.pop_last_instruction();
                        }
                    }

                    register_manager.free_register();

                }
            }
            
        }
        else if (symbol_table.is_param_ref(last_word, offset)) {


        string instruction = "\n\t\tmov [esi],";
        string offset_str = to_string(offset);
        string register_name = register_manager.get_last_register_name();
        if (register_name._Equal("") == false) {//we need a valid register name for all this to work
            bool reg_is_lit = register_manager.register_is_literal(register_name);


            //instruction = instruction.append(offset_str);
            //instruction = instruction.append("],");

            if (reg_is_lit == false || did_redundancy_check == true) {

                string pre_instruction = "\n\t\tmov esi, [edi +";
                pre_instruction = pre_instruction.append(offset_str);
                pre_instruction = pre_instruction.append("]");
                instruction = instruction.append(register_name);
                register_manager.add_to_register(false, -1);

                //cout << instruction << endl;
                print_to_assembly_file(pre_instruction);
                print_to_assembly_file(instruction);
            }
            else {

                int reg_value = register_manager.get_register_val(register_name);
                string str_val = to_string(reg_value);
                string pre_instruction = "\n\t\tmov ";
                pre_instruction = pre_instruction.append(register_name);
                pre_instruction = pre_instruction.append(",");
                pre_instruction = pre_instruction.append(str_val);
                //cout << pre_instruction << endl;
                print_to_assembly_file(pre_instruction);

                if (register_manager.get_instruction_stack().empty() == false) {

                    if (pre_instruction._Equal(register_manager.get_instruction_stack().top())) {

                        register_manager.pop_last_instruction();
                    }
                }
                instruction = instruction.append(register_name);
                //cout << instruction << endl;
                print_to_assembly_file(instruction);

                if (register_manager.get_instruction_stack().empty() == false) {

                    if (instruction._Equal(register_manager.get_instruction_stack().top())) {

                        register_manager.pop_last_instruction();
                    }
                }

                register_manager.free_register();

            }
        }
            

        }
        else if (symbol_table.is_local_var(last_word, offset)) {//if we're assigning to a local variable

        offset = offset+4;
        string instruction = "\n\t\tmov [edi-";
        string offset_str = to_string(offset);
        string register_name = register_manager.get_last_register_name();
        if (register_name._Equal("") == false) {//we need a valid register name for all this to work
            bool reg_is_lit = register_manager.register_is_literal(register_name);


            instruction = instruction.append(offset_str);
            instruction = instruction.append("],");

            if (reg_is_lit == false || did_redundancy_check==true) {
                instruction = instruction.append(register_name);
                register_manager.add_to_register(false, -1);

                //cout << instruction << endl;
                print_to_assembly_file(instruction);

                if (datatype_stack.empty()) {

                    register_manager.free_register_all();


                }
            }
            else {

                int reg_value = register_manager.get_register_val(register_name);
                string str_val = to_string(reg_value);
                string pre_instruction = "\n\t\tmov ";
                pre_instruction = pre_instruction.append(register_name);
                pre_instruction = pre_instruction.append(",");
                pre_instruction = pre_instruction.append(str_val);
                //cout << pre_instruction << endl;
                print_to_assembly_file(pre_instruction);

                if (register_manager.get_instruction_stack().empty() == false) {

                    if (pre_instruction._Equal(register_manager.get_instruction_stack().top())) {

                        register_manager.pop_last_instruction();
                    }
                }
                instruction = instruction.append(register_name);
                //cout << instruction << endl;
                print_to_assembly_file(instruction);

                if (register_manager.get_instruction_stack().empty() == false) {

                    if (instruction._Equal(register_manager.get_instruction_stack().top())) {

                        register_manager.pop_last_instruction();
                    }
                }

                register_manager.free_register();

            }
        }



        }

        register_manager.free_register_all();

    }
    void number_10() {// Pop two datatypes off of the stack. Make sure the two datatypes are the same.			

        string datatype_0 = "";
        string datatype_1 = "";


        if (this->datatype_stack.empty() == false) {
            datatype_0 = this->datatype_stack.top();//essentially i need to remove both elements from stack
            this->datatype_stack.pop();
        }


        if (this->datatype_stack.empty() == false) {

            datatype_1 = this->datatype_stack.top();
            this->datatype_stack.pop();
        }


        if (datatype_0._Equal(datatype_1)==false) {//if the two vars are same datatype

            cout << "\n!!!type mismatch!!!\n";
            bad_code();

        }
        else {

           write_assignment_assembly(last_word);
        }
        


    }

    void token_alert(string value) {

        if (parser_token.tokenValue._Equal(value)) {

            cout << "\n\nNeeded token detected\n\n";
        }
    }
   
    void grab_new_token(CToken* token) {

        if (parser_token.consumed == 1) {//no point in getting token if prev was not consumedd

            LEX.GetToken(*token);

            if (!(*token).tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = (*token).tokenType;

                parser_token.tokenValue = (*token).tokenValue;
            }
            else {




                parser_token.tokenValue = "";
            }

        }

        //token_alert("a");

    }
    void consume() {

        parser_token.consumed = 1;
    }

    bool parse_code() {//we parse code, true if good code, false if not







        return program();

        //LEX.GetToken(token);

       /*
        if (parser_token.consumed)
            return true;
        else
            return false;

            */


            /*
            E();

            return true;

            */
    }


    bool program() {

        special_do_situaation = false;
        write_assembly_prog_header();
        CToken token;

        block();

       
       
        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }

        if (parser_token.tokenValue == ".") {

            consume();
            write_assembly_prog_footer();
            return true;

        }
        else {

            bad_code();
            return false;
        }


    }
    bool get_bad_flag() {

        return this->bad_code_flag;
    }
    void bad_code() {//tells the parser this is bad code

        parser_token.consumed = 0;

        bad_code_flag = true;

        cout << "\nbad code!!!!\n";


    }
    void var_list() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }


        if (parser_token.tokenValue == ",") {

            consume();

            grab_new_token(&token);

            if (parser_token.tokenType == WORD) {

                consume();

                store_var_word(parser_token.tokenValue);

                var_list();

            }
            else {

                bad_code();
            }
        }


    }

    void datat_type() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }

        if (parser_token.tokenValue._Equal(INTEGER)) {

            consume();


        }
        else if (parser_token.tokenValue._Equal(CHAR)) {

            consume();
            
           
        }
        else if (parser_token.tokenValue._Equal(BOOLEAN)) {

            consume();
            
        }
        else {

            bad_code();
        }

    }

    void m_var() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }

        if (parser_token.tokenType._Equal(WORD)) {

            consume();

            store_var_word(parser_token.tokenValue);

            var_list();

            grab_new_token(&token);

            if (parser_token.tokenValue._Equal(":")) {

                consume();

                datat_type();

                number_2();

                grab_new_token(&token);

                if (parser_token.tokenValue._Equal(";")) {

                    consume();

                    m_var();

                }
                else {

                    bad_code();
                }

            }
            else {

                bad_code();

            }

        }

    }

    void ovar() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }

        if (parser_token.tokenValue._Equal("var")) {
            set_ref_pass_true();
            consume();
        }

       

    }

    void m_param() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }


        if (parser_token.tokenValue._Equal(";")) {

            consume();

            ovar();

            grab_new_token(&token);

            if (parser_token.tokenType._Equal(WORD)) {

                consume();

                store_var_word(parser_token.tokenValue);

                var_list();

                grab_new_token(&token);


                if (parser_token.tokenValue._Equal(":")) {

                    consume();

                    datat_type();
                    number_5();

                    m_param();

                }
                else {

                    bad_code();

                }
            }
            else {

                bad_code();
            }
        }

    }
    void param() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }


        if (parser_token.tokenValue._Equal("var")) {

            set_ref_pass_true();//we need to know things have been passed by ref
            consume();

            grab_new_token(&token);

            if (parser_token.tokenType._Equal(WORD)) {



                consume();

                store_var_word(parser_token.tokenValue);


                var_list();

                grab_new_token(&token);

                if (parser_token.tokenValue._Equal(":")) {

                    consume();

                    datat_type();

                    number_5();
                    m_param();

                }
                else {

                    bad_code();
                }


            }
            else {

                bad_code();
            }



        }
        else if (parser_token.tokenType._Equal(WORD)) {

            consume();

            store_var_word(parser_token.tokenValue);

            var_list();

            grab_new_token(&token);

            if (parser_token.tokenValue._Equal(":")) {

                consume();

                datat_type();
                number_5();
                m_param();


            }
            else {

                bad_code();
            }

        }

    }

    void store_var_word(string var_name) {//stores variable words in vector

        this->var_vector.push_back(var_name);

    }


    void store_var_type(string var_type) {//stores variable words in vector

        this->var_vector_type = var_type;

    }

    void clear_var_vectors() {

        this->var_vector.clear();

        this->var_vector_type.clear();
    }

    void set_func_ret_datatype() {
        string datatype = "";
        
        if (parser_token.tokenValue._Equal(INTEGER)) {
            
            datatype = "integer";
        }
        else if (parser_token.tokenValue._Equal(BOOLEAN)) {

            datatype = "boolean";
        }
        else {
            datatype = "char";
        }

        this->symbol_table.set_function_return_type(datatype);
    }
    void print_proc_assembly_header() {

        ofstream out;
        out.open("assembler.cpp", ios::out | ios::app);


       
        out << "		// Assembler header for all proc (Top)" << endl;
       
        out << "		pushad" << endl << endl;
        
      

        out.close();
    }


    void print_proc_assembly_footer() {

        ofstream out;
        out.open("assembler.cpp", ios::out | ios::app);
        

        

        out << "		\n" << endl;
      
        out << "		popad" << endl << endl;

        


        out.close();
    }
    void p_f_v() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one

            grab_new_token(&token);
        }


        if (parser_token.tokenValue._Equal("var")) {

            parser_token.consumed = 1;//tell them we consumed the token, get next one

            grab_new_token(&token);

            if (parser_token.tokenType._Equal(WORD)) {


                store_var_word(parser_token.tokenValue);
                parser_token.consumed = 1;

                var_list();

                //grab_new_token(&token);

                



                if (parser_token.tokenValue == ":") {

                    consume();

                    datat_type();

                    number_2();
                    

                    grab_new_token(&token);

                    if (parser_token.tokenValue == ";") {

                        consume();

                        m_var();



                        p_f_v();

                    }
                    else {

                        bad_code();
                    }



                }
                else {

                    bad_code();
                }
            }
            else {

                bad_code();
            }

           


        }
        else  if (parser_token.tokenValue._Equal("procedure")) {
            string proc_name = "";//this stores the procedure's name for useful lookups
            param_count = -1; 
            
            consume();

            grab_new_token(&token);

            if (parser_token.tokenType._Equal(WORD)) {
                
                string assemb_instruct = "";
                assemb_instruct = parser_token.tokenValue;
                proc_name = assemb_instruct;
                assemb_instruct = assemb_instruct.append(":\n");
               
                   


                print_to_assembly_file(assemb_instruct);
                print_to_assembly_file("push edi // save old edi");
                print_to_assembly_file("mov edi, esp    // Put the current stack pointer into edi");
                consume();

                if (!symbol_table.AddProcFunc(parser_token.tokenValue, "procedure"))
                    bad_code();
                

                grab_new_token(&token);

                if (parser_token.tokenValue._Equal("(")) {

                    consume();

                    param();

                    grab_new_token(&token);

                    if (parser_token.tokenValue._Equal(")")) {

                        consume();
                    

                        param_count = symbol_table.param_count();//assembler needs to know how many parameters func has
                        

                      
                       

                    
                        grab_new_token(&token);

                        if (parser_token.tokenValue._Equal(";")) {

                            consume();

                            block();


                            grab_new_token(&token);

                            if (parser_token.tokenValue._Equal(";")) {

                                consume();

                                back_out_scope();

                                p_f_v();

                            }
                            else {

                                bad_code();
                            }

                        }
                        else {

                            bad_code();
                        }

                    }
                    else {

                        bad_code();
                    }

                }
                else {

                    bad_code();
                }
            }
            else {

                bad_code();
            }
           

           



        }
        else if (parser_token.tokenValue._Equal("function")) {


            consume();
            grab_new_token(&token);

            if (parser_token.tokenType._Equal(WORD)) {

                consume();

                if (!symbol_table.AddProcFunc(parser_token.tokenValue, "function"))
                    bad_code();
                grab_new_token(&token);


                if (parser_token.tokenValue._Equal("(")) {

                    consume();
                    param();
                    grab_new_token(&token);

                    if (parser_token.tokenValue._Equal(")")) {

                        consume();
                        grab_new_token(&token);

                        if (parser_token.tokenValue._Equal(":")) {

                            consume();

                            datat_type();

                            set_func_ret_datatype();
                            grab_new_token(&token);


                            if (parser_token.tokenValue._Equal(";")) {

                                consume();

                                block();
                                grab_new_token(&token);

                                if (parser_token.tokenValue._Equal(";")) {

                                    consume();
                                    back_out_scope();
                                    p_f_v();

                                }
                                else {

                                    bad_code();
                                }

                            }
                            else {

                                bad_code();

                            }


                        }
                        else {

                            bad_code();



                        }


                    }
                    else {

                        bad_code();
                    }


                }
                else {
                    bad_code();
                }

            }
            else {

                bad_code();
            }



        }



    }

    

    void block() {

    
        p_f_v();

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);

            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;

                parser_token.tokenValue = token.tokenValue;
            }
            else {




                parser_token.tokenValue = "";
            }
        }


        if (parser_token.tokenValue._Equal("begin")) {

            if (symbol_table.is_in_procedure() == false) {

                print_to_assembly_file("kmain:\n");
            }
            else if (symbol_table.is_in_procedure() == true) {//we need to do the header for functions/procedures

                int local_var_count = -1;//assembler needs to know how many local vars there are
                local_var_count = symbol_table.get_local_var_count();

                if (local_var_count > 0) {//we need to make space on stack for local vars

                    int space_needed = symbol_table.get_space_need();
                    assembler_add_sub = space_needed;
                    string str_space = to_string(space_needed);
                    string space_instruct = "\n\t\tsub esp,";
                    space_instruct = space_instruct.append(str_space);
                    space_instruct = space_instruct.append("\n\t\t");

                    print_to_assembly_file(space_instruct);

                   

                }

                print_proc_assembly_header();

                

            }
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound (";
            statement();

            m_statement();





            if (parser_token.tokenValue._Equal("end")) {

                parser_token.consumed = 1;//tell them we consumed the token, get next one

                if (symbol_table.is_in_procedure()) {

                    int local_var_count = -1;//assembler needs to know how many local vars there are
                    local_var_count = symbol_table.get_local_var_count();
                    if (symbol_table.has_local_variables()) {

                        int space_needed = symbol_table.total_local_variable_space();

                        if (space_needed != assembler_add_sub) {

                            space_needed = assembler_add_sub;
                        }
                        string str_space = to_string(space_needed);
                        string space_instruct = "\n\t\tadd esp,";
                        space_instruct = space_instruct.append(str_space);
                        space_instruct = space_instruct.append("\n\t\t");

                        print_proc_assembly_footer();

                        print_to_assembly_file(space_instruct);


                    }



                    int quant_removed_from_stack = param_count * 4;//how much assembler removes from stack
                    string remove_str = to_string(quant_removed_from_stack);
                    string remove_instruct = "ret ";
                    remove_instruct = remove_instruct.append(remove_str);
                    remove_instruct = remove_instruct.append("\n\n");

                    if (symbol_table.has_local_variables() == false) {

                        print_proc_assembly_footer();


                    }
                    print_to_assembly_file("\n");
                    print_to_assembly_file("pop edi// resore old edi");
                    print_to_assembly_file(remove_instruct);

                    register_manager.free_register_all();

                }


            }
            else {

                bad_code();
            }

        }
        else {

            bad_code();
        }
    }

    void compute_param_copy_assembly() {

        int instruction_stack_size = register_manager.get_instruction_stack().size();

        string stack_instruction = "";

        if (instruction_stack_size > 0) {

            stack_instruction = register_manager.get_last_instruction();
            register_manager.pop_last_instruction();

            if (register_manager.get_instruction_stack().empty()==false  && register_manager.get_instruction_stack().top()._Equal("")) {//contigency for empty instruction on stack

                register_manager.pop_last_instruction();

               
            }
            
            stack_instruction = stack_instruction.append("\n\t\tpush eax");

            param_assembly_instructs.push(stack_instruction);

        }
    }
    void compute_param_ref_assembly(string offset) {


        string instruction = "\n\t\tmov eax,";
        instruction = instruction.append(offset);
        instruction = instruction.append("\n\t\t");
        instruction = instruction.append("add  eax,ebp");
        instruction = instruction.append("\n\t\t");
        instruction = instruction.append("push eax");
        instruction = instruction.append("\n\t\t");


        param_assembly_instructs.push(instruction);


    }
    void consume_param(string func_name) {

       
        CToken token;
        int param_num = symbol_table.find_param_num(func_name);//total number of params for the function
        int curr_param = 0;
        if (param_num==-1) {
            bad_code();
        }

        while (curr_param < param_num) {

            register_manager.free_register_all();

            string datatype="";
            int val_or_ref = -1;
            int off_set = -1;

            this->symbol_table.find_param_info(func_name, datatype, val_or_ref, off_set, curr_param);

            if (val_or_ref == 2) {//if it passed by ref

                if (parser_token.tokenValue._Equal("(")) {

                    consume();
                    grab_new_token(&token);
                }

                //cout << "\n\n!!!!Pass by ref not yet implemented!!!\n\n";
                if (parser_token.tokenType._Equal(WORD)) {

                    string datatype_to_push = "";//essentially i need to find the datatype of the variable passed by ref and push it on stack
                    string var_name = parser_token.tokenValue;
                    symbol_table.find_var_type(var_name, datatype_to_push);

                    this->datatype_stack.push(datatype_to_push);

                    int passed_offset = -1; //offset of variable passed by ref
                    string str_offset = "";
                    symbol_table.FindVariable(var_name, passed_offset);

                    str_offset = to_string(passed_offset);
                    compute_param_ref_assembly(str_offset);


                    consume();

                    grab_new_token(&token);

                    if (this->datatype_stack.empty() || this->datatype_stack.empty() == false && this->datatype_stack.top()._Equal(datatype)==false) {

                        bad_code();
                    }
                    else {

                        this->datatype_stack.pop();

                    }

                }
                else {

                    bad_code();
                }
            }
            else {

                string var_name = "";
                int passed_offset = -1; //offset of variable passed by value
                

                expression();
                
                compute_param_copy_assembly();

                if (this->datatype_stack.empty() || this->datatype_stack.empty() == false && this->datatype_stack.top()._Equal(datatype) == false) {

                    bad_code();
                }
                else {

                    this->datatype_stack.pop();
                    
                }

            }

            curr_param++;

            if (curr_param < param_num) {//gets us past ","

                if (parser_token.tokenValue._Equal(",")) {
                    consume();
                    grab_new_token(&token);

                }
                else {
                    bad_code();
                }

            }
        }

        while (param_assembly_instructs.empty() == false) {

            string line = param_assembly_instructs.top();
            print_to_assembly_file(line);
            param_assembly_instructs.pop();
                
        }
        string last_instruction = "\n\t\tcall ";

        last_instruction = last_instruction.append(func_name);

        last_instruction = last_instruction.append("\n\t\t");

        print_to_assembly_file(last_instruction);

        while (register_manager.get_instruction_stack().empty() == false) {

            register_manager.pop_last_instruction();
        }


    }



    void do_end_if_jmp() {

        string instruct = "\n\t\tJMP ENDIF";

        string if_counter = ""; 
        if_counter=cond_assembler->stack_top().back();

        instruct = instruct.append(if_counter);


        print_to_assembly_file(instruct);


    }

    void print_while_lable() {
        string cond_stack = "w";
        string instructs = "\n\t\tWHILE";
        string while_count = to_string(cond_assembler->get_while_count());
        instructs = instructs.append(while_count);
        cond_stack = cond_stack.append(while_count);
        cond_assembler->inc_while_count();
        cond_assembler->push_cond_stack(cond_stack);
        instructs = instructs.append(":");


        print_to_assembly_file(instructs);
   

    }

    void statement() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);

            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;

                parser_token.tokenValue = token.tokenValue;
            }
            else {




                parser_token.tokenValue = "";
            }
        }


        if (parser_token.tokenType._Equal("word")) {

            last_word = parser_token.tokenValue;

            number_8();
            parser_token.consumed = 1;//tell them we consumed the token, get next one


            if (parser_token.consumed) {//if the token was consumed previously get new one
                LEX.GetToken(token);

                if (!token.tokenType._Equal("")) {
                    parser_token.consumed = 0;
                    parser_token.tokenType = token.tokenType;

                    parser_token.tokenValue = token.tokenValue;
                }
                else {




                    parser_token.tokenValue = "";
                }
            }

            if (parser_token.tokenValue._Equal("(")) {

                consume();

                consume_param(last_word);

                grab_new_token(&token);

                if (parser_token.tokenValue._Equal(")")) {

                    consume();
                }
                else {

                    bad_code();
                }
               

            }

            else if (parser_token.tokenType._Equal("assignment")) {

                parser_token.consumed = 1;

                expression();

                number_10();

            }
            else {

                bad_code();
            }


        }
        else if (parser_token.tokenValue._Equal("begin")) {

            parser_token.consumed = 1;//tell them we consumed the token, get next one


            statement();
            m_statement();


            if (parser_token.tokenValue._Equal("end")) {

                parser_token.consumed = 1;//tell them we consumed the token, get next one
            }
            else {

                bad_code();
            }

        }
        else if (parser_token.tokenValue._Equal("if")) {

            register_manager.free_register_all();
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            cond_assembler->inc_if_count();

            expression();

            if (parser_token.tokenValue._Equal("then")) {




                string compar_var_type = "";

                if (datatype_stack.empty() == false) {
                    compar_var_type = datatype_stack.top();

                    if (compar_var_type._Equal("boolean"))//we need to make sure a conditional was passed for comparison
                        datatype_stack.pop();
                    else
                        bad_code();
                }
                parser_token.consumed = 1;//tell them we consumed the token, get next one

                statement();

                do_end_if_jmp();

                if_function_prime();
            }
            else {

                bad_code();
            }


        }
        else if (parser_token.tokenValue._Equal("while")) {


            parser_token.consumed = 1;//tell them we consumed the token, get next one

            print_while_lable();


            expression();

        
            do_while_comp_jump();
            

            if (parser_token.tokenValue._Equal("do")) {

                parser_token.consumed = 1;//tell them we consumed the token, get next one


                statement();

                do_while_end_jump();


            }
            else {

                bad_code();

            }




        }





    }

    void m_statement() {

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);

            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;

                parser_token.tokenValue = token.tokenValue;
            }
            else {




                parser_token.tokenValue = "";
            }
        }



        if (parser_token.tokenValue._Equal(";")) {

            parser_token.consumed = 1;//tell them we consumed the token, get next one

            statement();

            m_statement();
        }

    }

    void if_function() {


    }
    void print_else_end_label() {

        string instruct = "\n\t\tEND";
        string if_counter = ""; 
        if_counter=cond_assembler->stack_top();
        instruct = instruct.append(if_counter);
        instruct = instruct.append(":");


        print_to_assembly_file(instruct);
    }
    void print_else_label() {

        string instruct = "\n\t\tELSE";
        string if_counter = ""; 
        if_counter=cond_assembler->stack_top();
        instruct = instruct.append(if_counter);
        instruct = instruct.append(":");


        print_to_assembly_file(instruct);
    }
    void if_function_prime() {


        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);

            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;

                parser_token.tokenValue = token.tokenValue;
            }
            else {




                parser_token.tokenValue = "";
            }
        }

        if (parser_token.tokenValue._Equal("else")) {


            parser_token.consumed = 1;//tell them we consumed the token, get next one

            print_else_label();

            statement();

            print_else_end_label();



        }

    }




    void expression() {

        s_expression();
    }


    void s_expression() {

        E();
        E_prime();
    }

    void E_prime() {//this is really s_expression_prime

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);

            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;

                parser_token.tokenValue = token.tokenValue;
            }
            else {




                parser_token.tokenValue = "";
            }
        }


        if (parser_token.tokenValue._Equal("<")) {


            parser_token.consumed = 1;//tell them we consumed the token, get next one
            
            curr_op.push("<");

            E();

            number_9("<");

            E_prime();



        }
        else if (parser_token.tokenValue._Equal(">")) {


            parser_token.consumed = 1;//tell them we consumed the token, get next one
            curr_op.push(">");

            E();


            number_9(">");

            E_prime();



        }
        else if (parser_token.tokenValue._Equal("=")) {


            parser_token.consumed = 1;//tell them we consumed the token, get next one
            curr_op.push("=");


            E();

            number_9("=");

            E_prime();



        }


    }

    void E() {//this is actually s_expresion_r

        T();
    }

    void T() {//T actually stands for term

        TR();
        T_prime();

    }

    void TR() {//TR is term_r

        F();
    }
    void print_an_orrr() {//prints an or label as needed
        
        if (cond_assembler->or_is_used()) {
            string instruct = "\n\t\tOR";

            string or_count = to_string(cond_assembler->get_or_count());

            instruct = instruct.append(or_count);

            instruct = instruct.append(":");

            cond_assembler->inc_or_count();


            print_to_assembly_file(instruct);
        }
    }
    void T_prime() {//TR is term_prime

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);
            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;

                parser_token.tokenValue = token.tokenValue;
            }

        }

        //cout << "\nT' grabing token type: " + parser_token.tokenType + " , value: " + parser_token.tokenValue + "...";
        if (parser_token.tokenValue._Equal("+")) {

            curr_op.push("+");
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound +";
            TR();

            number_9("+");

            T_prime();
        }
        else if (parser_token.tokenValue._Equal("-")) {
            curr_op.push("-");
            parser_token.consumed = 1;
            //cout << "\nfound -";
            TR();

            number_9("-");

            T_prime();

        }
        else if (parser_token.tokenValue._Equal("or")) {

            parser_token.consumed = 1;
            do_or_comp_jump();

            print_an_orrr();

           


            //cout << "\nfound -";
            TR();
            number_9("or");
            T_prime();

        }
    }

    void F() {//F is fact

        FR();

        F_prime();
    }

    void do_while_end_jump() {

        string jmp_instruct = "\n\t\tJMP WHILE";
        string label_instruct = "\n\t\t";
        string w = cond_assembler->stack_top();
        string index = "";
        
        index=index+w.back();

        label_instruct = label_instruct.append(w);
        label_instruct = label_instruct.append("end:");

       
        jmp_instruct = jmp_instruct.append(index);



        print_to_assembly_file(jmp_instruct);
        print_to_assembly_file(label_instruct);

        cond_assembler->stack_pop();

    }

    void  do_while_comp_jump() {//writes the jump assembly code for comparisons

        special_do_situaation = false;

        string last_comp = cond_assembler->get_last_comp();
        string or_label_instruct = "\n\t\tOR";
        string or_counter =to_string(cond_assembler->get_or_count());
        or_label_instruct = or_label_instruct + or_counter;
        or_label_instruct = or_label_instruct.append(":");

        if (last_comp._Equal("<")) {
            string label_instruct = "\n\t\tJL ";
            string label2_instruct = "\n\t\tJMP ";
            string label3_instruct = "\n\t\t";
            label_instruct = label_instruct.append(cond_assembler->stack_top());
            label2_instruct = label2_instruct.append(cond_assembler->stack_top());
            label3_instruct = label3_instruct.append(cond_assembler->stack_top());

            label2_instruct = label2_instruct.append("end");
            //cout << "\n\nEquality detected\n\n";
            label_instruct = label_instruct.append("top");
            label3_instruct = label3_instruct.append("top:");
           

            print_to_assembly_file(label_instruct);
           
            print_an_orrr();
            print_to_assembly_file(label2_instruct);
            print_to_assembly_file(label3_instruct);
            //cond_assembler->set_or_not_used();
            //cond_assembler->set_or_to_used();
        }
        else if (last_comp._Equal("=")) {
            string label_instruct = "\n\t\tJE ";
            string label2_instruct = "\n\t\tJMP ";
            string label3_instruct = "\n\t\t";
            label_instruct = label_instruct.append(cond_assembler->stack_top());
            label2_instruct = label2_instruct.append(cond_assembler->stack_top());
            label3_instruct = label3_instruct.append(cond_assembler->stack_top());

            label2_instruct = label2_instruct.append("end");
            //cout << "\n\nEquality detected\n\n";
            label_instruct = label_instruct.append("top");
            label3_instruct = label3_instruct.append("top:");


            print_to_assembly_file(label_instruct);
            
            print_an_orrr();
            print_to_assembly_file(label2_instruct);
            print_to_assembly_file(label3_instruct);
        }
        else if (last_comp._Equal(">")) {
            string label_instruct = "\n\t\tJG ";
            string label2_instruct = "\n\t\tJMP ";
            string label3_instruct = "\n\t\t";
            label_instruct = label_instruct.append(cond_assembler->stack_top());
            label2_instruct = label2_instruct.append(cond_assembler->stack_top());
            label3_instruct = label3_instruct.append(cond_assembler->stack_top());

            label2_instruct = label2_instruct.append("end");
            //cout << "\n\nEquality detected\n\n";
            label_instruct = label_instruct.append("top");
            label3_instruct = label3_instruct.append("top:");


            print_to_assembly_file(label_instruct);

         
            print_an_orrr();
            print_to_assembly_file(label2_instruct);
            print_to_assembly_file(label3_instruct);
        }


    
      

        //cond_assembler->inc_or_count();

    }

    void  do_then_comp_jump() {//writes the jump assembly code for comparisons



        string label_instruct = "\n\t\tOR";//instruction to print the label for ors
        string label2_instruct = "\n\t\t";//instruction to print the label for ors
        string else_instruct = "\n\t\tJMP ELSE";
        string if_counter =cond_assembler->stack_top();
        string if_counter2 = cond_assembler->stack_top();
        string last_comp = cond_assembler->get_last_comp();
        string instruct = "";
        string or_counter = to_string(cond_assembler->get_or_count());
        label_instruct = label_instruct.append(or_counter);
        label_instruct = label_instruct.append(":");
        label2_instruct = label2_instruct.append(if_counter);
        label2_instruct = label2_instruct.append("top:");
        else_instruct = else_instruct.append(if_counter2);
        
        if (last_comp._Equal("=")) {

            string if_count = cond_assembler->stack_top();
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JE ");
            instruct = instruct.append(if_count);
            instruct = instruct.append("top");

            print_to_assembly_file(instruct);

            //cond_assembler->set_or_not_used();




        }
        else if (last_comp._Equal(">")) {

            string if_count = cond_assembler->stack_top();
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JLE ");
            instruct = instruct.append(if_count);
            instruct = instruct.append("top");
            print_to_assembly_file(instruct);

            //cond_assembler->set_or_not_used();
        }
        else if (last_comp._Equal("<")) {


            string if_count = cond_assembler->stack_top();
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JL ");
            instruct = instruct.append(if_count);
            instruct = instruct.append("top");
            print_to_assembly_file(instruct);
            //cond_assembler->set_or_not_used();
            //cond_assembler->set_or_to_used();
        }

     
        print_an_orrr();//weird glitch with or label

        print_to_assembly_file(else_instruct);

        print_to_assembly_file(label2_instruct);


        then_done_flag = true;


        //cond_assembler->inc_or_count();

    }


    void  do_or_comp_jump() {//writes the jump assembly code for comparisons

        string label_instruct = "\n\t\tOR";//instruction to print the label for ors
        string last_comp = cond_assembler->get_last_comp();
        string instruct = "";
        string or_counter = to_string(cond_assembler->get_or_count());
        label_instruct = label_instruct.append(or_counter);
        label_instruct = label_instruct.append(":");
        if (last_comp._Equal("=")) {

            string if_count = cond_assembler->stack_top();
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JE ");
            instruct = instruct.append(if_count);
            instruct = instruct.append("top");
            print_to_assembly_file(instruct);

            //cond_assembler->set_or_not_used();




        }
        else if (last_comp._Equal(">")) {

            string if_count = cond_assembler->stack_top();
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JG ");
            instruct = instruct.append(if_count);
            instruct = instruct.append("top");
            print_to_assembly_file(instruct);

            //cond_assembler->set_or_not_used();
        }
        else if (last_comp._Equal("<")) {

            string if_count = cond_assembler->stack_top();
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JL ");
            instruct = instruct.append(if_count);
            instruct = instruct.append("top");

            print_to_assembly_file(instruct);
            //cond_assembler->set_or_not_used();
            //cond_assembler->set_or_to_used();
        }
        
        

    }

    void  do_and_comp_jump(){//writes the jump assembly code for comparisons

        string last_comp = cond_assembler->get_last_comp();
        string instruct = "";

        if (last_comp._Equal("=")) {

            string or_count = to_string(cond_assembler->get_or_count());
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JNE OR");
            instruct = instruct.append(or_count);

            print_to_assembly_file(instruct);

            cond_assembler->set_or_to_used();


            

        }
        else if (last_comp._Equal(">")) {

            string or_count = to_string(cond_assembler->get_or_count());
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JLE OR");
            instruct = instruct.append(or_count);

            print_to_assembly_file(instruct);

            cond_assembler->set_or_to_used();
        }
        else if (last_comp._Equal("<")) {

            string or_count = to_string(cond_assembler->get_or_count());
            //cout << "\n\nEquality detected\n\n";
            instruct = instruct.append("JGE OR");
            instruct = instruct.append(or_count);

            print_to_assembly_file(instruct);
            /*
            instruct = "";
            instruct = instruct.append("\n\t\tJE  OR");
            instruct = instruct.append(or_count);
            print_to_assembly_file(instruct);

            */

            cond_assembler->set_or_to_used();
        }

     }
    void F_prime() {//F_prime is fact_prime

        CToken token;


        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);
            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;
                string token_val= token.tokenValue;
                parser_token.tokenValue = token_val;

                if (parser_token.tokenValue._Equal(token_val) == false) {
                    parser_token.tokenValue = "";
                    parser_token.tokenValue.append(token_val);
                    //cout << "\n\n!!!weird glitch detected\n\n";
                }
            }
        }
        int weird_glitch_checker = 0;
        if (parser_token.tokenValue.find("\\x") != std::string::npos && weird_glitch_checker<100) {

            //cout << "\n\n!!!weird glitch detected\n\n";
            /*
            grab_new_token(&token);
            weird_glitch_checker++;
            */
        }

        //cout << "\nF' grabing token type: " + parser_token.tokenType + " , value: " + parser_token.tokenValue + "...";

        if (parser_token.tokenValue._Equal("*")) {

            curr_op.push("*");
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound *";

          
            FR();
            number_9("*");
            F_prime();
        }
        else if (parser_token.tokenValue._Equal("/")) {

            curr_op.push("/");
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound /";
            FR();
            number_9("/");
            F_prime();
            //register_manager.free_register_all()

        }
        else if (parser_token.tokenValue._Equal("and")) {
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound /";

            do_and_comp_jump();
            FR();
            number_9("and");
            F_prime();

        }
    }


    bool doing_comparisons() {

        bool do_compars = false;

        if (curr_op.empty() == false) {

            if (curr_op.top()._Equal("=")) {
                do_compars = true;
            }
            else if (curr_op.top()._Equal(">")) {
                do_compars = true;
            }
            else if (curr_op.top()._Equal("<")) {
                do_compars = true;
            }
        }

        return do_compars;
    }

    bool there_are_no_instructs() {

        bool empty = false;

        empty = register_manager.get_instruction_stack().empty();

        return empty;
    }
    void write_equal_comp_code() {

        cond_assembler->set_last_comp("=");//we need to know what the last comparison was
        print_to_assembly_file("\n\n\t");

        string instruct_1 = register_manager.get_last_instruction();
        string instruct_2 = "";
        string instruct_3 = "CMP ";
        string* regs;
        //string reg2 = "";
        register_manager.pop_last_instruction();

        if (there_are_no_instructs() == false) {

            instruct_2 = register_manager.get_last_instruction();

            register_manager.pop_last_instruction();

        }
        regs = register_manager.get_last_2_registers();
        instruct_3 = instruct_3.append(regs[1]);
        instruct_3 = instruct_3.append(",");
        instruct_3 = instruct_3.append(regs[0]);

        register_manager.free_register();
        register_manager.free_register();


        print_to_assembly_file(instruct_2);
        print_to_assembly_file(instruct_1);
        print_to_assembly_file(instruct_3);
    }
    void write_greater_comp_code() {

        cond_assembler->set_last_comp(">");//we need to know what the last comparison was
        print_to_assembly_file("\n\t");

        string instruct_1 = register_manager.get_last_instruction();
        string instruct_2 = "";
        string instruct_3 = "CMP ";
        string* regs;
        //string reg2 = "";
        register_manager.pop_last_instruction();

        if (there_are_no_instructs() == false) {

            instruct_2 = register_manager.get_last_instruction();

            register_manager.pop_last_instruction();

        }
        regs = register_manager.get_last_2_registers();
        instruct_3 = instruct_3.append(regs[1]);
        instruct_3 = instruct_3.append(",");
        instruct_3 = instruct_3.append(regs[0]);

        register_manager.free_register();
        register_manager.free_register();


        print_to_assembly_file(instruct_2);
        print_to_assembly_file(instruct_1);
        print_to_assembly_file(instruct_3);

    }
    void write_lesser_comp_code() {

        cond_assembler->set_last_comp("<");//we need to know what the last comparison was
        print_to_assembly_file("\n\t");

        string instruct_1 = register_manager.get_last_instruction();
        string instruct_2 = "";
        string instruct_3 = "CMP ";
        string* regs;
        //string reg2 = "";
        register_manager.pop_last_instruction();

        if (there_are_no_instructs() == false) {

            instruct_2 = register_manager.get_last_instruction();

            register_manager.pop_last_instruction();

        }
        regs = register_manager.get_last_2_registers();
        instruct_3 = instruct_3.append(regs[1]);
        instruct_3 = instruct_3.append(",");
        instruct_3 = instruct_3.append(regs[0]);

        register_manager.free_register();
        register_manager.free_register();


        print_to_assembly_file(instruct_2);
        print_to_assembly_file(instruct_1);
        print_to_assembly_file(instruct_3);

    }
    void write_comp_code(string compar_op) {//writes assembler code for comparisons

        if (compar_op._Equal("=") && there_are_no_instructs()==false)
        {
           
            write_equal_comp_code();

        }
        else if (compar_op._Equal(">") && there_are_no_instructs() == false)
        {

            //cout << "\n\nComparison detected\n\n";

            write_greater_comp_code();
        }
        else if (compar_op._Equal("<") && there_are_no_instructs() == false)
        {

            //cout << "\n\nComparison detected\n\n";

            write_lesser_comp_code();
        }
    }

    void  check_data_type_err_64() {

        string dat1 = "";
        string dat2 = "";

        if (datatype_stack.size() > 1) {

            dat1 = datatype_stack.top();

            datatype_stack.pop();

            dat2 = datatype_stack.top();

            datatype_stack.pop();


            if (dat1._Equal(dat2) == false) {

                bad_code();
            }
        }

    }
    void FR() {//FR is fact_r

        CToken token;

        if (parser_token.consumed) {//if the token was consumed previously get new one
            LEX.GetToken(token);

            if (!token.tokenType._Equal("")) {
                parser_token.consumed = 0;
                parser_token.tokenType = token.tokenType;

                parser_token.tokenValue = token.tokenValue;

                if (parser_token.tokenValue._Equal("true") || parser_token.tokenValue._Equal("false"))
                    parser_token.tokenType = "boolean";

            }
            else {




                parser_token.tokenValue = "";
            }
        }

        //cout << "\nFR grabing token type: " + parser_token.tokenType + " , value: " + parser_token.tokenValue+ "...";
        if (parser_token.tokenValue._Equal("(")) {
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound (";
           // E();

            expression();



            if (parser_token.tokenValue._Equal(")")) {

                parser_token.consumed = 1;//tell them we consumed the token, get next one
                //cout << "\nfound )";
            }
            else if (parser_token.tokenValue._Equal("then") && then_done_flag) {

                then_done_flag = true;
            }
            else if (parser_token.tokenValue._Equal("do") || special_do_situaation) {

                special_do_situaation = true;
            }
            else {

                bad_code();
                //system("pause");
                //exit(99);
            }

        }
        else if (parser_token.tokenType._Equal("real")) {
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            this->datatype_stack.push("real");//we need to add litterals to the stack for calculation
            literal_added = 1;
            //cout << "\nfound real";
        }
        else if (parser_token.tokenType._Equal("integer")) {

            string op_to_carry = "";

            if (curr_op.empty() == false) {
                op_to_carry = curr_op.top();
               

            }
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            this->datatype_stack.push("integer");

            
            create_assembler_code(parser_token.tokenType, parser_token.tokenValue, true);

            literal_added = 1;

            carry_out_lit_op(op_to_carry);

            if (doing_comparisons()) {

                    string compar_op = "";

                    compar_op = curr_op.top();

                    //cout << "\n\ncomparison detected\n\n";

                    write_comp_code(compar_op);

                    curr_op.pop();

                    grab_new_token(&token);

                    if (parser_token.tokenValue._Equal(")")) {

                        consume();

                        check_data_type_err_64();
                    }

                    F_prime();


                    if (parser_token.tokenValue._Equal("then") && then_done_flag==false) {//this marks then end of comparisons

                       

                        datatype_stack.push("boolean");

                        do_then_comp_jump();
                      
                    }
                    else if (parser_token.tokenValue._Equal("do")) {

                        bool special_do_situaation = true;
                    }
                    



                

            }
           
            //cout << "\nfound integer";
        }
        else if (parser_token.tokenType._Equal("boolean")) {

            parser_token.consumed = 1;//tell them we consumed the token, get next one
            this->datatype_stack.push("boolean");
            literal_added = 1;
            //cout << "\nfound integer";
        }
        else if (parser_token.tokenType._Equal("word")) {

            parser_token.consumed = 1;//tell them we consumed the token, get next one

           
            
            number_8();

            create_assembler_code(parser_token.tokenType, parser_token.tokenValue, false);

            string datatype = "";
            if (this->symbol_table.is_function(parser_token.tokenValue)) {

                grab_new_token(&token);
            }

            if (parser_token.tokenValue._Equal("(")) {
                parser_token.consumed = 1;//tell them we consumed the token, get next one
                //cout << "\nfound (";
               // E();

                expression();



                if (parser_token.tokenValue._Equal(")")) {

                    parser_token.consumed = 1;//tell them we consumed the token, get next one
                    //cout << "\nfound )";
                }
                else {

                    bad_code();
                    //system("pause");
                    //exit(99);
                }

            }
            //cout << "\nfound integer";
        }
        else if (parser_token.tokenValue._Equal("+")) {

            curr_op.push("+");
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound +";

            TR();
        }
        else if (parser_token.tokenValue._Equal("-")) {
            curr_op.push("-");
            parser_token.consumed = 1;//tell them we consumed the token, get next one
            //cout << "\nfound -";
            TR();
        }
        else if (parser_token.tokenType._Equal("")) {

        }
        else {

            bad_code();
            system("pause");
            exit(99);
        }

    }


}CParser;