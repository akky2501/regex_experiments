
#define _CRT_SECURE_NO_WARNINGS

//2015.7.9 一応ピリオド対応させたけどちゃんと動くかワカラン


#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<set>
#include<deque>


class Lexer{//一文字先読みレキサー
public:
    Lexer(std::string str_)
    :ptr(0),str(str_),current_str('\0')
    {}

    char next(void){//次のトークンを読み込む
        current_str = str[ptr];
        ptr++;
        return current_str;
    }

    char current(void){
        return current_str;
    }

	int current_count(void){
		return ptr-1;
	}
private:
    int ptr;
    std::string str;
    char current_str;
};

enum Operator{
    opEmpty,
    opChar,
	opAny,
    opConnect,
    opOr,
    opClosure
};


const char* OperatorString[]={
	"Empty",
	"Char",
	"Any",
	"Connect",
	"Or",
	"Closure"
};

class Node{
public:
    Node(Operator op_,char c_,Node* lhs_,Node* rhs_)
    :op(op_),lhs(lhs_),rhs(rhs_),c(c_)
    {}

    Node(Operator op_,Node* lhs_,Node* rhs_)
    :op(op_),lhs(lhs_),rhs(rhs_),c('\0')
    {}

    Node(char c_)   //とりあえず文字クラスは考えないこととする
    :op(opChar),lhs(nullptr),rhs(nullptr),c(c_)
    {}

    ~Node(){
        if(lhs) delete lhs;
        if(rhs) delete rhs;
    }

    Node* deepCopy(void){
        return new Node(op,c,(lhs)?lhs->deepCopy():nullptr,(rhs)?rhs->deepCopy():nullptr);
    }

//private:
    Operator op;
    Node *lhs,*rhs;
    char c;
};


class RegularExpression{
public:
    RegularExpression(std::string str_)
    :lex(str_)
    {
        root = parse();

		//std::cout << "Parse-------------------------" << std::endl;
		print_counter = 0;
        //printTree(root);
		//std::cout << std::endl;

    }

    ~RegularExpression(){
        delete root;
    }

	bool match(std::string str){
		return check(str.c_str(),root->deepCopy());
	}

private:
    Lexer lex;
    Node* root;

	int print_counter;
    void printTree(Node* n){
        if(n->lhs) printTree(n->lhs);
        if(n->rhs) printTree(n->rhs);

        std::cout<<print_counter++<<" : op["<<OperatorString[n->op]<<"]";
        if(n->op == opChar) std::cout<<" "<<n->c;
        std::cout<<std::endl;
    }

	bool isMeta(char c){
		return (c == '+') || (c == '*') || (c == '?') || (c == '|') || (c == '(') || (c == ')')||(c == '.');
	} 
	
	bool isUnary(char c){
		return (c == '+') || (c == '*') || (c == '?') ;
	}

    Node* parse(void){
        lex.next();
        Node* res = parseOr();
		if (lex.current() != '\0') std::cout << "error(parse):" << lex.current_count() << std::endl;
        return res;
    }

    Node* parseOr(void){
        Node* lhs = parseTerm();

        while(lex.current() == '|'){
			lex.next();
            Node* rhs = parseTerm();
            lhs = new Node(opOr,lhs,rhs);
        }

        return lhs;
    }

    Node* parseTerm(void){
		if (lex.current() == '|' || lex.current() == ')' || lex.current() == '\0') return new Node(opEmpty, nullptr, nullptr);

        Node* lhs = parseUnary();

		while (!isUnary(lex.current()) && lex.current() != '|' && lex.current() != ')' && lex.current() != '\0'){
            Node* rhs = parseUnary();
            lhs = new Node(opConnect,lhs,rhs);
        }

        return lhs;
    }

	Node* parseUnary(void){
		Node* lhs = parsePrimary();

		char x = lex.current();
		switch (x){
		case '?'://0or1文字
			lhs = new Node(opOr, lhs, new Node(opEmpty, nullptr, nullptr));
			lex.next();
			break;

		case '*'://0文字以上
			lhs = new Node(opClosure, lhs, nullptr);
			lex.next();
			break;

		case '+'://1文字以上
			lhs = new Node(opConnect, lhs->deepCopy(), new Node(opClosure, lhs, nullptr));
			lex.next();
			break;
		}


        return lhs;

    }

    Node* parsePrimary(void){
        Node* lhs;
        if(lex.current() == '('){
			int count = lex.current_count();
			lex.next();
            lhs = parseOr();
            if(lex.current() != ')') {//エラー、対応括弧なし
				std::cout << "error(parsePrimary):" << count << std::endl;
			}
			lex.next();//')'をはきだす
        }
        else{
            //TODO:文字クラスとの判定
            lhs = parseChar();
			lex.next();
        }

		return lhs;
    }

