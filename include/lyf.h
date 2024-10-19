#ifndef LYF_H
#define LYF_H

#include <cassert>
#include <cxxabi.h>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <sstream>
#include <regex>
#include <iterator>

// 用于获取变量类型的快捷调用宏, 此方法会忽略掉没有显示指明的默认参数类型, 
// 例如template<class T, class U = string> class A {};则对于auto a = A<Color>();将返回A<Color>而不是A<Color, string>, 若要获取完整的类型, 可以使用getWholeTypeName(x)方法
#define typeof(x) lyf::getTypeName<decltype(x)>()
#define getWholeTypeName(x) lyf::type_class<decltype(x)>::get()	// 用于获取变量类型的快捷调用宏, 此方法会获取完整的类型, 包括默认参数类型
#define m_print(arg) std::cout << #arg << " = " << arg << std::endl	// 用于快捷打印变量名和值的宏

namespace lyf {
	using std::cout, std::endl;
	using std::string, std::vector;
	using std::size_t;

	/// 采用模板参数的方式, 基于宏__PRETTY_FUNCTION__获取类型, 此方法不依赖于typeid关键字, 调用方法为lyf::getTypeName<decltype(param)>() 
	/// 此方法获取的名称更加简洁直观, 但会忽略掉默认的参数类型;
	/// 例如template<class T, class U = string> class A {};则对于auto a = A<Color>();此方法将返回A<Color>而不是A<Color, string>
	/// 若需要获取完整的类型, 可以使用getWholeTypeName(x)方法
	template<typename T>
	string getTypeName() {
		string s = __PRETTY_FUNCTION__;
		auto pos1 = s.find("T = ") + 4;	// +4是为了跳过"T = "这几个字符
		auto pos2 = s.find_first_of("	]", pos1);
		return s.substr(pos1, pos2 - pos1);
	}

	template<typename Helper>
	struct cvr_saver {};	// 用于保存变量的const/volatile/reference属性
	// 采用模板参数而不是函数参数的方式获取变量类型获取param的类型的调用格式为lyf::type_class<decltype(param)>::get()
	template<typename T>
	class type_class {
	public:
		// 获取变量类型(依赖于typeid关键字)
		static string get() {
			string all_realName = string{ abi::__cxa_demangle(typeid(cvr_saver<T>).name(), nullptr, nullptr, nullptr) };	// 包含cuv_saver结构体的全名
			auto pos1 = all_realName.find_first_of('<') + 1;	// 第一个'<'后的位置
			auto pos2 = all_realName.find_last_of('>');	// 最后一个'>'的位置
			return all_realName.substr(pos1, pos2 - pos1);	// 去掉干扰信息
		}
	};	// class type_class

	namespace StringTool {
		/// @brief 仿python的split函数, 分隔符为char
		/// @param str 要分隔的字符串
		/// @param delim 分隔符
		/// @return 分隔后的字符串数组, 以vector<string>形式返回
		inline vector<string> split(const string& str, const char delim) {
			std::stringstream ss(str);
			string s;
			vector<string> res;
			res.clear();
			while (getline(ss, s, delim)) {
				res.push_back(s);
			}
			return res;
		}

		/// @brief 仿python的split函数, 分隔符为string
		/// @param str 要分隔的字符串
		/// @param delim 分隔符
		/// @return 分隔后的字符串数组, 以vector<string>形式返回
		inline vector<string> split(const string& str, const string& delim) {
			size_t pos1 = 0;
			size_t pos2 = str.find_first_of(delim, pos1);	// 查找第一个分隔符的位置
			vector<string> res;
			while (string::npos != pos2) {
				res.push_back(str.substr(pos1, pos2 - pos1));
				pos1 = pos2 + delim.size();
				pos2 = str.find(delim, pos1);
			}
			if (pos1 != str.size())
				res.push_back(str.substr(pos1));
			return res;
		}

		/// @brief 以正则表达式匹配字符串
		/// @param str 要匹配的字符串
		/// @param pattern 要匹配的正则表达式
		/// @return 匹配后的字符串数组, 以vector<string>形式返回
		inline vector<string> regex_match(const string& str, const string& pattern) {
			using std::regex, std::smatch, std::sregex_iterator;
			regex m_pattern{ pattern };
			auto word_begin = sregex_iterator(str.begin(), str.end(), m_pattern);
			auto word_end = sregex_iterator();
			vector<string> res;
			for (auto i = word_begin; i != word_end; ++i) {
				smatch match = *i;
				res.emplace_back(match.str());
			}
			return res;
		}

