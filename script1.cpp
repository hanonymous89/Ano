﻿#include <iostream>
#include <string>
#include <utility>
#include <regex>
#include <stack>
#include <unordered_map>
#include <any>
#include <functional>
#include <fstream>
#include <sstream>
#include <windows.h>
template <typename T>
auto resizeAssign(std::vector<T>& vec, size_t index, T value) {
	if (vec.size() <= index) {
		vec.resize(index + 1);
	}
	return vec[index] = value;
}
template <class T>
concept AddC = requires(T t) {
	t + t;
};
template <class T>
concept SubC = requires(T t) {
	t - t;
};
template <class T>
concept MulC = requires(T t) {
	t* t;
};
template <class T>
concept DivC = requires(T t) {
	t / t;
};
template <class T>
concept LessC = requires(T t) {
	t < t;
};
template <class T>
concept SstreamC = requires(T t) {
	std::stringstream() << t;
};
class Any {
private:
	class None {};
	class Base {
	public:
		virtual ~Base() {}
		virtual std::unique_ptr<Base> clone() = 0;
		virtual Any add(Any&) = 0;
		virtual Any sub(Any&) = 0;
		virtual Any mul(Any&) = 0;
		virtual Any div(Any&) = 0;
		virtual bool less(Any&) = 0;
		virtual std::string sstream() = 0;
	};
	template <class T>
	class Derived :public Base {
	private:
		T value;
		template <bool T>
		class Implement {
		private:
		public:
		};
		template <>
		class Implement<false> {
		private:
		public:
			template <class TF, class FF, class... A>
			auto operator()(TF trueFunc, FF falseFunc, A... arguments) {
				return falseFunc(arguments...);
			}
		};
		template <>
		class Implement<true> {
		private:
		public:
			template <class TF, class FF, class... A>
			auto operator()(TF trueFunc, FF falseFunc, A... arguments) {
				return trueFunc(arguments...);
			}
		};
	public:
		Derived(T const& value) :value(value) {}
		T get() {
			return value;
		}
		std::unique_ptr<Base> clone()override {
			return std::make_unique<Derived<T> >(value);
		}
		Any add(Any& any) override {
			return Implement<AddC<T> >()([](auto value, auto any) {return Any(value + any.get<T>()); }, [](auto value, auto any) {return any; }, value, any);
		}
		Any sub(Any& any) override {
			return Implement<SubC<T> >()([](auto value, auto any) {return Any(value - any.get<T>()); }, [](auto value, auto any) {return any; }, value, any);;
		}
		Any mul(Any& any) override {
			return Implement<MulC<T> >()([](auto value, auto any) {return Any(value * any.get<T>()); }, [](auto value, auto any) {return any; }, value, any);;
		}
		Any div(Any& any) override {
			return Implement<DivC<T> >()([](auto value, auto any) {return Any(value / any.get<T>()); }, [](auto value, auto any) {return any; }, value, any);;
		}
		bool less(Any& any)override {
			return Implement<LessC<T> >()([](auto value, auto any) {return value < any.get<T>(); }, [](auto value, auto any) {return false; }, value, any);
		}
		std::string sstream()override {
			return Implement<SstreamC<T> >()([](auto value) {std::stringstream ss; ss << value; return ss.str(); }, [](auto value) {return ""; }, value);
		}
	};
	std::unique_ptr<Base> data;
public:
	template <class T>
	Any(T const& value) :data(std::make_unique<Derived<T> >(value)) {

	}
	Any() :data(std::make_unique<Derived<None> >(None())) {}
	Any(const Any& any) {
		data = any.data->clone();
	}
	template <class T>
	auto operator=(T value) {
		data = std::make_unique<Derived<T> >(value);
	}
	auto operator=(const Any& any) {
		data = any.data->clone();
	}
	auto operator+(Any& any) {
		return data->add(any);
	}
	auto operator-(Any& any) {
		return data->sub(any);
	}
	auto operator*(Any& any) {
		return data->mul(any);
	}
	auto operator/(Any& any) {
		return data->div(any);
	}
	auto operator<(Any& any) {
		return data->less(any);
	}
	auto operator>(Any& any) {
		return any.data->less(*this);
	}
	auto operator<=(Any& any) {
		return !((*this) > any);
	}
	auto operator>=(Any& any) {
		return !(any < (*this));
	}
	auto operator!=(Any& any) {
		return ((*this) < any) || (any < (*this));
	}
	template <class T>
	auto operator!=(T value) {
		Any any(value);
		return (*this) != any;
	}
	auto operator==(Any& any) {
		return !((*this) != any);
	}
	auto sstream() {
		return data->sstream();
	}
	template <class T>
	auto get() {
		auto p = dynamic_cast<Derived<T> *>(data.get());
		return p->get();
	}
};
class File {
private:
	const std::string content;
public:
	File(std::string name) :content([&]()->std::string {
		std::fstream file(name);
		if (!file.is_open())return "";
		return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		}()) {
	}
	const auto& get()const {
		return content;
	}
};
enum class TOKEN {
	T_EOF,
	NUMBER,
	RESERVED,
	IDENT,
	STRING
};
class Token {
private:
public:
	auto tokenize(std::string source) {
		std::vector<std::pair<TOKEN, std::string> > data;
		auto check = [&](auto t, auto r, std::size_t group = 0) {
			std::smatch m;
			if (!std::regex_search(source, m, r))return false;
			data.push_back({ t,m[group].str() });
			source = source.substr(m[0].str().size());
			return true;
		};
		for (; source.size();) {
			if (check(TOKEN::NUMBER, std::regex(R"(^\d+)")) ||
				check(TOKEN::RESERVED, std::regex(R"(^(this|class|private|public|else if|func|return|else|while|if|==|<=|>=|!=|[:\.;{}+-/*\(\)=<>]))")) ||
				//check(TOKEN::STRING, std::regex(R"(^".*?[^\\]")"))||
				check(TOKEN::STRING, std::regex("^\"(.*?[^\\\\])\""), 1) ||
				check(TOKEN::IDENT, std::regex(R"(^[a-zA-Z_][\w]*)"))
				) {
				continue;
			}
			source = source.substr(1);
		}
		data.push_back({ TOKEN::T_EOF,"" });
		return data;
	}
};
enum class NODE {
	NUMBER,//std::pair<numbr,int>
	ADD,
	SUB,
	MUL,
	DIV,
	VARIABLE,
	ASSIGN,
	EQUAL,
	NOT_EQUAL,
	LESS,
	EQUAL_LESS,
	IF,
	ELSE,
	WHILE,
	BLOCK,
	FUNCTION,
	PROGRAM,
	CALL,
	ARGUMENT,
	RETURN,
	NONE,
	CALL_ARGUMENT,
	ELSE_IF,
	MEMBER,
	CLASS,
	SELF
};
class Node {
private:
	/*std::vector<Node > access;*/
	std::vector<Node > access;
public:
	static constexpr auto LEFT = 0, RIGHT = 1, SELF = -1;
	NODE node = NODE::NONE;
	Any value;
	auto& operator[](size_t index) {
		if (access.size() <= index) {
			access.resize(index + 1);
		}
		return access[index];
	}
	const auto size()const {
		return access.size();
	}
};
enum class OPERATION {
	PUSH,
	POP,
	ADD,
	SUB,
	MUL,
	DIV,
	LOAD,
	STORE,
	PUSH_I,
	EQUAL,
	NOT_EQUAL,
	LESS,
	EQUAL_LESS,
	JUMP,
	JZ,
	LABEL,
	EXIT,
	RETURN,
	CALL,
	GROUP_LOAD,
	GROUP_STORE,
	GROUP_ASSIGN,
	GROUP_RETURN,
};
class Operation {
private:
public:
	OPERATION opCode;
	std::vector<Any> operands;
	Operation(decltype(opCode) opCode, decltype(operands) operands) :opCode(opCode), operands(operands) {

	}
};
class Generator {
private:
	std::string scope;
	int callScope;
	std::string className;
	std::vector<Operation> operations;
	std::unordered_map<std::string, int> variables, functions;
	std::unordered_map<std::string, std::vector<std::string> > classVal, classFunc;//className->variables
	std::unordered_map<std::string, std::string> classes;//val->className
	std::unordered_map<int, std::string >  funcArg;//func->firstArgName
	int offset, label, arg,group;
	std::unordered_map<std::string, int> classGroup;
	std::unordered_map<std::string, int> groupOffset;
	auto getVar(decltype(variables)::key_type variable) {
		if (!variables.count(variable)) {
			variables.emplace(variable, offset++);
		}
		operations.push_back(Operation(OPERATION::PUSH_I, { variables.at(variable) }));
		return variables.at(variable);
	}
	auto getFunc(decltype(variables)::key_type funcname) {
		if (!functions.count(funcname)) {
			functions.emplace(funcname, label++);
		}
		return functions.at(funcname);
	}
public:
	std::vector<Operation> codegen(Node node) {
		switch (node.node) {
		case NODE::NUMBER://NODE::VALUE
			operations.push_back(Operation(OPERATION::PUSH_I, { node.value }));
			break;
		case NODE::ADD:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation(OPERATION::POP, { 0 }));
			operations.push_back(Operation(OPERATION::POP, { 1 }));
			operations.push_back(Operation(OPERATION::ADD, { 1,0 }));
			break;
		case NODE::MUL:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation(OPERATION::POP, { 0 }));
			operations.push_back(Operation(OPERATION::POP, { 1 }));
			operations.push_back(Operation(OPERATION::MUL, { 0,1 }));
			break;
		case NODE::SUB:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation(OPERATION::POP, { 0 }));
			operations.push_back(Operation(OPERATION::POP, { 1 }));
			operations.push_back(Operation(OPERATION::SUB, { 1,0 }));
			break;
		case NODE::DIV:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation(OPERATION::POP, { 0 }));
			operations.push_back(Operation(OPERATION::POP, { 1 }));
			operations.push_back(Operation(OPERATION::DIV, { 1,0 }));
			break;
		case NODE::ASSIGN:
			if (node[Node::RIGHT].node == NODE::VARIABLE && classVal.count(node[Node::RIGHT].value.get<std::string>())) {
				classes.emplace(scope + node[Node::LEFT].value.get<std::string>(), node[Node::RIGHT].value.get<std::string>());
				classGroup.emplace(scope + node[Node::LEFT].value.get<std::string>(),group++);
				operations.push_back(Operation(OPERATION::PUSH_I, { 0 }));
				break;
			}
			else if (node[Node::RIGHT].node == NODE::VARIABLE && classes.count(scope + node[Node::RIGHT].value.get<std::string>())) {
				classGroup.emplace(scope + node[Node::LEFT].value.get<std::string>(), group++);
				operations.push_back(Operation(OPERATION::GROUP_ASSIGN,{classGroup.at(scope + node[Node::LEFT].value.get<std::string>()),classGroup.at(scope + node[Node::RIGHT].value.get<std::string>()) }));
				classes.emplace(scope + node[Node::LEFT].value.get<std::string>(), classes.at(scope + node[Node::RIGHT].value.get<std::string>()));
				operations.push_back(Operation(OPERATION::PUSH_I, { 0 }));
				break;
			}
			codegen(node[Node::RIGHT]);
			if (node[Node::LEFT].node == NODE::MEMBER&& classGroup.count(scope + node[Node::LEFT][Node::LEFT].value.get<std::string>())&& groupOffset.count(node[Node::LEFT][Node::RIGHT].value.get<std::string>())) {
				operations.push_back(Operation{ OPERATION::POP, {1} });
				operations.push_back(Operation(OPERATION::GROUP_STORE, { classGroup.at(scope + node[Node::LEFT][Node::LEFT].value.get<std::string>()),groupOffset.at(node[Node::LEFT][Node::RIGHT].value.get<std::string>()),1}));
			}
			else if(node[Node::LEFT].node == NODE::SELF&& classGroup.count(className)&& groupOffset.count(node[Node::LEFT][Node::RIGHT].value.get<std::string>())) {
				operations.push_back(Operation{ OPERATION::POP, {1} });
				operations.push_back(Operation(OPERATION::GROUP_STORE, { classGroup.at(className),groupOffset.at(node[Node::LEFT][Node::RIGHT].value.get<std::string>()),1}));
			}
			else if (node[Node::LEFT].node == NODE::VARIABLE) {
				operations.push_back(Operation{ OPERATION::POP, {1} });
				classGroup.emplace(scope + node[Node::LEFT].value.get<std::string>(), group++);
				operations.push_back(Operation(OPERATION::GROUP_STORE, { classGroup.at(scope + node[Node::LEFT].value.get<std::string>()),0,1 }));
			}
			operations.push_back(Operation{ OPERATION::PUSH, {1} });
			break;
		case NODE::VARIABLE:
		{
			operations.push_back(Operation(OPERATION::GROUP_LOAD, { 1,classGroup.at(scope + node.value.get<std::string>()),0 }));
			operations.push_back(Operation{ OPERATION::PUSH, {1} });
		}
		break;
		case NODE::EQUAL:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation{ OPERATION::POP,{0} });
			operations.push_back(Operation{ OPERATION::POP,{1} });
			operations.push_back(Operation{ OPERATION::EQUAL,{1,0} });
			break;
		case NODE::NOT_EQUAL:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation{ OPERATION::POP,{0} });
			operations.push_back(Operation{ OPERATION::POP,{1} });
			operations.push_back(Operation{ OPERATION::NOT_EQUAL,{1,0} });
			break;
		case NODE::LESS:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation{ OPERATION::POP,{0} });
			operations.push_back(Operation{ OPERATION::POP,{1} });
			operations.push_back(Operation{ OPERATION::LESS,{1,0} });
			break;
		case NODE::EQUAL_LESS:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation{ OPERATION::POP,{0} });
			operations.push_back(Operation{ OPERATION::POP,{1} });
			operations.push_back(Operation{ OPERATION::EQUAL_LESS,{1,0} });
			break;
		case NODE::IF:
			codegen(node[Node::LEFT]);
			operations.push_back(Operation(OPERATION::POP, { 0 }));
			if (node[Node::RIGHT].node == NODE::ELSE) {
				auto label1 = label++, label2 = label++;
				operations.push_back(Operation{ OPERATION::JZ,{0,label1} });
				codegen(node[Node::RIGHT][Node::LEFT]);
				operations.push_back(Operation(OPERATION::POP, { 0 }));
				operations.push_back(Operation(OPERATION::JUMP, { label2 }));
				operations.push_back(Operation(OPERATION::LABEL, { label1 }));
				codegen(node[Node::RIGHT][Node::RIGHT]);
				operations.push_back(Operation(OPERATION::POP, { 0 }));
				operations.push_back(Operation(OPERATION::LABEL, { label2 }));
			}
			else {
				auto label1 = label++;
				operations.push_back(Operation(OPERATION::JZ, { 0,label1 }));
				codegen(node[Node::RIGHT]);
				operations.push_back(Operation(OPERATION::POP, { 0 }));
				operations.push_back(Operation(OPERATION::LABEL, { label1 }));

			}
			operations.push_back(Operation(OPERATION::PUSH_I, { 0 }));
			break;
		case NODE::WHILE:
		{
			auto label1 = label++, label2 = label++;
			operations.push_back(Operation(OPERATION::LABEL, { label1 }));
			codegen(node[Node::LEFT]);
			operations.push_back(Operation(OPERATION::POP, { 0 }));
			operations.push_back(Operation(OPERATION::JZ, { 0, label2 }));
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation(OPERATION::POP, { 0 }));
			operations.push_back(Operation(OPERATION::JUMP, { label1 }));
			operations.push_back(Operation(OPERATION::LABEL, { label2 }));
			operations.push_back(Operation(OPERATION::PUSH_I, { 0 }));
		}
		break;
		case NODE::BLOCK:
			if (node[Node::LEFT].node != NODE::NONE) {
				codegen(node[Node::LEFT]);
				operations.push_back(Operation(OPERATION::POP, { 0 }));
			}
			codegen(node[Node::RIGHT]);
			break;
		case NODE::PROGRAM:
			codegen(node[Node::LEFT]);
			codegen(node[Node::RIGHT]);
			break;
		case NODE::FUNCTION:
		{
			if (node[Node::LEFT].node == NODE::CALL) {
				auto func = getFunc(node[Node::LEFT].value.get<std::string>());
				scope = std::to_string(func);
				operations.push_back(Operation(OPERATION::LABEL, { func }));
				codegen(node[Node::RIGHT]);
				break;
			}
			auto func = getFunc(node[Node::LEFT][Node::LEFT].value.get<std::string>());
			scope = std::to_string(func);
			if (node[Node::LEFT][Node::RIGHT].node==NODE::VARIABLE) {
				funcArg.emplace(std::stoi(scope), scope + node[Node::LEFT][Node::RIGHT].value.get<std::string>());
				classGroup.emplace(scope + node[Node::LEFT][Node::RIGHT].value.get<std::string>(), group++);
			}
			else codegen(node[Node::LEFT][Node::RIGHT]);
			operations.push_back(Operation(OPERATION::LABEL, { func }));
			codegen(node[Node::RIGHT]);
		}
		break;
		case NODE::RETURN:
			codegen(node[Node::RIGHT]);
			operations.push_back(Operation(OPERATION::RETURN, {}));//return value?
			break;
		case NODE::CALL:
			operations.push_back(Operation(OPERATION::CALL, { getFunc(node.value.get<std::string>()) }));
			break;
		case NODE::ARGUMENT:
		{
			const auto registArg = [&](auto self,auto argument)->void {
				if (argument.node == NODE::ARGUMENT) {
					self(self,argument[Node::LEFT]);
				}
				else {
					funcArg.emplace(std::stoi(scope),scope + argument.value.get<std::string>());
					classGroup.emplace(scope + argument.value.get<std::string>(), group++);
				}
				if (argument[Node::RIGHT].node != NODE::VARIABLE)return;
				classGroup.emplace(scope + argument[Node::RIGHT].value.get<std::string>(), group++);
			};
			registArg(registArg,node);
		}
			break;
		case NODE::CALL_ARGUMENT:
		{
			auto arg = 0;
			const auto assignArg = [&](auto self, auto argument)->void {
				if (argument.node == NODE::CALL_ARGUMENT) {
					self(self, argument[Node::LEFT]);
				}//memberが来ている.
				else if(argument.node==NODE::MEMBER) {
					operations.push_back(Operation(OPERATION::GROUP_LOAD, {1,classGroup.at(scope+argument[Node::LEFT].value.get<std::string>()),groupOffset.at(argument[Node::RIGHT].value.get<std::string>())}));
					operations.push_back(Operation(OPERATION::GROUP_STORE, { (arg++) + classGroup.at(funcArg.at(getFunc(node[Node::LEFT].value.get<std::string>()))),0,1 }));
					return;
				}
				else if (argument.node == NODE::NUMBER) {
					codegen(argument);
					operations.push_back(Operation{ OPERATION::POP,{0} });
					operations.push_back(Operation(OPERATION::GROUP_STORE, { (arg++) + classGroup.at(funcArg.at(getFunc(node[Node::LEFT].value.get<std::string>()))),0,0 }));
					return;
				}
				else {
					operations.push_back(Operation(OPERATION::GROUP_ASSIGN, { (arg++) + classGroup.at(funcArg.at(getFunc(node[Node::LEFT].value.get<std::string>()))),classGroup.at(scope + argument.value.get<std::string>()) }));
				}
				if (argument[Node::RIGHT].node == NODE::MEMBER) {
					operations.push_back(Operation(OPERATION::GROUP_LOAD, { 1,classGroup.at(scope + argument[Node::RIGHT][Node::LEFT].value.get<std::string>()),groupOffset.at(argument[Node::RIGHT][Node::RIGHT].value.get<std::string>()) }));
					operations.push_back(Operation(OPERATION::GROUP_STORE, { (arg++) + classGroup.at(funcArg.at(getFunc(node[Node::LEFT].value.get<std::string>()))),0,1 }));
					return;
				}
				else if (argument[Node::RIGHT].node == NODE::NUMBER) {
					codegen(argument[Node::RIGHT]);
					operations.push_back(Operation{ OPERATION::POP,{0} });
					operations.push_back(Operation(OPERATION::GROUP_STORE, { (arg++) + classGroup.at(funcArg.at(getFunc(node[Node::LEFT].value.get<std::string>()))),0,0 }));
					return;
				}
				else if (argument[Node::RIGHT].node != NODE::VARIABLE)return;
				operations.push_back(Operation(OPERATION::GROUP_ASSIGN, { (arg++) + classGroup.at(funcArg.at(getFunc(node[Node::LEFT].value.get<std::string>()))),classGroup.at(scope + argument[Node::RIGHT].value.get<std::string>())}));
			};
			assignArg(assignArg, node[Node::RIGHT]);
			codegen(node[Node::LEFT]);
		}
		break;
		case NODE::CLASS:
		{
			className = node[Node::LEFT].value.get<std::string>();
			classVal.emplace(node[Node::LEFT].value.get<std::string>(), std::vector<std::string>{});
			classFunc.emplace(node[Node::LEFT].value.get<std::string>(), std::vector<std::string>{});
			classGroup.emplace(className,group++);
			auto gOffset = 0;
			const auto pushBack = [&](auto self, auto val) {
				if (val.node == NODE::FUNCTION && val[Node::RIGHT][Node::LEFT].node == NODE::CALL) {
					classFunc.at(node[Node::LEFT].value.get<std::string>()).push_back(val[Node::RIGHT][Node::LEFT].value.get<std::string>());
					val[Node::RIGHT][Node::LEFT].value = node[Node::LEFT].value.get<std::string>() + "." + val[Node::RIGHT][Node::LEFT].value.get<std::string>();
					codegen(val[Node::RIGHT]);
				}
				else if (val.node == NODE::FUNCTION && val[Node::RIGHT][Node::LEFT].node == NODE::ARGUMENT) {
					classFunc.at(node[Node::LEFT].value.get<std::string>()).push_back(val[Node::RIGHT][Node::LEFT][Node::LEFT].value.get<std::string>());
					val[Node::RIGHT][Node::LEFT][Node::LEFT].value = node[Node::LEFT].value.get<std::string>() + "." + val[Node::RIGHT][Node::LEFT][Node::LEFT].value.get<std::string>();
					codegen(val[Node::RIGHT]);
				}
				else {
					groupOffset.emplace(val.value.get<std::string>(),gOffset);
					groupOffset.emplace(className, gOffset++);
					classVal.at(node[Node::LEFT].value.get<std::string>()).push_back(val.value.get<std::string>());
					getVar(node[Node::LEFT].value.get<std::string>() + "." + val.value.get<std::string>());
				}
				if (!val.size())return;
				self(self, val[Node::LEFT]);
			};
			pushBack(pushBack, node[Node::RIGHT]);
		}

		break;
		case NODE::MEMBER:
		{
			if (node[Node::RIGHT].node == NODE::CALL && classes.count(scope + node[Node::LEFT].value.get<std::string>()) && classFunc.count(classes.at(scope + node[Node::LEFT].value.get<std::string>()))) {
				node[Node::RIGHT].value = classes.at(scope + node[Node::LEFT].value.get<std::string>()) + "." + node[Node::RIGHT].value.get<std::string>();
				operations.push_back(Operation(OPERATION::GROUP_ASSIGN, { classGroup.at(className),classGroup.at(scope + node[Node::LEFT].value.get<std::string>()) }));
				codegen(node[Node::RIGHT]);
				operations.push_back(Operation(OPERATION::GROUP_ASSIGN, { classGroup.at(scope + node[Node::LEFT].value.get<std::string>()),classGroup.at(className) }));
				break;
			}
			else if (node[Node::RIGHT].node == NODE::CALL_ARGUMENT && classes.count(scope + node[Node::LEFT].value.get<std::string>()) && classFunc.count(classes.at(scope + node[Node::LEFT].value.get<std::string>()))) {
				node[Node::RIGHT][Node::LEFT].value = classes.at(scope + node[Node::LEFT].value.get<std::string>()) + "." + node[Node::RIGHT][Node::LEFT].value.get<std::string>();
				operations.push_back(Operation(OPERATION::GROUP_ASSIGN, { classGroup.at(className),classGroup.at(scope + node[Node::LEFT].value.get<std::string>()) }));
				codegen(node[Node::RIGHT]);
				operations.push_back(Operation(OPERATION::GROUP_ASSIGN, { classGroup.at(scope + node[Node::LEFT].value.get<std::string>()),classGroup.at(className) }));
				break;
			}
			else if (node[Node::RIGHT].node == NODE::VARIABLE&& classGroup.count(scope + node[Node::LEFT].value.get<std::string>())&& groupOffset.count(node[Node::RIGHT].value.get<std::string>())) {
				operations.push_back(Operation(OPERATION::GROUP_LOAD, { 1,classGroup.at(scope + node[Node::LEFT].value.get<std::string>()),groupOffset.at(node[Node::RIGHT].value.get<std::string>())}));
				operations.push_back(Operation{ OPERATION::PUSH, {1} });
				break;
			}
		}

		break;
		case NODE::SELF:
			if (!groupOffset.count(node[Node::RIGHT].value.get<std::string>())) {
				break;
			}
			operations.push_back(Operation(OPERATION::GROUP_LOAD,{1,classGroup.at(className),groupOffset.at(node[Node::RIGHT].value.get<std::string>())}));
			operations.push_back(Operation{ OPERATION::PUSH, {1} });
			break;
		}

		return operations;
	}
	std::vector<Operation> codegen(std::pair<bool, Node> nodes) {
		codegen(nodes.second);
		operations.clear();
		codegen(nodes.second);
		operations.emplace(operations.begin(), Operation(OPERATION::CALL, { getFunc("main") }));
		operations.emplace(std::next(operations.begin(), 1), Operation(OPERATION::EXIT, {}));
		return operations;
	}

};
class VM {
private:
	std::stack<Any> stack;
	template <class T>
	class resizeVector {
	private:
		std::vector<T> vec;
		auto size() {
			return vec.size();
		}
	public:
		auto &operator[](int index) {
			if (vec.size() <= index) {
				vec.resize(index+1);
			}
			return vec[index];
		}
		auto operator=(resizeVector<T> target) {
			vec.resize(target.size());
			std::ranges::copy(target,vec.begin());
		}
		auto begin() {
			return vec.begin();
		}
		auto end() {
			return vec.end();
		}
	};
	resizeVector<Any> reg;
	resizeVector<Any> val;
	resizeVector<resizeVector<Any> > group;//group[0]は一時的に使用する　値渡しとかメンバーのやつとか
public:
	auto run(std::vector<Operation> code) {
		std::unordered_map<int, decltype(code)::iterator > labels;
		for (auto op = code.begin(); op != code.end(); ++op) {
			if ((*op).opCode == OPERATION::LABEL) {
				labels.insert_or_assign((*op).operands[0].get<int>(), op);
			}
		}
		for (auto op = code.begin();
			op->opCode != OPERATION::EXIT; ++op) {
			switch (op->opCode) {
			case OPERATION::PUSH:
				stack.push(reg[op->operands[0].get<int>()]);
				break;
			case OPERATION::POP:
				reg[op->operands[0].get<int>()] = stack.top();
				stack.pop();
				break;
			case OPERATION::ADD:
				stack.push(reg[op->operands[0].get<int>()] + reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::MUL:
				stack.push(reg[op->operands[0].get<int>()] * reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::SUB:
				stack.push(reg[op->operands[0].get<int>()] - reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::DIV:
				stack.push(reg[op->operands[0].get<int>()] / reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::PUSH_I:
				stack.push(op->operands[0]);
				break;
			case OPERATION::STORE:
				val[reg[op->operands[0].get<int>()].get<int>()] = reg[op->operands[1].get<int>()];
				break;
			case OPERATION::LOAD:
				reg[op->operands[0].get<int>()] = val[reg[op->operands[1].get<int>()].get<int>()];
				break;
			case OPERATION::EQUAL:
				stack.push(reg[op->operands[0].get<int>()] == reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::NOT_EQUAL:
				stack.push(reg[op->operands[0].get<int>()] != reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::LESS:
				stack.push(reg[op->operands[0].get<int>()] < reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::EQUAL_LESS:
				stack.push(reg[op->operands[0].get<int>()] <= reg[op->operands[1].get<int>()]);
				break;
			case OPERATION::JUMP:
				op = labels[op->operands[0].get<int>()];
				break;
			case OPERATION::JZ:
				if (reg[op->operands[0].get<int>()].get<bool>())break;//キャスト演算子をオーバーライドしてやればget<bool>いらない
				op = labels[op->operands[1].get<int>()];
				break;
			case OPERATION::CALL:
				//stack.push(stack);
				stack.push(op);
				op = labels[op->operands[0].get<int>()];
				break;
			case OPERATION::RETURN:
				reg[2] = stack.top();
				stack.pop();
				op = stack.top().get<decltype(op)>();
				stack.pop();
				stack.push(reg[2]);
				break;
			case OPERATION::GROUP_ASSIGN:
				group[op->operands[0].get<int>()] = group[op->operands[1].get<int>()];
				break;
			case OPERATION::GROUP_LOAD:
				reg[op->operands[0].get<int>()] = group[op->operands[1].get<int>()][op->operands[2].get<int>()];
				break;
			case OPERATION::GROUP_STORE:
				group[op->operands[0].get<int>()][op->operands[1].get<int>()]=reg[op->operands[2].get<int>()];
				break;
			}
		}
		return reg[2];
	}
};
class BNF {
private:
	std::function<std::pair<bool, Node>()> bnf;
public:
	BNF() {};
	BNF(decltype(bnf) bnf) :bnf(bnf) {}
	auto operator()() {
		return bnf();
	}
};
class Expression {
private:
public:
	auto expect(std::vector<std::pair<TOKEN, std::string> >::iterator& iter, BNF bnf, std::string_view str) {
		return BNF([=, &iter]()mutable {
			auto prev = iter;
			auto node = bnf();
			if (!node.first)return std::pair(false, Node());
			if (iter->first != TOKEN::T_EOF && iter->second == str) {
				++iter;
				return node;
			}
			iter = prev;
			node.first = false;
			return node;
			});
	}
	auto reverce(BNF bnf) {
		return BNF([=]()mutable {
			auto node = bnf();
			if (!node.first)return node;
			std::swap(node.second[Node::LEFT], node.second[Node::RIGHT]);
			return node;
			});
	}
	auto regist(NODE nodeOp, BNF right, decltype(Node::SELF) access = Node::RIGHT) {
		return BNF([=]()mutable {
			auto node = right();
			if (!node.first) return node;
			if (access == Node::SELF) {
				node.second.node = nodeOp;
				return node;
			}
			Node temp;
			temp[access] = node.second;
			temp.node = nodeOp;
			return std::pair(true, temp);
			});
	}
	auto link(std::vector<std::pair<TOKEN, std::string> >::iterator& iter, std::string_view str, BNF right) {
		return BNF([=, &iter]()mutable {
			if (iter->first != TOKEN::T_EOF && iter->second == str) {
				++iter;
				return right();
			}
			return std::pair(false, Node());
			});
	}
	auto link(std::vector<std::pair<TOKEN, std::string> >::iterator& iter, BNF left, BNF right) {
		return BNF([=, &iter]()mutable {
			auto prev = iter;
			auto node = left();
			if (!node.first) {
				iter = prev; return std::pair(false, node.second);
			}
			auto nodeRight = right();
			if (!nodeRight.first) { iter = prev; return std::pair(false, node.second); }
			nodeRight.second[Node::LEFT] = node.second;
			node = nodeRight;
			return node;
			});
	}
	auto _or(BNF left, BNF right) {
		return BNF([=]()mutable {
			auto node = left();
			if (node.first || (node = right()).first) {
				node.first = true;
				return node;
			}
			return node;
			});
	}
	auto ifAny(BNF left, BNF right) {
		return BNF([=]()mutable {
			auto node = left();
			if (!node.first)return node;
			auto nodeRight = right();
			if (!nodeRight.first)return node;
			nodeRight.second[Node::LEFT] = node.second;
			node = nodeRight;
			return node;
			});
	}
	auto loop(BNF left, BNF right) {
		return BNF([=]()mutable {
			auto node = left();
			while (node.first) {
				auto nodeRight = right();
				if (!nodeRight.first)return node;
				nodeRight.second[Node::LEFT] = node.second;
				node = nodeRight;
			}
			return node;
			});
	}
};
int main() {
	//auto source = File("data.txt").get();
	//std::cout << source;
	//system("pause");
	//auto data = Token().tokenize(std::regex_replace(source, std::regex(R"(\n)"), ""));
	auto data = Token().tokenize(std::regex_replace(File("data.txt").get(), std::regex(R"(\n)"), ""));
	auto iter = data.begin();
	BNF number([&] {
		if (iter->first != TOKEN::NUMBER && iter->first != TOKEN::STRING)return std::pair{ false,Node() };
		Node node;
		node.node = NODE::NUMBER;
		if (iter->first == TOKEN::STRING)node.value = iter->second;
		else node.value = std::stoi((*iter).second);
		++iter;
		return std::pair(true, node);
		});
	BNF ident([&] {
		if (iter->first != TOKEN::IDENT) {
			return std::pair(false, Node());
		}
		Node node;
		node.value = (iter++)->second;
		if (iter->second == "(") {
			node.node = NODE::CALL;
		}
		else node.node = NODE::VARIABLE;
		return std::pair(true, node);
		});
	Expression e;
	BNF program, mul, add, primary, expr, compare, ifExpr, whileExpr, block, ret, call, function, member, registClass, self;//menber function? menber variable?????
	BNF unary([&] {
		if (iter->second == "+") {
			++iter;
		}
		else if (iter->second == "-") {
			++iter;
			auto node = primary();
			node.second[Node::RIGHT] = node.second;
			node.second.value = 0;
			node.second[Node::LEFT] = node.second;
			node.second.node = NODE::SUB;
			return node;
		}
		return primary();
		});
	mul = e.loop(unary, e._or(e.regist(NODE::MUL, e.link(iter, "*", unary)), e.regist(NODE::DIV, e.link(iter, "/", unary))));
	add = e.loop(mul, e._or(e.regist(NODE::ADD, e.link(iter, "+", mul)), e.regist(NODE::SUB, e.link(iter, "-", mul))));
	compare = e._or(e._or(e.link(iter, add, e._or(e.regist(NODE::EQUAL, e.link(iter, "==", add)), e._or(e.regist(NODE::NOT_EQUAL, e.link(iter, "!=", add)), e._or(e.regist(NODE::LESS, e.link(iter, "<", add)), e.regist(NODE::EQUAL_LESS, e.link(iter, "<=", add)))))), e.reverce(e.link(iter, add, e._or(e.regist(NODE::LESS, e.link(iter, ">", add)), e.regist(NODE::EQUAL_LESS, e.link(iter, ">=", add)))))), add);
	whileExpr = BNF([&] {
		static auto bnf = e.link(iter, e.expect(iter, e.link(iter, "while", e.link(iter, "(", compare)), ")"), e.regist(NODE::WHILE, expr));
		return bnf();
		});
	ifExpr = BNF([&] {
		static auto bnf = e.link(iter, e.link(iter, "if", e.expect(iter, e.link(iter, "(", compare), ")")),
			e.regist(NODE::IF,
				e.ifAny(
					//e.loop(expr,e.link(iter, e.link(iter, "else if", e.expect(iter, e.link(iter, "(", compare), ")")),e.regist(NODE::ELSE_IF,expr))),
					expr,
					e.link(iter,
						"else",
						e.regist(NODE::ELSE,
							expr)))
			)
		);
		return bnf();
		});
	block = BNF([&] {
		static auto bnf = e.expect(iter, e.link(iter, "{", e.regist(NODE::BLOCK, e.loop(e.expect(iter, expr, ";"), e.regist(NODE::BLOCK, e.expect(iter, expr, ";"))))), "}");
		return bnf();
		});
	call = BNF([&] {
		static auto bnf = e.expect(iter, e.ifAny(e.expect(iter, ident, "("), e.regist(NODE::CALL_ARGUMENT, e.loop(expr, e.link(iter, ",", e.regist(NODE::CALL_ARGUMENT, expr))))), ")");
		return bnf();
		});
	ret = BNF([&] {
		static auto bnf = e.link(iter, "return", e.regist(NODE::RETURN, expr));
		return bnf();
		});
	member = e.link(iter, e.expect(iter, e._or(call, ident), "."), e.regist(NODE::MEMBER, e.loop(e._or(call, ident),e.regist(NODE::MEMBER,e.link(iter,".",e._or(call,ident))) )));
	self = e.link(iter, "this", e.link(iter, ".", e.regist(NODE::SELF, e._or(call, ident))));
	primary = e._or(e._or(e._or(e.expect(iter, e.link(iter, "(", compare), ")"), call), e._or(self, e._or(member, ident))), number);
	function = BNF([&] {
		static auto bnf = e.link(iter, e.ifAny(e.link(iter, "func", e.expect(iter, ident, "(")), e.regist(NODE::ARGUMENT, e.loop(expr, e.link(iter, ",", e.regist(NODE::ARGUMENT, expr))))), e.link(iter, ")", e.regist(NODE::FUNCTION, block)));
		return bnf();
		});
	registClass = e.expect(iter, e.link(iter, e.expect(iter, e.link(iter, "class", ident), "{"), e.regist(NODE::CLASS, e.loop(e.loop(e.expect(iter, ident, ";"), e.expect(iter, ident, ";")), e.regist(NODE::FUNCTION, function)))), "}");
	expr = e._or(e._or(e._or(e._or(e._or(e.link(iter, e._or(self, e._or(member, ident)), e.regist(NODE::ASSIGN, e.link(iter, "=", compare))), whileExpr), ifExpr), ret), block), compare);
	program = e.loop(e._or(registClass, function), e.regist(NODE::PROGRAM, e._or(registClass, function)));
	Generator generator;
	VM vm;
	std::cout << vm.run(generator.codegen(program())).sstream();
	return EXIT_SUCCESS;
}