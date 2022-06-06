#pragma once
#include <string>
using namespace std;
#include <stack>


class Register {


private:


	string name;
	bool literal;
	int value;
	bool free;//tells us wehter the register is free or occupied



public:

	Register() {

		name = "";
		literal = false;
		value = -1;
		free = true;
	}

	string get_name();

	void set_name(string);

	bool is_literal();

	bool is_free();
	
	void set_occupied();

	void set_literal_status(bool);

	int  get_value();

	void set_value(int);

	void set_free();
};

class RegisterManager {


private:

	Register registers[4];//we have four registers

	int current_register;

	stack<string> instruction_stack;

public:

	RegisterManager() {

		registers[0].set_name("EAX");
		registers[1].set_name("EBX");
		registers[2].set_name("ECX");
		registers[3].set_name("EDX");

		for (int i = 0; i < 4; i++) {

			registers[i].set_value(-1);
			registers[i].set_literal_status(false);
		}

		current_register = 0;



	}
	void add_to_register(bool literal, int value);
	Register* get_registers();
	Register get_1_register(int);
	stack<string> get_instruction_stack();
	string get_register_name();
	bool carry_out_literral_operation(string);
	void free_register();
	void push_to_insstruction_stack(string);
	void change_curr_reg_value(int);
	bool has_to_literals();
	int get_curr_index();//gives us index of current register
	string get_last_instruction();
	string * get_last_2_registers();
	string get_last_register_name();
	int get_register_val(string);
	void free_register_all();
	bool register_is_literal(string);
	bool pop_last_instruction();

};