    Node* parseChar(void){
		if (lex.current() == '.') return new Node(opAny,0,nullptr,nullptr);
		else return parseNotMetaChar();
    }

	Node* parseNotMetaChar(void){
		//エラーチェック
		if (isMeta(lex.current())) {//えらー
			std::cout << "error(parseChar):" << lex.current_count() << std::endl;

		}

		char c = '$';

		if (lex.current() == '\\') c = lex.next();
		else c = lex.current();

		return new Node(c);
	}

	Node* delta(Node* node){
		//std::cout<<"del"<<std::endl;
		if(node == nullptr) return nullptr;

		switch(node->op){
			case opEmpty: 
			//std::cout<<"demp"<<std::endl;
				return new Node(opEmpty,nullptr,nullptr);

			case opAny:	
			case opChar:{ 
			//std::cout<<"dchr"<<std::endl;
				return nullptr; }
			case opClosure:{
			//std::cout<<"dclo"<<std::endl;
				 return new Node(opEmpty,nullptr,nullptr); }
			case opConnect:{
			//std::cout<<"dcon"<<std::endl;
				Node* l = delta(node->lhs);
				Node* r = delta(node->rhs);
				Node* result = nullptr;
				if(l != nullptr && r != nullptr) result = new Node(opEmpty,nullptr,nullptr);
				//if(l) delete l;
				//if(r) delete r;
				return result;
			}
			case opOr:{
			//std::cout<<"dor"<<std::endl;
				Node* l = delta(node->lhs);
				Node* r = delta(node->rhs);
				Node* result = nullptr;
				if( !(l == nullptr && r == nullptr) ) result = new Node(opEmpty,nullptr,nullptr);
				//if(l) delete l;
				//if(r) delete r;
				return result;
			}

		}

		return nullptr;
	}

	Node* derive(char c,Node* L){
		//std::cout<<"der"<<std::endl;
		if(L == nullptr) return nullptr;
		if(L->op == opEmpty){
			//std::cout<<"emp"<<std::endl;
			return nullptr;}
		if(L->op == opChar){
			//std::cout<<"chr"<<std::endl;
			if(c == L->c){ return new Node(opEmpty,nullptr,nullptr); }
			else { return nullptr; }
		}
		if(L->op == opAny){
			//std::cout<<"any"<<std::endl;
			return new Node(opEmpty,nullptr,nullptr);
		}
		if(L->op == opClosure){
			//std::cout<<"clo"<<std::endl;
			Node* p = L;//->deepCopy();
			return new Node(opConnect,derive(c,L->lhs),p);
		}
		if(L->op == opConnect){
			//std::cout<<"con"<<std::endl;
			//L->op = opOr;
			Node* cpl = L->lhs;//(L->lhs)?L->lhs->deepCopy():nullptr;
			Node* cpr = L->rhs;//(L->rhs)?L->rhs->deepCopy():nullptr;
			Node* l = new Node(opConnect,derive(c,cpl),cpr);
			Node* d = delta(L->lhs);
			Node* ret;
			if(d){
				Node* r = derive(c,L->rhs);
				ret = new Node(opOr,l,r);
			}
			else{
				ret = l;
			}
			//L->lhs = l;
			//L->rhs = r;
			//return L;
			return ret;
		}
		if(L->op == opOr){
			//std::cout<<"or"<<std::endl;
			//L->op = opOr;
			Node* l = derive(c,L->lhs);
			Node* r = derive(c,L->rhs);
			//L->lhs = l;
			//L->rhs = r;
			//return L;
			return new Node(opOr,l,r);
		}

		return nullptr;

	}

	bool check(const char* str,Node* L){
		//std::cout<<"----------------------"<<std::endl;
		//std::cout<<((str[0]=='\0')?'$':str[0])<<std::endl;
		//printTree(L);
		if(str[0] == '\0') return delta(L) != nullptr;
		return check(&str[1],derive(str[0],L));
	}


};


int main(int argc,char* argv[]){

//	FILE* fp_out = freopen("result.txt", "w", stdout);

//	RegularExpression re("0|(1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
	RegularExpression re(argv[1]);
	
	//while(1){
		std::string s(argv[2]);
		//std::cin>>s;
	bool result = re.match(s);
	
	std::cout<<"D"<<std::endl;
	std::cout<<s<<" "<<(result?"T":"F")<<std::endl;
	//}
//	fclose(fp_out);

    return 0;
}



