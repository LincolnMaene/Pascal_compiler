#pragma once
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

using namespace std;

struct Scope;

struct VarData
{
	string type;
	int size;
	int offset;
	int param_offset;
	int value_or_ref;//1 is by copy, 2 is by ref, 0 means it's a variable, not a parameter
	string return_type;
	Scope* NextScope;
};

struct Scope
{
	unordered_map<string, VarData> Table;
	int offset;
	int param_offset;
	int local_var_count;
	int space_needed;
	string ScopeName;
	string return_type;
	Scope* PreviousScope;
	vector<string> ParamList;//list of parameters



};

class SymbolTable
{
private:

	Scope* CurrentScope;
	bool has_locos;//has local variables
	int total_loco_space;//how much space they use


public:

	SymbolTable()
	{
		CurrentScope = new Scope;
		CurrentScope->offset = 0;
		CurrentScope->local_var_count = 0;
		CurrentScope->param_offset = 8;
		CurrentScope->space_needed = 0;
		CurrentScope->ScopeName = "";
		CurrentScope->return_type = "";
		CurrentScope->PreviousScope = NULL;
		has_locos = false;
		total_loco_space = 0;
	}

	bool has_local_variables() {

		return this->has_locos;
	}

	int total_local_variable_space() {

		return this->total_loco_space;
	}

	bool is_global_var(string name, int& offset)
	{

		bool is_global = false;
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{
				offset = TravScope->Table[name].offset;

				if (TravScope->PreviousScope == NULL)
					is_global = true;

				return is_global;
			}
		}