		/// @brief 替换字符串中的第一个指定子串
		/// @param str 要替换的字符串
		/// @param old_value 要替换的子串
		/// @param new_value 替换后的子串
		/// @return 替换后的字符串
		inline string replace_first(const string& str, const string& old_value, const string& new_value) {
			string res = str;
			auto pos = res.find(old_value);
			if (pos != string::npos) {
				return res.replace(pos, old_value.length(), new_value);
			}
			else return str;
		}

		/// @brief 替换字符串中的所有指定子串
		/// @param str 要替换的字符串
		/// @param old_value 要替换的子串
		/// @param new_value 替换后的子串
		/// @return 替换后的字符串
		inline string replace_all(const string& str, const string& old_value, const string& new_value) {
			string res = str;
			for (size_t pos = 0; pos != string::npos; pos += new_value.length()) {
				pos = res.find(old_value, pos);
				if (pos != string::npos) {
					res.replace(pos, old_value.length(), new_value);
				}
				else break;
			}
			return res;
		}

		/// @brief 替换字符串中的最后一个指定子串
		/// @param str 要替换的字符串
		/// @param old_value 要替换的子串
		/// @param new_value 替换后的子串
		/// @return 替换后的字符串
		inline string replace_last(const string& str, const string& old_value, const string& new_value) {
			string res = str;
			auto pos = res.rfind(old_value);
			if (pos != string::npos) {
				return res.replace(pos, old_value.length(), new_value);
			}
			else return str;
		}

		/// @brief 判断字符串是否以指定前缀开头
		/// @param str 要判断的字符串
		/// @param prefix 前缀字符串
		/// @return 是否以指定前缀开头
		inline bool begin_with(const string& str, const string& prefix) {
			for (size_t i = 0; i < prefix.size(); ++i) {
				if (str[i] != prefix[i]) return false;
			}
			return true;
		}

		/// @brief 判断字符串是否以指定后缀结尾
		/// @param str 要判断的字符串
		/// @param suffix 后缀字符串
		/// @return 是否以指定后缀结尾
		inline bool end_with(const string& str, const string& suffix) {
			size_t str_len = str.size();
			size_t suffix_len = suffix.size();
			if (str_len < suffix_len) return false;
			for (size_t i = 0; i < suffix_len; ++i) {
				if (str[str_len - suffix_len + i] != suffix[i]) return false;
			}
			return true;
		}
	};	// namespace StringTool

	namespace PrintTool {
#if __cplusplus >= 201703L	// C++17以上才编译
		static string printDelim{ " " };
		/// @brief 用于print_args的分隔符, 默认为空格, 单次使用, 调用后会被重置, 可以通过设置delimIsPersist为true使其持久化
		inline void set_printDelim(std::string_view delim = " ") { printDelim = delim; }

		static bool delimIsPersist{ false };
		/// @brief 分隔符是否持久化, 默认不持久化
		inline void set_printDelimPersist(bool flag = false) { delimIsPersist = flag; }

		/// @brief 形参包遍历打印元素(C++17以后)
		template<typename T, typename ...Args>
		void print_args(T&& v, Args&&... args) {
			cout << v << printDelim;
			if constexpr (sizeof...(args) > 0) {
				print_args(std::forward<Args>(args)...);
			}
			else {
				cout << endl;
				if (!delimIsPersist)
					printDelim = " ";	// 恢复默认分隔符
			}
		}
#endif

		/// @brief 以迭代器方式遍历容器元素
		/// @param v 要遍历的容器
		/// @param delim 每个元素之间的分隔符
		/// @param os 输出流(默认为cout)
		template<typename T>
		void print_container(const T& v, const string& delim = " ", std::ostream& os = cout) {
			for (const auto& i : v) {
				os << i << delim;
			}
			os << endl;
		}

		/// @brief 范围内遍历打印容器元素
		/// @tparam Iter 迭代器类型
		/// @param first 开始迭代器(包含)
		/// @param last 结束迭代器(不包含)
		/// @param delim 每个元素之间的分隔符 
		/// @param os 输出流(默认为cout)
		template<typename Iter>
		void print_container(Iter first, Iter last, const string& delim = " ", std::ostream& os = cout) {
			using T = typename std::iterator_traits<Iter>::value_type;
			std::copy(first, last, std::ostream_iterator<T>(os, delim.c_str()));
			os << endl;
		}

	};	// namespace PrintTool

