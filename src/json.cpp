//
// Created by menv on 9/4/26.
//

#include "../include/shark/json.hpp"

#include "shark/json.hpp"

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace shark {
namespace {

class Parser {
public:
	explicit Parser(std::string_view input) : input_(input) {}
	Json parse() { auto value = parse_value(); ws(); if (pos_ != input_.size()) fail("trailing data"); return value; }

private:
	std::string_view input_; size_t pos_ = 0;
	[[noreturn]] void fail(const char* what) const { throw std::runtime_error("JSON byte " + std::to_string(pos_) + ": " + what); }
	void ws() { while (pos_ < input_.size() && std::string_view(" \t\r\n").find(input_[pos_]) != std::string_view::npos) ++pos_; }
	bool take(char c) { ws(); if (pos_ < input_.size() && input_[pos_] == c) { ++pos_; return true; } return false; }
	void literal(std::string_view x) { if (input_.substr(pos_, x.size()) != x) fail("invalid literal"); pos_ += x.size(); }

	Json parse_value() {
		ws(); if (pos_ == input_.size()) fail("expected value");
		switch (input_[pos_]) {
		case 'n': literal("null"); return Json(nullptr); case 't': literal("true"); return Json(true);
		case 'f': literal("false"); return Json(false); case '"': return Json(parse_string());
		case '[': return Json(parse_array()); case '{': return Json(parse_object());
		default: return Json(parse_number());
		}
	}
	static void utf8(std::string& out, uint32_t cp) {
		if (cp <= 0x7f) out += static_cast<char>(cp);
		else if (cp <= 0x7ff) { out += static_cast<char>(0xc0 | cp >> 6); out += static_cast<char>(0x80 | (cp & 63)); }
		else if (cp <= 0xffff) { out += static_cast<char>(0xe0 | cp >> 12); out += static_cast<char>(0x80 | ((cp >> 6) & 63)); out += static_cast<char>(0x80 | (cp & 63)); }
		else { out += static_cast<char>(0xf0 | cp >> 18); out += static_cast<char>(0x80 | ((cp >> 12) & 63)); out += static_cast<char>(0x80 | ((cp >> 6) & 63)); out += static_cast<char>(0x80 | (cp & 63)); }
	}
	uint32_t hex4() { uint32_t x = 0; for (int n = 0; n < 4; ++n) { if (pos_ >= input_.size()) fail("short unicode escape"); char c=input_[pos_++]; x <<= 4; if(c>='0'&&c<='9')x+=c-'0';else if(c>='a'&&c<='f')x+=c-'a'+10;else if(c>='A'&&c<='F')x+=c-'A'+10;else fail("bad unicode escape"); } return x; }
	std::string parse_string() {
		if (!take('"')) fail("expected string");
		std::string out;
		while (pos_ < input_.size()) { char c=input_[pos_++]; if(c=='"') return out; if(static_cast<unsigned char>(c)<0x20) fail("control in string"); if(c!='\\'){out+=c;continue;} if(pos_==input_.size())fail("short escape"); c=input_[pos_++];
			switch(c){case '"':out+='"';break;case '\\':out+='\\';break;case '/':out+='/';break;case 'b':out+='\b';break;case 'f':out+='\f';break;case 'n':out+='\n';break;case 'r':out+='\r';break;case 't':out+='\t';break;case 'u':{auto cp=hex4();if(cp>=0xd800&&cp<=0xdbff){if(input_.substr(pos_,2)!="\\u")fail("missing low surrogate");pos_+=2;auto lo=hex4();if(lo<0xdc00||lo>0xdfff)fail("bad low surrogate");cp=0x10000+((cp-0xd800)<<10)+(lo-0xdc00);}utf8(out,cp);break;}default:fail("bad escape");}
		} fail("unterminated string");
	}
	double parse_number() { ws(); auto first=input_.data()+pos_; char* end=nullptr; auto x=std::strtod(first,&end); if(end==first)fail("expected number"); pos_ += static_cast<size_t>(end-first); if(!std::isfinite(x))fail("non-finite number"); return x; }
	Json::Array parse_array() { take('['); Json::Array a; if(take(']'))return a; do { a.push_back(parse_value()); } while(take(',')); if(!take(']'))fail("expected ]"); return a; }
	Json::Object parse_object() { take('{'); Json::Object o; if(take('}'))return o; do { ws(); auto k=parse_string(); if(!take(':'))fail("expected :"); if(!o.emplace(std::move(k),parse_value()).second)fail("duplicate key"); } while(take(',')); if(!take('}'))fail("expected }"); return o; }
};
} // namespace

Json Json::parse(std::string_view text) { return Parser(text).parse(); }
bool Json::is_null() const { return std::holds_alternative<std::nullptr_t>(value_); }
bool Json::is_string() const { return std::holds_alternative<std::string>(value_); }
const std::string& Json::string() const { return std::get<std::string>(value_); }
double Json::number() const { return std::get<double>(value_); }
const Json::Array& Json::array() const { return std::get<Array>(value_); }
const Json::Object& Json::object() const { return std::get<Object>(value_); }
const Json& Json::at(const std::string& key) const { auto i=object().find(key); if(i==object().end())throw std::runtime_error("missing JSON key: "+key); return i->second; }
const Json* Json::find(const std::string& key) const { auto* o=std::get_if<Object>(&value_); if(!o)return nullptr; auto i=o->find(key); return i==o->end()?nullptr:&i->second; }

} // namespace shark


