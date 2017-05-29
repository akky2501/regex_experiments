
//2015.7.31 begin coding

#define _CRT_SECURE_NO_WARNINGS


#include<stdio.h>
#include<iostream>
#include<fstream>
#include<regex>
#include<string>

int main(void) try{
	struct {
		std::string type;
		std::regex pattern;
	} table[] = {
		{ "ignore", std::regex{"^(\\t|\\n|\\r|\\s)"} },
		{ "comment", std::regex("^//.*$") },

		{ "::", std::regex("^::") },
		{ "->", std::regex("^->") },
		{ ";",  std::regex("^;") },
		{ ":",  std::regex("^\\:") },
		{ ".",  std::regex("^\\.") },
		{ ",",  std::regex("^,") },
		{ "(",  std::regex("^\\(") },
		{ ")",  std::regex("^\\)") },
		{ "{",  std::regex("^\\{") },
		{ "}",  std::regex("^\\}") },
		{ "[",  std::regex("^\\[") },
		{ "]",  std::regex("^\\]") },
		

		{ "==", std::regex("^==") },
		{ "+=", std::regex("^\\+=") },
		{ "-=", std::regex("^-=") },
		{ "*=", std::regex("^\\*=") },
		{ "/=", std::regex("^/=") },
		{ ">=", std::regex("^>=") },
		{ "<=", std::regex("^<=") },
		{ "&&", std::regex("^&&") },
		{ "||", std::regex("^\\|\\|") },
		{ ">",  std::regex("^>") },
		{ "<",  std::regex("^<") },
		{ "+",  std::regex("^\\+") },
		{ "-",  std::regex("^-") },
		{ "*",  std::regex("^\\*") },
		{ "/",  std::regex("^/") },
		{ "=",  std::regex("^=") },
		

		{ "void",   std::regex("^void") },
		{ "auto",   std::regex("^auto") },
		{ "int",    std::regex("^int") },
		{ "float",  std::regex("^float") },
		{ "if",     std::regex("^if") },
		{ "else",   std::regex("^else") },
		{ "for",    std::regex("^for") },
		{ "while",  std::regex("^while") },
		{ "do",     std::regex("^do") },
		{ "break",  std::regex("^break") },
		{ "return", std::regex("^return") },


		{ "digit"  ,std::regex("^[1-9][0-9]*\\.[0-9]*f?") },
		{ "integer",std::regex("^[1-9][0-9]*") },
		{ "symbol", std::regex("^[_a-zA-Z][_a-zA-Z0-9]*") },

		{ "raw_str", std::regex("^\"[^\"]*\"") },
		{ "raw_char",std::regex("^'.'")},
	};

	std::string test("float PI->3.141592;//constant number.");

	//FILE* fp_in = freopen("analized.txt", "w", stdout);
	//std::ifstream ifs("regex_training.cpp");

	std::string line;
	line = test;
	//while (std::getline(ifs, line)){
		std::smatch m;
		auto it = line.cbegin();
		auto end = line.cend();
		while (it != end){
			bool unknown = true;
			for (auto& t : table){
				if (std::regex_search(it, end, m, t.pattern)){
					std::cout << t.type << "[" << m.str() << "]" << std::endl;
					it = m[0].second;
					unknown = false;
					break;
				}
			}
			if (unknown){
				std::cout << "unknown symbol" << *it << std::endl;
				it++;
			}
		}
	//}

	//fclose(fp_in);

	return 0;
}
catch (const std::regex_error& err){
	std::cout << err.code() << ':' << err.what() << std::endl;
}
