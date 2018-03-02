#include "ErrorResolver.h"

ErrorResolver::ErrorResolver(){

	// https://stackoverflow.com/questions/1898657/result-of-sizeof-on-array-of-structs-in-c
	_errcount = sizeof errors / sizeof errors[0];

}

int ErrorResolver::get_name(uint32_t code, char* name){

	// search all available errors
	for(uint32_t i = 0; i<_errcount; i++){

		if(errors[i].code==code){
			strcpy(name, errors[i].name);
			return 0;
		}
	}

	// error not found in table
	strcpy(name, "unknown error");

	return 1;
}
