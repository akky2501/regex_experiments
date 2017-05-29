
#define _CRT_SECURE_NO_WARNINGS

//2015.7.9 一応ピリオド対応させたけどちゃんと動くかワカラン


#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<set>
#include<deque>

typedef int StateID;
typedef char EventID;//ここでは-1をε遷移とする,-2を'.'の挙動とする

const EventID EpsilonEvent = -1;
const EventID AnyEvent = -2;

class Converter;


bool operator==(const std::set<StateID>& lhs, const std::set<StateID>& rhs){
	if (lhs.size() != rhs.size()) return false;
	auto lit = lhs.begin();
	auto rit = rhs.begin();
	for (size_t i = 0; i < lhs.size(); i++ , lit++ , rit++){
		if (*lit != *rit) return false;
	}

	return true;
}


class NFA{
	friend Converter;
public:
	NFA(){}

	~NFA(){}

	StateID makeState(void){
		states.push_back(State());

		return states.size() - 1;
	}

	void setEvent(StateID next, StateID now, EventID ev){
		states[now].table.emplace_back(ev, next);
	}

	int countState(void){ return states.size(); }

private:

	struct State{
		State(){}
		typedef std::vector<std::pair<EventID, StateID> > Map;
		Map table;
	};

	std::vector<State> states;

	std::string::iterator send;//string end

};

class DFA{
	friend Converter;
public:
	DFA(){}
	~DFA(){}
	
	std::string::iterator accept(std::string::iterator &it){
		std::string::iterator tmp = it;
		std::string::iterator str_end = str.end();
		StateID current = 0;

		bool accepted = false;
		while (tmp != str_end){
			auto x = states[current].table.find(AnyEvent);
			if (x == states[current].table.end()){
				x = states[current].table.find(*it);
				if (x == states[current].table.end()) break;
			}

			current = (*x).second;
			tmp++;

			if (states[current].is_end) accepted = true;

			
			/*if (it == str_end){
				if (states[current].is_end) return true;
				else break;
			}*/
		}

		if (states[current].is_end) return ++tmp;//つぎの文字をさしておく

		return it;
	}

	/*void printState(void){
		int i = 0;

		std::cout << "DFA State---------------------" << std::endl;
		for (auto& it : states){
			std::cout << i << " : {" << std::endl;

			std::cout << "	end : " << ((it.is_end) ? "true" : "false") << std::endl;

			std::cout << "	table : {" << std::endl;
			for (auto& t : it.table){
				std::cout << "		[ " << "'" << t.first << "'" << " -> " << t.second << " ]" << std::endl;
			}
			std::cout << "	}" << std::endl;

			std::cout << "}" << std::endl;

			std::cout << std::endl;

			i++;
		}
	}*/

private:
	
	struct State{
		State(){}
		typedef std::map<EventID, StateID> Map;
		Map table;//先頭が開始状態
		bool is_end;
	};

	std::vector<State> states;
};


class Converter{//NFA to DFA Converter
public:
	Converter(NFA& nfa_)
	:	 nfa(nfa_)
	{
		states.push_back(State());

		convert();
	}

	~Converter(){}

	void make(DFA* dfa){
		optimize();//最適化(未実装)

		for (auto &it : states){
			dfa->states.push_back(it.dfa_state);
		}

	}

private:
	NFA& nfa;
	struct State{
		State(){ dfa_state.is_end = false; }
		State(std::set<StateID>& nfa_state_,bool is_end_)
			:	nfa_state(nfa_state_)
		{
			dfa_state.is_end = is_end_;
		}
		std::set<StateID> nfa_state;
		DFA::State dfa_state;
	};

	std::vector<State> states;

	

	void convert(void){
		bool updated = false;
		StateID current = 0;
		std::deque<StateID> que;


		states[0].nfa_state.insert(0);
		que.push_back(0);
		while(!que.empty()){
			current = que.front();
			states[current].nfa_state = findEpsilonState(states[current].nfa_state);//ε遷移できる状態をまとめる
			//まとめた状態に終点があるかどうか確認が必要->最後に一括して行う
			std::set<EventID> events;
			for (auto i : states[current].nfa_state){//まとめた状態からの可能なイベントの列挙
				for (auto j : nfa.states[i].table){
					if (j.first != EpsilonEvent) events.insert(j.first);
				}
			}

			for (auto i : events){
				std::set<StateID> nfa_same_state = findSameEventState(states[current].nfa_state, i);//同じイベント一回で遷移する状態(その状態からのε遷移も含む)をまとめる
				StateID id;
				if ((id = findRegisteredState(nfa_same_state)) == -1/*未登録*/){
					id = registerState(nfa_same_state);
					que.push_back(id);
				}

				states[current].dfa_state.table[i] = id;//遷移先に追加
			}
			que.pop_front();
		} 

		//最後に終点を処理する
		for (auto& it : states){
			if (it.nfa_state.count(nfa.countState() - 1) == 1) it.dfa_state.is_end = true;
		}

		//DFA出来上がり！
	}

