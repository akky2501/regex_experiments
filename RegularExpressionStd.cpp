#include <iostream>
#include <regex>

int main(int argc,char* argv[])
{
    std::regex re(argv[1]) ;
    std::string s(argv[2]) ;

    bool result = std::regex_match( s, re ) ;

	std::cout<<"S"<<std::endl;
	std::cout<<s<<(result?"T":"F")<<std::endl;

	return 0;
}
