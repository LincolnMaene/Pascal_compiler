#pragma once
#include <string>
#include <stack>
using namespace std;
class Conditional_assembler {


private:

	int or_count;
	int if_count;
	int while_count;

	string last_comp = "";

	bool or_used;

	stack<string> condition_stack;






public:

	Conditional_assembler() {

		or_count = 1;
		if_count = 1;
		while_count = 1;

		or_used = false;
	}

	bool stack_pop();
	void inc_if_count();
	string stack_top();
	void push_cond_stack(string cond_stack);
	void inc_while_count();
	int get_while_count();
	string get_last_comp();
	void set_last_comp(string last_comp);
	int get_if_count();
	int get_or_count();
	void set_or_to_used();
	void inc_or_count();
	bool or_is_used();
	void set_or_not_used();

};