		return is_global;
	}

	bool is_local_var(string name, int& offset)
	{

		bool is_local = false;
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{
				offset = TravScope->Table[name].offset;

				int val_or_ref = TravScope->Table[name].value_or_ref;

				if (TravScope->PreviousScope != NULL && val_or_ref==0)
					is_local = true;

				return is_local;
			}
		}


		return is_local;
	}

	bool is_param_copy(string name, int& offset)
	{

		bool is_local = false;
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{
				offset = TravScope->Table[name].param_offset;

				int val_or_ref = TravScope->Table[name].value_or_ref;

				if (TravScope->PreviousScope != NULL && val_or_ref == 1)
					is_local = true;

				return is_local;
			}
		}


		return is_local;
	}

	bool is_param_ref(string name, int& offset)
	{

		bool is_local = false;
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{
				offset = TravScope->Table[name].param_offset;

				int val_or_ref = TravScope->Table[name].value_or_ref;

				if (TravScope->PreviousScope != NULL && val_or_ref == 2)
					is_local = true;

				return is_local;
			}
		}


		return is_local;
	}
	void set_return_type(string datatype) {

		CurrentScope->return_type = datatype;

	}
	void find_param_info(string scope_name, string& datatype, int& val_or_ref, int& off_set, int param_num) {

		if (CurrentScope->ScopeName._Equal(scope_name)) {

			VarData  var_ptr = CurrentScope->Table[CurrentScope->ParamList[param_num]];

			datatype = var_ptr.type;

			val_or_ref = var_ptr.value_or_ref;

			off_set = var_ptr.param_offset;





		}

		else if (CurrentScope->Table.find(scope_name) != CurrentScope->Table.end()) {
			Scope* scpe_ptr = NULL;
			scpe_ptr = CurrentScope->Table[scope_name].NextScope;
			VarData  var_ptr = scpe_ptr->Table[scpe_ptr->ParamList[param_num]];
			
			datatype = var_ptr.type;

			val_or_ref = var_ptr.value_or_ref;

			off_set = var_ptr.param_offset;

		}

	}
	int find_param_num(string scope_name) {//finds the number of parameters in the scope, returns -1 if scope doesnn't exist

		int param_num=-1;

		if (CurrentScope->ScopeName._Equal(scope_name))
			param_num = CurrentScope->ParamList.size();
		
		else if (CurrentScope->Table.find(scope_name) != CurrentScope->Table.end()) {

			VarData variable = CurrentScope->Table[scope_name];

			string var_type = variable.type;

			if(var_type._Equal("integer") || var_type._Equal("boolean") || var_type._Equal("character"))//we need to make sure it's a function
				return param_num;


			param_num= variable.NextScope->ParamList.size();
		}

		return param_num;

	}

	bool is_in_procedure() {//allows the symbol table to know whether it is in a procedure or not

		bool is_in = false;


		if (CurrentScope->PreviousScope)//if there is no previous scope we're global
			is_in = true;


		return is_in;
	}
	int get_local_var_count() {

		return CurrentScope->local_var_count;
	}
	bool add_parameters(string pararm_name, string param_type, bool passed_by_val) {

		bool parameter_added = false;

		int find_offset;

		if (FindVariable(pararm_name, find_offset) == false) {//we need to prevent duplicate identifiers

			
			add_param_helper(pararm_name, param_type, 4, passed_by_val);

			this->CurrentScope->ParamList.push_back(pararm_name);

			parameter_added = true;

		}
		else {

			duplicate_failure();
		}

		return parameter_added;
	}

	int param_count() {//if it works, returns the number of parameters in the scope

		return this->CurrentScope->ParamList.size();
	}

	bool add_param_helper(string name, string type, int size, bool passed_by_val)
	{
		if (CurrentScope->Table.find(name) != CurrentScope->Table.end())
		{
			return false;
		}
		CurrentScope->Table[name].type = type;
		CurrentScope->Table[name].size = size;
		if (passed_by_val)
			CurrentScope->Table[name].value_or_ref = 1;
		else
			CurrentScope->Table[name].value_or_ref = 2;
		CurrentScope->Table[name].param_offset = CurrentScope->param_offset;
		CurrentScope->Table[name].NextScope = NULL;
		CurrentScope->param_offset += size;
		return true;
	}

	int get_space_need() {

		return CurrentScope->space_needed;
	}
	bool AddVariable(string name, string type, int size)
	{
		if (CurrentScope->Table.find(name) != CurrentScope->Table.end())
		{
			duplicate_failure();
			return false;
		}

		if (CurrentScope->PreviousScope!=NULL) {
			has_locos = true;
			CurrentScope->local_var_count = CurrentScope->local_var_count + 1;
			CurrentScope->space_needed = CurrentScope->space_needed + size;
			total_loco_space = total_loco_space + CurrentScope->space_needed;
		}

		CurrentScope->Table[name].type = type;
		CurrentScope->Table[name].size = size;
		CurrentScope->Table[name].value_or_ref = 0;
		CurrentScope->Table[name].offset = CurrentScope->offset;
		CurrentScope->Table[name].NextScope = NULL;
		CurrentScope->offset += size;
		return true;
	}

	string find_func_name() {
		string name = "";

		for (auto const& x : CurrentScope->PreviousScope->Table)
		{
			if (x.second.type._Equal("function") && x.second.return_type._Equal(""))
				name = x.first;
		}

		return name;

	}

	string set_function_return_type(string& datatype) {
		
		string name = find_func_name();
		string found_type = "";
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{

				TravScope->Table[name].return_type= datatype;

				

				return datatype;
			}
		}


	}

	bool is_function(string name) {

		bool is_func_flag = false;

		string found_type = "";
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{

				found_type = TravScope->Table[name].type;

				if (found_type._Equal("function"))
					is_func_flag = true;
				return is_func_flag;
			}
		}

			return is_func_flag;

	}
	string find_var_type(string name, string& datatype) {
		
		string found_type = "";
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{
				
				found_type = TravScope->Table[name].type;

				if (found_type._Equal("function"))
					found_type = TravScope->Table[name].return_type;
				datatype = found_type;

				return found_type;
			}
		}
	
		return found_type;
	}
	bool FindVariable(string name, int& offset)
	{
		Scope* TravScope = CurrentScope;
		while (TravScope != NULL)
		{
			if (TravScope->Table.find(name) == TravScope->Table.end())
			{
				TravScope = TravScope->PreviousScope;
			}
			else
			{
				if(TravScope->Table[name].value_or_ref==0)
					offset = TravScope->Table[name].offset;
				else {
					offset = TravScope->Table[name].param_offset;
				}
				return true;
			}
		}

		
		return false;
	}

	bool AddProcFunc(string name, string type)
	{
		if (CurrentScope->Table.find(name) != CurrentScope->Table.end())
		{
			duplicate_func_failure();
			return false;
		}
		CurrentScope->Table[name].type = type;
		CurrentScope->Table[name].size = 0;
		CurrentScope->Table[name].offset = 0;

		Scope* OldCurrent = CurrentScope;

		CurrentScope->Table[name].NextScope = new Scope;
		CurrentScope = CurrentScope->Table[name].NextScope;
		CurrentScope->param_offset = 8;
		CurrentScope->space_needed = 0;
		CurrentScope->local_var_count = 0;
		CurrentScope->offset = 0;
		CurrentScope->ScopeName = name;
		CurrentScope->PreviousScope = OldCurrent;

		return true;

	}

	void BackOut()
	{
		if (CurrentScope->PreviousScope) CurrentScope = CurrentScope->PreviousScope;
	}

	void duplicate_failure() {

		cout << "\n\n!!!Duplicate Identifier!!!\n\n";
	}

	void duplicate_func_failure() {

		cout << "\n\n!!!Duplicate Identifier!!!\n\n";
	}



};


