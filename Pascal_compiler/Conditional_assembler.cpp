#include "Conditional_assembler.h"


void Conditional_assembler:: inc_if_count() {
	string instruct = "IF";
	instruct = instruct.append(to_string(if_count));
	condition_stack.push(instruct);
	if_count++;
	
}

bool Conditional_assembler:: stack_pop() {

	bool pop_success = false;


	if (condition_stack.empty() == false) {

		condition_stack.pop();
		pop_success = true;
		//while_count--;
	}


	return pop_success;
}

string  Conditional_assembler:: stack_top() {

	string val = "";

	if (condition_stack.empty()==false) {

		val = condition_stack.top();
	}

	return val;
}

void Conditional_assembler:: push_cond_stack(string cond_stack) {

	this->condition_stack.push(cond_stack);
}

void Conditional_assembler::inc_while_count() {

	while_count++;
}

int Conditional_assembler::get_while_count() {

	return while_count;

}

string Conditional_assembler::get_last_comp() {

	return last_comp;
}

int Conditional_assembler::get_if_count() {

	return if_count;
}

void Conditional_assembler:: set_or_not_used() {

	or_used = false;

}

bool Conditional_assembler::or_is_used() {

	return or_used;
}
void  Conditional_assembler:: inc_or_count() {

	or_count++;
}
void Conditional_assembler:: set_or_to_used() {

	or_used = true;
}
int Conditional_assembler::get_or_count() {

	return or_count;
}
void Conditional_assembler::set_last_comp(string last_comp) {

	this->last_comp = last_comp;
}