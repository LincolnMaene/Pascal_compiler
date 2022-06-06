#include "Registers.h"

/*---------------Register Codwe--------------------*/

string Register:: get_name() {

	return this->name;
}

bool Register:: is_literal() {

	return this->literal;
}

void Register:: set_free() {

	this->free = true;
	this->literal = false;
}

void Register:: set_occupied() {

	this->free = false;
}

int Register::get_value() {

	return this->value;
}

bool Register::is_free() {

	return this->free;
}



void  Register:: set_name(string name) {

	this->name = name;
}

void  Register:: set_literal_status(bool is_literal) {

	this->literal = is_literal;
}


void Register:: set_value(int value) {

	this->value = value;
}

/*---------------Register Manager Code --------------------*/