	template<class T1, class T2>
	auto max(T1&& a, T2&& b) {
		return a > b ? std::forward<T1>(a) : std::forward<T2>(b);
	}

	template<class T, class... Args>
	auto max(T&& a, Args&&... args) {
		return max(std::forward<T>(a), max(std::forward<Args>(args)...));
	}

	template<class T1, class T2>
	auto min(T1&& a, T2&& b) {
		return a < b ? std::forward<T1>(a) : std::forward<T2>(b);
	}

	template<class T, class... Args>
	auto min(T&& a, Args&&... args) {
		return min(std::forward<T>(a), min(std::forward<Args>(args)...));
	}

	/// @brief 基于迭代器的范围比较模板函数
	/// @param first 第一个迭代器(包含)
	/// @param last 最后一个迭代器(不包含)
	/// @param comp 比较函数
	/// @return 范围内按照比较函数比较后的最大值
	template<class Iterator, class Compare>
	auto range_compare(Iterator first, Iterator last, Compare comp = Compare()) -> typename std::iterator_traits<Iterator>::value_type {
		if (first == last) {
			throw std::invalid_argument("Range cannot be empty");
		}
		auto maxValue = *first;
		for (Iterator it = std::next(first); it != last; ++it) {
			if (comp(maxValue, *it)) {
				maxValue = *it;
			}
		}
		return maxValue;
	}

	/// @brief 基于迭代器的范围最大值模板函数
	/// @param first 第一个迭代器(包含)
	/// @param last 最后一个迭代器(不包含)
	/// @return 范围内的最大值
	template<class Iterator>
	auto range_max(Iterator first, Iterator last) {
		return range_compare(first, last, std::less<typename std::iterator_traits<Iterator>::value_type>());
	}

	/// @brief 基于迭代器的范围最小值模板函数
	/// @param first 第一个迭代器(包含)
	/// @param last 最后一个迭代器(不包含)
	/// @return 范围内的最小值
	template<class Iterator>
	auto range_min(Iterator first, Iterator last) {
		return range_compare(first, last, std::greater<typename std::iterator_traits<Iterator>::value_type>());
	}

    /// 1. 通用单例模式
    /// 推荐使用继承方式 class Foo : public Singleton<Foo>
    /// 此时如果有 auto f1 = Singleton<Foo>::GetInstance();   则编译器会报错阻止拷贝构造，保证单例对象的唯一性
    /// 
    /// 2. 对于不继承直接使用单例模式的方式Singleton<Foo2>::GetInstance().printAdress(); 这种方式没有阻止单例的拷贝或赋值, 不推荐使用, 会导致单例失效
    /// 例如: auto f2 = Singleton<Foo2>::GetInstance();
    /// 此时f2是一个新的实例，对Singleton<Foo2>::GetInstance()的单例进行了拷贝
	template <typename T>
	class Singleton {	// 泛型单例
	public:
		// 获取单例实例对象
		static T& GetInstance() {
			// 利用局部静态变量实现单例
			static T instance;
			return instance;
		}

		// 打印单例的地址
		void printAdress() {
			cout << this << endl;
		}

		// 禁止外部拷贝或赋值
		Singleton(const Singleton&) = delete;
		Singleton(Singleton&&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton& operator=(Singleton&&) = delete;

	protected:
		// 禁止外部构造和析构
		Singleton() = default;
		~Singleton() = default;
	};

	/// @brief 运行时断言函数, 若condition为false, 则抛出异常
	/// @param condition 断言条件 
	/// @param what 异常信息
	inline void assure(bool condition, std::string_view what = "Assertion failed!") {
		if (!condition) {
			throw std::runtime_error{ what.data() };
		}
    }

    /// @brief 随机数生成器
    /// @tparam T 数据类型
    /// @param begin 开始值(包含)
	/// @param end 结束值(不包含)
    template<class T>
    T getRandom(T begin, T end) {
        assert(begin <= end);
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_int_distribution<T> dis(begin, end);
		return dis(gen);
	}
}	// namespace lyf

#endif // LYF_H