	StateID registerState(std::set<StateID>& nfa_state){
		/*int end = nfa_state.count(nfa.countState()-1);
		states.emplace_back(nfa_state,(end == 1) ? true : false);*/
		states.emplace_back(nfa_state,false);
		return states.size() - 1;
	}

	StateID findRegisteredState(std::set<StateID>& nfa_state){
		int id = 0;
		for (auto& it : states){
			if (it.nfa_state == nfa_state) return id;
				id++;
		}

		return -1;
	}

	std::set<StateID> findEpsilonState(std::set<StateID>& nfa_state){
		std::set<StateID> res;
		std::deque<StateID> que;

		for (auto x : nfa_state){//まとめられているすべてのNFA状態について幅優先探索
			res.insert(x);
			que.push_back(x);
			while(!que.empty()){	
				for (auto& i : nfa.states[que.front()].table){
					if (i.first == EpsilonEvent){
						res.insert(i.second);
						que.push_back(i.second);
					}
				}
				que.pop_front();
			}
		}

		return res;
	}

	std::set<StateID> findSameEventState(std::set<StateID>& nfa_state, EventID ev){
		std::set<StateID> res;

		for (auto x : nfa_state){
			for (auto& i : nfa.states[x].table){
				if (i.first == ev){ //1段階目とそこからのε遷移を登録
					res.insert(i.second); 
					res = findEpsilonState(res);
				}
			}
		}

		return res;
	}


	void optimize(void){
		while (marge());
	}

	bool marge(void){
		return false;
	}

};

class Lexer{//正規表現用一文字先読みレキサー
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

/*
char *OperatorString[]={
    "Empty",
    "Char",
	"Any",
    "Connect",
    "Or",
    "Closure"
};
*/

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
        Node* root = parse();

		convertNFA(root);
		
		convertDFA(nfa);

		//dfa.printState();
		
		delete root;

    }

    ~RegularExpression(){
    }

	std::string::iterator match(std::string::iterator& str){
		return dfa.accept(str);
	}

private:
    Lexer lex;
	NFA nfa;
	DFA dfa;


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


	void convertNFA(Node* node){
		StateID initial = nfa.makeState();

		StateID tmp = convertNFAImpl(initial,node);

		StateID final = nfa.makeState();//最後に作ったステートが目的状態

		nfa.setEvent(final,tmp,EpsilonEvent);

	}

	StateID convertNFAImpl(StateID now,Node* node){

		StateID next;

		switch (node->op){
		case opEmpty:{
			next = nfa.makeState();
			nfa.setEvent(next,now,EpsilonEvent);
			break;
		}
		case opChar:{
			next = nfa.makeState();
			nfa.setEvent(next,now,node->c);
			break;
		}
		case opAny:{
			next = nfa.makeState();
			nfa.setEvent(next, now, AnyEvent);
			break;
		}
		case opConnect:{
			StateID t0 = convertNFAImpl(now, node->lhs);
			next = convertNFAImpl(t0, node->rhs);
			break;
		}
		case opOr:{
			next = nfa.makeState();
			StateID t0 = convertNFAImpl(now, node->lhs);
			StateID t1 = convertNFAImpl(now, node->rhs);
			nfa.setEvent(next,t0, EpsilonEvent);
			nfa.setEvent(next,t1, EpsilonEvent);
			break; 
		}
		case opClosure:{
			next = nfa.makeState();
			StateID t0 = nfa.makeState();
			StateID t1 = convertNFAImpl(t0,node->lhs);
			nfa.setEvent(t0, now, EpsilonEvent);
			nfa.setEvent(next, now, EpsilonEvent);
			nfa.setEvent(t0, t1, EpsilonEvent);
			nfa.setEvent(next, t1, EpsilonEvent);

		}break;
		default:break;//おそらくないと思われる
		}

		return next;

	}

	void convertDFA(NFA& nfa){
		Converter cvt(nfa);

		cvt.make(&dfa);

	}

};


int main(void){
	
	//FILE* fp_out = freopen("result.txt", "w", stdout);

	//RegularExpression integer("0|(1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
	RegularExpression symbol("((a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)|(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z)|_)((a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)|(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z)|_|(0|1|2|3|4|5|6|7|8|9))*");
	std::cout << "!!" << std::endl;

	std::string str;
	while (1){
		std::cin >> str;
		std::cout << symbol.match(str) << std::endl;
	}
	//fclose(fp_out);

    return 0;
}



