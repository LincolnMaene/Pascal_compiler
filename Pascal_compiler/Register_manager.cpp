#include "Registers.h"

Register* RegisterManager::get_registers(){

	return this->registers;

}

void RegisterManager:: free_register() {

	registers[current_register - 1].set_free();
	current_register = current_register - 1;
}

void RegisterManager:: change_curr_reg_value(int new_val) {

	registers[current_register - 1].set_value(new_val);

}

int RegisterManager:: get_curr_index() {

	return this->current_register;

}
string RegisterManager:: get_last_instruction() {
	string ret_str = "";

	if (instruction_stack.empty() == false) {
		ret_str = instruction_stack.top();
	}

	return ret_str;
}

bool RegisterManager::pop_last_instruction() {

	bool succcess = false;
	if (instruction_stack.empty() == false) {
		instruction_stack.pop();
		succcess = true;
	}

	return succcess;
}

void RegisterManager::free_register_all() {

	for (int i = 0; i < 4; i++) {

		registers[i].set_free();
		
	}
	current_register = 0;
}

bool RegisterManager::has_to_literals() {
	bool literal_flag = false;
	if (registers[current_register - 1].is_literal() && registers[current_register - 2].is_literal())
		literal_flag = true;

	return literal_flag;
}
bool RegisterManager::carry_out_literral_operation(string operation) {//optimizes by carrying out operations on litterals before assemble

	string new_instruction = "\n\t\tmov ";

	bool did_op = false;

	if (current_register > 1) {//if the current registers are more than 2

		string register_name = registers[current_register - 2].get_name();

		new_instruction = new_instruction.append(register_name);

		if (registers[current_register - 1].is_literal() && registers[current_register - 2].is_literal()) {//if they are both litteral

			if (instruction_stack.size() > 1) {
				int reg1_val = registers[current_register - 1].get_value();
				int reg2_val = registers[current_register - 2].get_value();
				int new_val = -1;
				if (operation._Equal("*")) {
					 new_val = reg1_val * reg2_val;
				}
				else if (operation._Equal("-")) {

					new_val = reg2_val- reg1_val;
				}
				else if (operation._Equal("+")) {

					new_val = reg2_val + reg1_val;
				}
				else if (operation._Equal("/")) {

					new_val = reg2_val / reg1_val;
				}
				string string_val = to_string(new_val);

				new_instruction = new_instruction.append(",");
				new_instruction = new_instruction.append(string_val);

				instruction_stack.pop();
				instruction_stack.pop();
				instruction_stack.push(new_instruction);

				free_register();
				change_curr_reg_value(new_val);

				did_op = true;



			}

		}

	}

	return did_op;

}

Register RegisterManager::get_1_register(int index) {


	Register register_returned;

	if(index>=0 && index<=3){
		register_returned = registers[index];
	}

	return register_returned;

}


void RegisterManager::add_to_register(bool literal, int value) {

	registers[current_register].set_literal_status(literal);

	registers[current_register].set_value(value);

	registers[current_register].set_occupied();

	current_register++;


}
void RegisterManager:: push_to_insstruction_stack(string instruct) {

	this->instruction_stack.push(instruct);

}
string RegisterManager:: get_register_name() {//gets name of current register

	return registers[current_register].get_name();
}

int  RegisterManager::get_register_val(string reg_name) {

	int val = -1;
	if (reg_name._Equal("EAX")) {
		val = registers[0].get_value();
		

	}
	else if (reg_name._Equal("EBX")) {

		val = registers[1].get_value();
	}
	else if (reg_name._Equal("ECX")) {

		val = registers[2].get_value();

	}
	else if (reg_name._Equal("EDX")) {

		val = registers[3].get_value();
	}

	return val;
}

bool  RegisterManager::register_is_literal(string reg_name) {
	bool is_literal = false;
	/*
	registers[0].set_name("EAX");
	registers[1].set_name("EBX");
	registers[2].set_name("ECX");
	registers[3].set_name("EDX");

	*/
	if (reg_name._Equal("EAX")) {

		if (registers[0].is_literal())
			is_literal = true;
	
	}
	else if (reg_name._Equal("EBX")) {

		if (registers[1].is_literal())
			is_literal = true;
	}
	else if (reg_name._Equal("ECX")) {

		if (registers[2].is_literal())
			is_literal = true;

	}
	else if (reg_name._Equal("EDX")) {

		if (registers[3].is_literal())
			is_literal = true;
	}

	return is_literal;

}
string RegisterManager::get_last_register_name() {//gets name of last occupied register

	string reg_name = "";
	if(current_register>0)
		return registers[current_register-1].get_name();

	return reg_name;
}

string* RegisterManager::get_last_2_registers() {

	string * registers_ret=new string[2];

	if (current_register > 1) {

		registers_ret[0] = registers[current_register - 1].get_name();
		registers_ret[1] = registers[current_register - 2].get_name();

	}

	return registers_ret;

}
stack<string> RegisterManager::get_instruction_stack() {

	return this->instruction_stack;
}