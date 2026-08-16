//
//  peglib.h
//
//  Copyright (c) 2022 Yuji Hirose. All rights reserved.
//  MIT License
//

#pragma once

#define CPPPEGLIB_VERSION "1.16.0"
#define CPPPEGLIB_VERSION_NUM "0x011000"

/*
 * Configuration
 */

#ifndef CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT
#define CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT 32
#endif

#include <algorithm>
#include <any>
#include <bitset>
#include <cassert>
#include <cctype>
#if __has_include(<charconv>)
#include <charconv>
#endif
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if !defined(__cplusplus) || __cplusplus < 201703L
#error "Requires complete C++17 support"
#endif

namespace peg {

struct GrammarBlob;

/*-----------------------------------------------------------------------------
 *  scope_exit
 *---------------------------------------------------------------------------*/

// This is based on
// "http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4189".

template <typename EF> struct scope_exit {
  explicit scope_exit(EF &&f)
      : exit_function(std::move(f)), execute_on_destruction{true} {}

  scope_exit(scope_exit &&rhs)
      : exit_function(std::move(rhs.exit_function)),
        execute_on_destruction{rhs.execute_on_destruction} {
    rhs.release();
  }

  ~scope_exit() {
    if (execute_on_destruction) { this->exit_function(); }
  }

  void release() { this->execute_on_destruction = false; }

private:
  scope_exit(const scope_exit &) = delete;
  void operator=(const scope_exit &) = delete;
  scope_exit &operator=(scope_exit &&) = delete;

  EF exit_function;
  bool execute_on_destruction;
};

/*-----------------------------------------------------------------------------
 *  UTF8 functions
 *---------------------------------------------------------------------------*/

inline size_t codepoint_length(const char *s8, size_t l) {
  if (l) {
    auto b = static_cast<uint8_t>(s8[0]);
    if ((b & 0x80) == 0) {
      return 1;
    } else if ((b & 0xE0) == 0xC0 && l >= 2) {
      return 2;
    } else if ((b & 0xF0) == 0xE0 && l >= 3) {
      return 3;
    } else if ((b & 0xF8) == 0xF0 && l >= 4) {
      return 4;
    }
  }
  return 0;
}

inline size_t codepoint_count(const char *s8, size_t l) {
  size_t count = 0;
  for (size_t i = 0; i < l;) {
    auto len = codepoint_length(s8 + i, l - i);
    if (len == 0) {
      // Invalid UTF-8 byte, treat as single byte to avoid infinite loop
      len = 1;
    }
    i += len;
    count++;
  }
  return count;
}

inline size_t encode_codepoint(char32_t cp, char *buff) {
  if (cp < 0x0080) {
    buff[0] = static_cast<char>(cp & 0x7F);
    return 1;
  } else if (cp < 0x0800) {
    buff[0] = static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
    buff[1] = static_cast<char>(0x80 | (cp & 0x3F));
    return 2;
  } else if (cp < 0xD800) {
    buff[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (cp & 0x3F));
    return 3;
  } else if (cp < 0xE000) {
    // D800 - DFFF is invalid...
    return 0;
  } else if (cp < 0x10000) {
    buff[0] = static_cast<char>(0xE0 | ((cp >> 12) & 0xF));
    buff[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    buff[2] = static_cast<char>(0x80 | (cp & 0x3F));
    return 3;
  } else if (cp < 0x110000) {
    buff[0] = static_cast<char>(0xF0 | ((cp >> 18) & 0x7));
    buff[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
    buff[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    buff[3] = static_cast<char>(0x80 | (cp & 0x3F));
    return 4;
  }
  return 0;
}

inline std::string encode_codepoint(char32_t cp) {
  char buff[4];
  auto l = encode_codepoint(cp, buff);
  return std::string(buff, l);
}

inline bool decode_codepoint(const char *s8, size_t l, size_t &bytes,
                             char32_t &cp) {
  if (l) {
    auto b = static_cast<uint8_t>(s8[0]);
    if ((b & 0x80) == 0) {
      bytes = 1;
      cp = b;
      return true;
    } else if ((b & 0xE0) == 0xC0) {
      if (l >= 2) {
        bytes = 2;
        cp = ((static_cast<char32_t>(s8[0] & 0x1F)) << 6) |
             (static_cast<char32_t>(s8[1] & 0x3F));
        return true;
      }
    } else if ((b & 0xF0) == 0xE0) {
      if (l >= 3) {
        bytes = 3;
        cp = ((static_cast<char32_t>(s8[0] & 0x0F)) << 12) |
             ((static_cast<char32_t>(s8[1] & 0x3F)) << 6) |
             (static_cast<char32_t>(s8[2] & 0x3F));
        return true;
      }
    } else if ((b & 0xF8) == 0xF0) {
      if (l >= 4) {
        bytes = 4;
        cp = ((static_cast<char32_t>(s8[0] & 0x07)) << 18) |
             ((static_cast<char32_t>(s8[1] & 0x3F)) << 12) |
             ((static_cast<char32_t>(s8[2] & 0x3F)) << 6) |
             (static_cast<char32_t>(s8[3] & 0x3F));
        return true;
      }
    }
  }
  return false;
}

inline size_t decode_codepoint(const char *s8, size_t l, char32_t &cp) {
  size_t bytes;
  if (decode_codepoint(s8, l, bytes, cp)) { return bytes; }
  return 0;
}

inline char32_t decode_codepoint(const char *s8, size_t l) {
  char32_t cp = 0;
  decode_codepoint(s8, l, cp);
  return cp;
}

inline std::u32string decode(const char *s8, size_t l) {
  std::u32string out;
  size_t i = 0;
  while (i < l) {
    auto beg = i++;
    while (i < l && (s8[i] & 0xc0) == 0x80) {
      i++;
    }
    out += decode_codepoint(&s8[beg], (i - beg));
  }
  return out;
}

template <typename T> const char *u8(const T *s) {
  return reinterpret_cast<const char *>(s);
}

/*-----------------------------------------------------------------------------
 *  escape_characters
 *---------------------------------------------------------------------------*/

inline std::string escape_characters(const char *s, size_t n) {
  std::string str;
  for (size_t i = 0; i < n; i++) {
    auto c = s[i];
    switch (c) {
    case '\f': str += "\\f"; break;
    case '\n': str += "\\n"; break;
    case '\r': str += "\\r"; break;
    case '\t': str += "\\t"; break;
    case '\v': str += "\\v"; break;
    default: str += c; break;
    }
  }
  return str;
}

inline std::string escape_characters(std::string_view sv) {
  return escape_characters(sv.data(), sv.size());
}

/*-----------------------------------------------------------------------------
 *  resolve_escape_sequence
 *---------------------------------------------------------------------------*/

inline bool is_hex(char c, int &v) {
  if ('0' <= c && c <= '9') {
    v = c - '0';
    return true;
  } else if ('a' <= c && c <= 'f') {
    v = c - 'a' + 10;
    return true;
  } else if ('A' <= c && c <= 'F') {
    v = c - 'A' + 10;
    return true;
  }
  return false;
}

inline bool is_digit(char c, int &v) {
  if ('0' <= c && c <= '9') {
    v = c - '0';
    return true;
  }
  return false;
}

inline std::pair<int, size_t> parse_hex_number(const char *s, size_t n,
                                               size_t i) {
  int ret = 0;
  int val;
  while (i < n && is_hex(s[i], val)) {
    ret = static_cast<int>(ret * 16 + val);
    i++;
  }
  return std::pair(ret, i);
}

inline std::pair<int, size_t> parse_octal_number(const char *s, size_t n,
                                                 size_t i) {
  int ret = 0;
  int val;
  while (i < n && is_digit(s[i], val)) {
    ret = static_cast<int>(ret * 8 + val);
    i++;
  }
  return std::pair(ret, i);
}

inline std::string resolve_escape_sequence(const char *s, size_t n) {
  std::string r;
  r.reserve(n);

  size_t i = 0;
  while (i < n) {
    auto ch = s[i];
    if (ch == '\\') {
      i++;
      assert(i < n);

      switch (s[i]) {
      case 'f':
        r += '\f';
        i++;
        break;
      case 'n':
        r += '\n';
        i++;
        break;
      case 'r':
        r += '\r';
        i++;
        break;
      case 't':
        r += '\t';
        i++;
        break;
      case 'v':
        r += '\v';
        i++;
        break;
      case '\'':
        r += '\'';
        i++;
        break;
      case '"':
        r += '"';
        i++;
        break;
      case '[':
        r += '[';
        i++;
        break;
      case ']':
        r += ']';
        i++;
        break;
      case '^':
        r += '^';
        i++;
        break;
      case '-':
        r += '-';
        i++;
        break;
      case '\\':
        r += '\\';
        i++;
        break;
      case 'x':
      case 'u': {
        char32_t cp;
        std::tie(cp, i) = parse_hex_number(s, n, i + 1);
        r += encode_codepoint(cp);
        break;
      }
      default: {
        char32_t cp;
        std::tie(cp, i) = parse_octal_number(s, n, i);
        r += encode_codepoint(cp);
        break;
      }
      }
    } else {
      r += ch;
      i++;
    }
  }
  return r;
}

/*
 * Predefined character classes (ASCII semantics)
 */
inline const std::vector<std::pair<char32_t, char32_t>> *
predefined_character_class(std::string_view name) {
  static const std::map<std::string_view,
                        std::vector<std::pair<char32_t, char32_t>>>
      table = {
          {"alnum", {{'0', '9'}, {'A', 'Z'}, {'a', 'z'}}},
          {"alpha", {{'A', 'Z'}, {'a', 'z'}}},
          {"ascii", {{0x00, 0x7F}}},
          {"blank", {{'\t', '\t'}, {' ', ' '}}},
          {"cntrl", {{0x00, 0x1F}, {0x7F, 0x7F}}},
          {"digit", {{'0', '9'}}},
          {"graph", {{0x21, 0x7E}}},
          {"lower", {{'a', 'z'}}},
          {"print", {{0x20, 0x7E}}},
          {"punct", {{0x21, 0x2F}, {0x3A, 0x40}, {0x5B, 0x60}, {0x7B, 0x7E}}},
          {"space", {{'\t', '\r'}, {' ', ' '}}},
          {"upper", {{'A', 'Z'}}},
          {"word", {{'0', '9'}, {'A', 'Z'}, {'_', '_'}, {'a', 'z'}}},
          {"xdigit", {{'0', '9'}, {'A', 'F'}, {'a', 'f'}}},
      };
  auto it = table.find(name);
  return it != table.end() ? &it->second : nullptr;
}

// Ranges must be sorted and non-overlapping.
inline std::vector<std::pair<char32_t, char32_t>> complement_character_ranges(
    const std::vector<std::pair<char32_t, char32_t>> &ranges) {
  std::vector<std::pair<char32_t, char32_t>> r;
  char32_t next = 0;
  for (const auto &[lo, hi] : ranges) {
    if (lo > next) { r.emplace_back(next, lo - 1); }
    next = hi + 1;
  }
  if (next <= 0x10FFFF) { r.emplace_back(next, 0x10FFFF); }
  return r;
}

/*-----------------------------------------------------------------------------
 *  token_to_number_ - This function should be removed eventually
 *---------------------------------------------------------------------------*/

template <typename T> T token_to_number_(std::string_view sv) {
  T n = 0;
#if __has_include(<charconv>)
  if constexpr (!std::is_floating_point<T>::value) {
    std::from_chars(sv.data(), sv.data() + sv.size(), n);
#else
  if constexpr (false) {
#endif
  } else {
    auto s = std::string(sv);
    std::istringstream ss(s);
    ss >> n;
  }
  return n;
}

inline std::string to_lower(std::string s) {
  for (auto &c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return s;
}

/*-----------------------------------------------------------------------------
 *  Trie
 *---------------------------------------------------------------------------*/

class Trie {
public:
  Trie(const std::vector<std::string> &items, bool ignore_case)
      : ignore_case_(ignore_case), items_count_(items.size()) {
    size_t id = 0;
    for (const auto &item : items) {
      const auto &s = ignore_case ? to_lower(item) : item;
      if (item.size() > max_len_) { max_len_ = item.size(); }
      for (size_t len = 1; len <= item.size(); len++) {
        auto last = len == item.size();
        std::string_view sv(s.data(), len);
        auto it = dic_.find(sv);
        if (it == dic_.end()) {
          dic_.emplace(sv, Info{last, last, id});
        } else if (last) {
          it->second.match = true;
        } else {
          it->second.done = false;
        }
      }
      id++;
    }
  }

  size_t match(const char *text, size_t text_len, size_t &id) const {
    auto limit = std::min(text_len, max_len_);
    std::string lower_text;
    if (ignore_case_) {
      lower_text = to_lower(std::string(text, limit));
      text = lower_text.data();
    }

    size_t match_len = 0;
    auto done = false;
    size_t len = 1;
    while (!done && len <= limit) {
      std::string_view sv(text, len);
      auto it = dic_.find(sv);
      if (it == dic_.end()) {
        done = true;
      } else {
        if (it->second.match) {
          match_len = len;
          id = it->second.id;
        }
        if (it->second.done) { done = true; }
      }
      len += 1;
    }
    return match_len;
  }

  size_t size() const { return dic_.size(); }
  size_t items_count() const { return items_count_; }

  friend struct ComputeFirstSet;
  friend struct GrammarBlob;

private:
  struct Info {
    bool done;
    bool match;
    size_t id;
  };

  // TODO: Use unordered_map when heterogeneous lookup is supported in C++20
  // std::unordered_map<std::string, Info> dic_;
  std::map<std::string, Info, std::less<>> dic_;

  bool ignore_case_;
  size_t items_count_;
  size_t max_len_ = 0;
};

/*-----------------------------------------------------------------------------
 *  PEG
 *---------------------------------------------------------------------------*/

/*
 * Line information utility function
 */
inline std::pair<size_t, size_t> line_info(const char *start, const char *cur) {
  auto p = start;
  auto col_ptr = p;
  auto no = 1;

  while (p < cur) {
    if (*p == '\n') {
      no++;
      col_ptr = p + 1;
    }
    p++;
  }

  auto col = codepoint_count(col_ptr, p - col_ptr) + 1;

  return std::pair(no, col);
}

/*
 * String tag
 */
inline constexpr unsigned int str2tag_core(const char *s, size_t l,
                                           unsigned int h) {
  return (l == 0) ? h
                  : str2tag_core(s + 1, l - 1,
                                 (h * 33) ^ static_cast<unsigned char>(*s));
}

inline constexpr unsigned int str2tag(std::string_view sv) {
  return str2tag_core(sv.data(), sv.size(), 0);
}

namespace udl {

inline constexpr unsigned int operator""_(const char *s, size_t l) {
  return str2tag_core(s, l, 0);
}

} // namespace udl

/*
 * Semantic values
 */
class Context;

struct SemanticValues : protected std::vector<std::any> {
  SemanticValues() = default;
  SemanticValues(Context *c) : c_(c) {}

  // Input text
  const char *path = nullptr;
  const char *ss = nullptr;

  // Matched string
  std::string_view sv() const { return sv_; }

  // Definition name
  const std::string &name() const { return name_; }

  std::vector<unsigned int> tags;

  // Line number and column at which the matched string is
  std::pair<size_t, size_t> line_info() const;

  // Choice count
  size_t choice_count() const { return choice_count_; }

  // Choice number (0 based index)
  size_t choice() const { return choice_; }

  // Tokens
  std::vector<std::string_view> tokens;

  std::string_view token(size_t id = 0) const {
    if (tokens.empty()) { return sv_; }
    assert(id < tokens.size());
    return tokens[id];
  }

  // Token conversion
  std::string token_to_string(size_t id = 0) const {
    return std::string(token(id));
  }

  template <typename T> T token_to_number() const {
    return token_to_number_<T>(token());
  }

  // Transform the semantic value vector to another vector
  template <typename T>
  std::vector<T> transform(size_t beg = 0,
                           size_t end = static_cast<size_t>(-1)) const {
    std::vector<T> r;
    end = (std::min)(end, size());
    for (size_t i = beg; i < end; i++) {
      r.emplace_back(std::any_cast<T>((*this)[i]));
    }
    return r;
  }

  using std::vector<std::any>::iterator;
  using std::vector<std::any>::const_iterator;
  using std::vector<std::any>::size;
  using std::vector<std::any>::empty;
  using std::vector<std::any>::assign;
  using std::vector<std::any>::begin;
  using std::vector<std::any>::end;
  using std::vector<std::any>::rbegin;
  using std::vector<std::any>::rend;
  using std::vector<std::any>::operator[];
  using std::vector<std::any>::at;
  using std::vector<std::any>::resize;
  using std::vector<std::any>::front;
  using std::vector<std::any>::back;
  using std::vector<std::any>::push_back;
  using std::vector<std::any>::pop_back;
  using std::vector<std::any>::insert;
  using std::vector<std::any>::erase;
  using std::vector<std::any>::clear;
  using std::vector<std::any>::swap;
  using std::vector<std::any>::emplace;
  using std::vector<std::any>::emplace_back;

private:
  friend class Context;
  friend class Dictionary;
  friend class Sequence;
  friend class PrioritizedChoice;
  friend class Repetition;
  friend class Holder;
  friend class PrecedenceClimbing;

  Context *c_ = nullptr;
  std::string_view sv_;
  size_t choice_count_ = 0;
  size_t choice_ = 0;
  std::string name_;
};

/*
 * Semantic action
 */
template <typename F, typename... Args> std::any call(F fn, Args &&...args) {
  using R = decltype(fn(std::forward<Args>(args)...));
  if constexpr (std::is_void<R>::value) {
    fn(std::forward<Args>(args)...);
    return std::any();
  } else if constexpr (std::is_same<typename std::remove_cv<R>::type,
                                    std::any>::value) {
    return fn(std::forward<Args>(args)...);
  } else {
    return std::any(fn(std::forward<Args>(args)...));
  }
}

template <typename T>
struct argument_count : argument_count<decltype(&T::operator())> {};
template <typename R, typename... Args>
struct argument_count<R (*)(Args...)>
    : std::integral_constant<unsigned, sizeof...(Args)> {};
template <typename R, typename C, typename... Args>
struct argument_count<R (C::*)(Args...)>
    : std::integral_constant<unsigned, sizeof...(Args)> {};
template <typename R, typename C, typename... Args>
struct argument_count<R (C::*)(Args...) const>
    : std::integral_constant<unsigned, sizeof...(Args)> {};

class Action {
public:
  Action() = default;
  Action(Action &&rhs) = default;
  template <typename F> Action(F fn) : fn_(make_adaptor(fn)) {}
  template <typename F> void operator=(F fn) { fn_ = make_adaptor(fn); }
  Action &operator=(const Action &rhs) = default;

  operator bool() const { return bool(fn_); }

  std::any operator()(SemanticValues &vs, std::any &dt,
                      const std::any &predicate_data) const {
    return fn_(vs, dt, predicate_data);
  }

private:
  using Fty = std::function<std::any(SemanticValues &vs, std::any &dt,
                                     const std::any &predicate_data)>;

  template <typename F> Fty make_adaptor(F fn) {
    if constexpr (argument_count<F>::value == 1) {
      return [fn](auto &vs, auto & /*dt*/, const auto & /*predicate_data*/) {
        return call(fn, vs);
      };
    } else if constexpr (argument_count<F>::value == 2) {
      return [fn](auto &vs, auto &dt, const auto & /*predicate_data*/) {
        return call(fn, vs, dt);
      };
    } else {
      return [fn](auto &vs, auto &dt, const auto &predicate_data) {
        return call(fn, vs, dt, predicate_data);
      };
    }
  }

  Fty fn_;
};

class Predicate {
public:
  Predicate() = default;
  Predicate(Predicate &&rhs) = default;
  template <typename F> Predicate(F fn) : fn_(make_adaptor(fn)) {}
  template <typename F> void operator=(F fn) { fn_ = make_adaptor(fn); }
  Predicate &operator=(const Predicate &rhs) = default;

  operator bool() const { return bool(fn_); }

  bool operator()(const SemanticValues &vs, const std::any &dt,
                  std::string &msg, std::any &predicate_data) const {
    return fn_(vs, dt, msg, predicate_data);
  }

private:
  using Fty = std::function<bool(const SemanticValues &vs, const std::any &dt,
                                 std::string &msg, std::any &predicate_data)>;

  template <typename F> Fty make_adaptor(F fn) {
    if constexpr (argument_count<F>::value == 3) {
      return [fn](const auto &vs, const auto &dt, auto &msg,
                  auto & /*predicate_data*/) { return fn(vs, dt, msg); };
    } else {
      return [fn](const auto &vs, const auto &dt, auto &msg,
                  auto &predicate_data) {
        return fn(vs, dt, msg, predicate_data);
      };
    }
  }

  Fty fn_;
};

/*
 * Parse result helper
 */
inline bool success(size_t len) { return len != static_cast<size_t>(-1); }

inline bool fail(size_t len) { return len == static_cast<size_t>(-1); }

/*
 * Log
 */
using Log = std::function<void(size_t line, size_t col, const std::string &msg,
                               const std::string &rule)>;

/*
 * ErrorReport - structured error information passed to an ErrorReporter.
 * Unlike Log, nothing is flattened into a display string, so applications
 * can map errors to their own error types, localize messages, or feed
 * diagnostics to IDEs.
 */
struct ErrorReport {
  size_t line = 0;              // 1-based
  size_t col = 1;               // 1-based
  size_t position = 0;          // byte offset in the input
  std::string unexpected_token; // heuristic token at the error position
  std::vector<std::string> expected_literals;
  std::vector<std::string> expected_rules; // rules starting with '_' excluded
  std::string message; // custom error_message if any (placeholders resolved)
  std::string label;   // rule name or recovery label the error belongs to
};

using ErrorReporter = std::function<void(const ErrorReport &report)>;

/*
 * ErrorInfo
 */
class Definition;

struct ErrorInfo {
  const char *error_pos = nullptr;
  std::vector<std::pair<const char *, const Definition *>> expected_tokens;
  const char *message_pos = nullptr;
  std::string message;
  std::string label;
  const char *last_output_pos = nullptr;
  bool keep_previous_token = false;

  void clear() {
    error_pos = nullptr;
    expected_tokens.clear();
    message_pos = nullptr;
    message.clear();
  }

  void add(const char *error_literal, const Definition *error_rule) {
    for (const auto &[t, r] : expected_tokens) {
      if (t == error_literal && r == error_rule) { return; }
    }
    expected_tokens.emplace_back(error_literal, error_rule);
  }

  void output_log(const Log &log, const char *s, size_t n) {
    output_log(log, nullptr, s, n);
  }
  void output_log(const Log &log, const ErrorReporter &reporter, const char *s,
                  size_t n);

private:
  int cast_char(char c) const { return static_cast<unsigned char>(c); }

  std::string heuristic_error_token(const char *s, size_t n,
                                    const char *pos) const {
    auto len = n - std::distance(s, pos);
    if (len) {
      size_t i = 0;
      auto c = cast_char(pos[i++]);
      if (!std::ispunct(c) && !std::isspace(c)) {
        while (i < len && !std::ispunct(cast_char(pos[i])) &&
               !std::isspace(cast_char(pos[i]))) {
          i++;
        }
      }

      size_t count = CPPPEGLIB_HEURISTIC_ERROR_TOKEN_MAX_CHAR_COUNT;
      size_t j = 0;
      while (count > 0 && j < i) {
        j += codepoint_length(&pos[j], i - j);
        count--;
      }

      return escape_characters(pos, j);
    }
    return std::string();
  }

  std::string replace_all(std::string str, const std::string &from,
                          const std::string &to) const {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
      str.replace(pos, from.length(), to);
      pos += to.length();
    }
    return str;
  }
};

/*
 * Context
 */
class Ope;

using TracerEnter = std::function<void(
    const Ope &name, const char *s, size_t n, const SemanticValues &vs,
    const Context &c, const std::any &dt, std::any &trace_data)>;

using TracerLeave = std::function<void(
    const Ope &ope, const char *s, size_t n, const SemanticValues &vs,
    const Context &c, const std::any &dt, size_t, std::any &trace_data)>;

using TracerStartOrEnd = std::function<void(std::any &trace_data)>;

// Packrat memoization table: open-addressing hash map keyed by the fused
// (position * rule count + rule id) index. The insert-heavy access pattern
// makes node-based containers a bottleneck, so keys and lengths live in one
// flat array of 16-byte POD slots probed linearly; semantic values go into a
// parallel array that is never allocated when no cached result carries a
// value. Erased slots become tombstones (erase only happens during
// left-recursion cache invalidation).
class PackratCache {
public:
  explicit PackratCache(size_t expected_entries) {
    while (initial_capacity_ < expected_entries) {
      initial_capacity_ *= 2;
    }
  }

  bool find(size_t key, size_t &len, std::any &val) const {
    if (slots_.empty()) { return false; }
    auto mask = slots_.size() - 1;
    auto i = mix(key) & mask;
    while (true) {
      auto &slot = slots_[i];
      if (slot.key == key) {
        len = slot.len;
        if (!vals_.empty()) {
          val = vals_[i];
        } else {
          val.reset();
        }
        return true;
      }
      if (slot.key == kEmpty) { return false; }
      i = (i + 1) & mask;
    }
  }

  void insert_or_assign(size_t key, size_t len, const std::any &val) {
    if (slots_.empty() || (used_ + 1) * 4 > slots_.size() * 3) { grow(); }
    auto mask = slots_.size() - 1;
    auto i = mix(key) & mask;
    auto insert_pos = kEmpty;
    while (true) {
      auto &slot = slots_[i];
      if (slot.key == key) {
        insert_pos = i;
        break;
      }
      if (slot.key == kTombstone) {
        if (insert_pos == kEmpty) { insert_pos = i; }
      } else if (slot.key == kEmpty) {
        if (insert_pos == kEmpty) { insert_pos = i; }
        if (slots_[insert_pos].key == kEmpty) { used_++; }
        break;
      }
      i = (i + 1) & mask;
    }
    auto &dest = slots_[insert_pos];
    dest.key = key;
    dest.len = len;
    if (val.has_value()) {
      if (vals_.empty()) { vals_.resize(slots_.size()); }
      vals_[insert_pos] = val;
    } else if (!vals_.empty()) {
      vals_[insert_pos].reset();
    }
  }

  void erase(size_t key) {
    if (slots_.empty()) { return; }
    auto mask = slots_.size() - 1;
    auto i = mix(key) & mask;
    while (true) {
      auto &slot = slots_[i];
      if (slot.key == key) {
        slot.key = kTombstone;
        if (!vals_.empty()) { vals_[i].reset(); }
        return;
      }
      if (slot.key == kEmpty) { return; }
      i = (i + 1) & mask;
    }
  }

private:
  static constexpr size_t kEmpty = static_cast<size_t>(-1);
  static constexpr size_t kTombstone = static_cast<size_t>(-2);

  struct Slot {
    size_t key = kEmpty;
    size_t len = 0;
  };

  static size_t mix(size_t key) {
    // Mix in 64 bits so `h >> 32` stays well-defined where size_t is 32-bit
    // (wasm32); on 64-bit targets this is bit-identical to the size_t mix.
    auto h = static_cast<uint64_t>(key) * 0x9E3779B97F4A7C15ull;
    return static_cast<size_t>(h ^ (h >> 32));
  }

  void grow() {
    auto new_cap = slots_.empty() ? initial_capacity_ : slots_.size() * 2;
    std::vector<Slot> old_slots = std::move(slots_);
    std::vector<std::any> old_vals = std::move(vals_);
    slots_.assign(new_cap, Slot{});
    if (!old_vals.empty()) { vals_.assign(new_cap, std::any()); }
    used_ = 0;
    auto mask = new_cap - 1;
    for (size_t j = 0; j < old_slots.size(); j++) {
      auto &slot = old_slots[j];
      if (slot.key == kEmpty || slot.key == kTombstone) { continue; }
      auto i = mix(slot.key) & mask;
      while (slots_[i].key != kEmpty) {
        i = (i + 1) & mask;
      }
      slots_[i] = slot;
      if (!old_vals.empty()) { vals_[i] = std::move(old_vals[j]); }
      used_++;
    }
  }

  size_t initial_capacity_ = 1024;
  std::vector<Slot> slots_;
  std::vector<std::any> vals_;
  size_t used_ = 0; // occupied + tombstone slots
};

class Context {
public:
  const char *path;
  const char *s;
  const size_t l;

  ErrorInfo error_info;
  bool recovered = false;

  std::vector<std::unique_ptr<SemanticValues>> value_stack;
  size_t value_stack_size = 0;

  std::vector<Definition *> rule_stack;

  // One frame per rule reference: the macro arguments in scope, and the
  // instantiation they identify (0 for anything but a left-recursive macro).
  struct ArgsFrame {
    std::vector<std::shared_ptr<Ope>> args;
    size_t macro_inst = 0;
  };
  std::vector<ArgsFrame> args_stack;

  size_t in_token_boundary_count = 0;

  std::shared_ptr<Ope> whitespaceOpe;
  bool in_whitespace = false;

  std::shared_ptr<Ope> wordOpe;

  std::vector<std::pair<std::string_view, std::string>> capture_entries;

  std::vector<bool> cut_stack;

  const size_t def_count;
  const bool enablePackratParsing;
  const std::vector<int32_t> *packrat_index; // def_id -> cache slot or -1
  size_t packrat_cached_count;               // number of memoized rules
  std::vector<bool> cache_registered;
  std::vector<bool> cache_success;
  // Innermost active start position per rule; re-entry guard for rules that
  // are not memoized (replaces the per-position bitvector for them).
  std::vector<const char *> active_pos;

  PackratCache cache_values;

  // Left recursion support
  struct LRMemo {
    size_t len = static_cast<size_t>(-1);
    std::any val;
  };

  // A left-recursive rule instance: the definition plus, for a macro, the
  // instantiation it was invoked with (0 for a plain rule). Two
  // instantiations of the same macro grow independent seeds.
  using LRRule = std::pair<const Definition *, size_t>;
  using LRKey = std::pair<LRRule, const char *>;

  std::map<LRKey, LRMemo> lr_memo;

  // Rules whose lr_memo was hit during the current parse scope.
  // Used to track LR cycle membership.
  std::set<LRRule> lr_refs_hit;

  // Rules currently in their seeding/growing phase at a given position.
  // Protected from having their lr_memo erased by inner growers.
  std::set<LRKey> lr_active_seeds;

  // Interned macro instantiations: (definition, resolved arguments) -> id.
  std::map<std::vector<const void *>, size_t> macro_inst_ids;
  size_t next_macro_inst_ = 1;

  // Map a def_id to its slot in the cache tables, or -1 for guard-only
  // rules (not memoized).
  int32_t cache_slot(size_t def_id) const {
    if (!packrat_index) { return static_cast<int32_t>(def_id); }
    return def_id < packrat_index->size() ? (*packrat_index)[def_id] : -1;
  }

  void clear_packrat_cache(const char *pos, size_t def_id) {
    if (!enablePackratParsing) { return; }
    auto slot = cache_slot(def_id);
    if (slot < 0) { return; }
    auto col = static_cast<size_t>(pos - s);
    auto idx = packrat_cached_count * col + static_cast<size_t>(slot);
    if (idx < cache_registered.size()) {
      cache_registered[idx] = false;
      cache_success[idx] = false;
    }
    cache_values.erase(idx);
  }

  void write_packrat_cache(const char *pos, size_t def_id, size_t len,
                           const std::any &val) {
    if (!enablePackratParsing) { return; }
    auto slot = cache_slot(def_id);
    if (slot < 0) { return; }
    auto col = pos - s;
    auto idx = packrat_cached_count * static_cast<size_t>(col) +
               static_cast<size_t>(slot);
    if (idx >= cache_registered.size()) { return; }
    cache_registered[idx] = true;
    cache_success[idx] = true;
    cache_values.insert_or_assign(idx, len, val);
  }

  TracerEnter tracer_enter;
  TracerLeave tracer_leave;
  const bool has_tracer;
  std::any trace_data;
  const bool verbose_trace;

  // Byte-wise tolower frozen at parse start, so case-insensitive matching
  // avoids a locale-sensitive libc call per input byte.
  unsigned char tolower_table[256];

  Log log;
  ErrorReporter error_reporter;

  Context(const char *path, const char *s, size_t l, size_t def_count,
          std::shared_ptr<Ope> whitespaceOpe, std::shared_ptr<Ope> wordOpe,
          bool enablePackratParsing, TracerEnter tracer_enter,
          TracerLeave tracer_leave, std::any trace_data, bool verbose_trace,
          Log log, ErrorReporter error_reporter = nullptr,
          const std::vector<int32_t> *packrat_index = nullptr,
          size_t packrat_cached_count = 0)
      : path(path), s(s), l(l), whitespaceOpe(whitespaceOpe), wordOpe(wordOpe),
        def_count(def_count), enablePackratParsing(enablePackratParsing),
        packrat_index(packrat_index),
        packrat_cached_count(packrat_index ? packrat_cached_count : def_count),
        cache_registered(
            enablePackratParsing ? this->packrat_cached_count * (l + 1) : 0),
        cache_success(
            enablePackratParsing ? this->packrat_cached_count * (l + 1) : 0),
        active_pos(enablePackratParsing ? def_count : 0, nullptr),
        cache_values(enablePackratParsing ? (packrat_index ? l / 8 + 16 : l / 2)
                                          : 0),
        tracer_enter(tracer_enter), tracer_leave(tracer_leave),
        has_tracer(tracer_enter && tracer_leave), trace_data(trace_data),
        verbose_trace(verbose_trace), log(log), error_reporter(error_reporter) {

    for (size_t i = 0; i < 256; i++) {
      tolower_table[i] =
          static_cast<unsigned char>(std::tolower(static_cast<int>(i)));
    }

    push_args({});
  }

  ~Context() {
    assert(!value_stack_size);
    assert(cut_stack.empty());
  }

  Context(const Context &) = delete;
  Context(Context &&) = delete;
  Context operator=(const Context &) = delete;

  // Per-rule packrat stats (populated when packrat_stats is non-null)
  struct PackratStats {
    size_t hits = 0;
    size_t misses = 0;
  };
  std::vector<PackratStats> *packrat_stats = nullptr;

  template <typename T>
  void packrat(const char *a_s, size_t def_id, size_t &len, std::any &val,
               T fn) {
    if (!enablePackratParsing) {
      fn(val);
      return;
    }

    auto slot = cache_slot(def_id);
    if (slot < 0) {
      // Guard-only rule: no memoization. Recursion at the same position is
      // caught by the per-rule active-position guard.
      if (active_pos[def_id] == a_s) {
        if (packrat_stats && def_id < packrat_stats->size()) {
          (*packrat_stats)[def_id].hits++;
        }
        len = static_cast<size_t>(-1);
        return;
      }
      if (packrat_stats && def_id < packrat_stats->size()) {
        (*packrat_stats)[def_id].misses++;
      }
      auto save = active_pos[def_id];
      active_pos[def_id] = a_s;
      fn(val);
      active_pos[def_id] = save;
      return;
    }

    auto col = a_s - s;
    auto idx = packrat_cached_count * static_cast<size_t>(col) +
               static_cast<size_t>(slot);

    if (cache_registered[idx]) {
      if (packrat_stats && def_id < packrat_stats->size()) {
        (*packrat_stats)[def_id].hits++;
      }
      if (cache_success[idx]) {
        if (!cache_values.find(idx, len, val)) {
          len = 0;
          val.reset();
        }
        return;
      } else {
        len = static_cast<size_t>(-1);
        return;
      }
    } else {
      // Pre-register as failure (re-entry guard + failure memoization)
      cache_registered[idx] = true;
      cache_success[idx] = false;

      if (packrat_stats && def_id < packrat_stats->size()) {
        (*packrat_stats)[def_id].misses++;
      }

      fn(val);

      if (success(len)) { write_packrat_cache(a_s, def_id, len, val); }
      return;
    }
  }

  // Semantic values
  SemanticValues &push_semantic_values_scope() {
    assert(value_stack_size <= value_stack.size());
    if (value_stack_size == value_stack.size()) {
      value_stack.emplace_back(std::make_unique<SemanticValues>(this));
    } else {
      auto &vs = *value_stack[value_stack_size];
      if (!vs.empty()) {
        vs.clear();
        if (!vs.tags.empty()) { vs.tags.clear(); }
      }
      vs.sv_ = std::string_view();
      vs.choice_count_ = 0;
      vs.choice_ = 0;
      if (!vs.tokens.empty()) { vs.tokens.clear(); }
    }

    auto &vs = *value_stack[value_stack_size++];
    vs.path = path;
    vs.ss = s;
    return vs;
  }

  void pop_semantic_values_scope() { value_stack_size--; }

  // Arguments
  void push_args(std::vector<std::shared_ptr<Ope>> &&args,
                 size_t macro_inst = 0) {
    args_stack.push_back({std::move(args), macro_inst});
  }

  void pop_args() { args_stack.pop_back(); }

  const std::vector<std::shared_ptr<Ope>> &top_args() const {
    return args_stack[args_stack.size() - 1].args;
  }

  size_t top_macro_inst() const {
    return args_stack[args_stack.size() - 1].macro_inst;
  }

  // Identify a macro invocation by what its resolved arguments denote (see
  // macro_inst_key). `Sum(A)` inside `Sum(N)`'s own body resolves A back to
  // the argument the outer call was given, so both invocations intern to the
  // same id and the inner one finds the outer's seed — which is what makes
  // growing terminate.
  size_t intern_macro_inst(std::vector<const void *> &&key) {
    auto [it, inserted] =
        macro_inst_ids.emplace(std::move(key), next_macro_inst_);
    if (inserted) { next_macro_inst_++; }
    return it->second;
  }

  // Snapshot/Rollback
  struct Snapshot {
    size_t sv_size;
    size_t sv_tags_size;
    size_t sv_tokens_size;
    std::string_view sv_sv;
    size_t choice_count;
    size_t choice;
    size_t capture_size;
  };

  Snapshot snapshot(const SemanticValues &vs) const {
    return {vs.size(),        vs.tags.size(), vs.tokens.size(),      vs.sv_,
            vs.choice_count_, vs.choice_,     capture_entries.size()};
  }

  void rollback(SemanticValues &vs, const Snapshot &snap) {
    vs.resize(snap.sv_size);
    vs.tags.resize(snap.sv_tags_size);
    vs.tokens.resize(snap.sv_tokens_size);
    vs.sv_ = snap.sv_sv;
    vs.choice_count_ = snap.choice_count;
    vs.choice_ = snap.choice;
    capture_entries.resize(snap.capture_size);
  }

  // Skip trailing whitespace with trace suppression.
  // Returns whitespace length, or -1 on failure.
  // No-op (returns 0) if inside a token boundary or no whitespaceOpe.
  size_t skip_whitespace(const char *a_s, size_t n, SemanticValues &vs,
                         std::any &dt);

  // Error
  void set_error_pos(const char *a_s, const char *literal = nullptr);

  // Trace
  void trace_enter(const Ope &ope, const char *a_s, size_t n,
                   const SemanticValues &vs, std::any &dt);
  void trace_leave(const Ope &ope, const char *a_s, size_t n,
                   const SemanticValues &vs, std::any &dt, size_t len);
  bool is_traceable(const Ope &ope) const;

  // Line info
  std::pair<size_t, size_t> line_info(const char *cur) const {
    std::call_once(source_line_index_init_, [this]() {
      for (size_t pos = 0; pos < l; pos++) {
        if (s[pos] == '\n') { source_line_index.push_back(pos); }
      }
      source_line_index.push_back(l);
    });

    auto pos = static_cast<size_t>(std::distance(s, cur));

    auto it = std::lower_bound(
        source_line_index.begin(), source_line_index.end(), pos,
        [](size_t element, size_t value) { return element < value; });

    auto id = static_cast<size_t>(std::distance(source_line_index.begin(), it));
    auto off = pos - (id == 0 ? 0 : source_line_index[id - 1] + 1);
    return std::pair(id + 1, off + 1);
  }

  size_t next_trace_id = 0;
  std::vector<size_t> trace_ids;
  bool ignore_trace_state = false;
  mutable std::once_flag source_line_index_init_;
  mutable std::vector<size_t> source_line_index;
};

/*
 * Parser operators
 */
class Ope {
public:
  struct Visitor;

  virtual ~Ope() = default;
  size_t parse(const char *s, size_t n, SemanticValues &vs, Context &c,
               std::any &dt) const;
  virtual size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                            Context &c, std::any &dt) const = 0;
  virtual void accept(Visitor &v) = 0;

  bool is_token_boundary = false;
  bool is_choice_like = false;
};

// Keyword-guarded identifier data, heap-allocated only for matching Sequences.
// Avoids bloating all Sequence objects with bitsets and keyword sets.
struct KeywordGuardData {
  std::bitset<256> identifier_first;        // first char of identifier
  std::bitset<256> identifier_rest;         // subsequent chars of identifier
  std::vector<std::string> exact_keywords;  // single-word keywords (lowercase)
  std::vector<std::string> prefix_keywords; // first word of compound keywords
  size_t min_keyword_len = 0;
  size_t max_keyword_len = 0;

  static bool matches_any(const std::vector<std::string> &keywords,
                          std::string_view input) {
    return std::any_of(keywords.begin(), keywords.end(),
                       [&](const auto &kw) { return kw == input; });
  }
};

class Sequence : public Ope {
public:
  template <typename... Args>
  Sequence(const Args &...args)
      : opes_{static_cast<std::shared_ptr<Ope>>(args)...} {}
  Sequence(const std::vector<std::shared_ptr<Ope>> &opes) : opes_(opes) {}
  Sequence(std::vector<std::shared_ptr<Ope>> &&opes) : opes_(std::move(opes)) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    // Keyword-guarded identifier fast path:
    // Fuses !ReservedKeyword <identifier> into scan-then-lookup
    if (kw_guard_) {
      if (auto result = parse_keyword_guarded(s, n, vs, c, dt)) {
        return *result;
      }
      // nullopt means prefix keyword match — fall through to normal path
    }
    size_t i = 0;
    for (const auto &ope : opes_) {
      auto len = ope->parse(s + i, n - i, vs, c, dt);
      if (fail(len)) { return len; }
      i += len;
    }
    return i;
  }

  void accept(Visitor &v) override;

  std::vector<std::shared_ptr<Ope>> opes_;

private:
  friend struct SetupFirstSets;
  std::unique_ptr<KeywordGuardData> kw_guard_;

  // Returns parse result, or nullopt to fall through to normal path
  std::optional<size_t> parse_keyword_guarded(const char *s, size_t n,
                                              SemanticValues &vs, Context &c,
                                              std::any &dt) const {
    const auto &kw = *kw_guard_;
    if (n < 1 || !kw.identifier_first.test(static_cast<unsigned char>(*s))) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
    // Scan identifier using bitset
    size_t id_len = 1;
    while (id_len < n &&
           kw.identifier_rest.test(static_cast<unsigned char>(s[id_len]))) {
      id_len++;
    }
    // Skip keyword matching if identifier length is out of range
    if (id_len >= kw.min_keyword_len && id_len <= kw.max_keyword_len) {
      char lower_buf[64];
      std::unique_ptr<char[]> lower_heap;
      char *lower = lower_buf;
      if (id_len > sizeof(lower_buf)) {
        lower_heap.reset(new char[id_len]);
        lower = lower_heap.get();
      }
      std::transform(s, s + id_len, lower, [&c](unsigned char ch) {
        return static_cast<char>(c.tolower_table[ch]);
      });
      std::string_view lower_sv(lower, id_len);

      if (KeywordGuardData::matches_any(kw.exact_keywords, lower_sv)) {
        c.set_error_pos(s);
        return static_cast<size_t>(-1);
      }
      if (KeywordGuardData::matches_any(kw.prefix_keywords, lower_sv)) {
        return std::nullopt;
      }
    }
    // Success: emit token and consume trailing whitespace
    vs.tokens.emplace_back(std::string_view(s, id_len));
    auto wl = c.skip_whitespace(s + id_len, n - id_len, vs, dt);
    if (fail(wl)) { return wl; }
    return id_len + wl;
  }
};

struct FirstSet {
  // First-Set: set of possible first bytes for an expression.
  // Used by PrioritizedChoice to skip alternatives that cannot match.
  std::bitset<256> chars;    // byte values that can appear as the first byte
  bool can_be_empty = false; // true if the expression can match empty string
  bool any_char = false;     // true if any character can appear (cannot filter)
  const char *first_literal = nullptr; // first literal for error reporting
  const Definition *first_rule =
      nullptr; // first token rule for error reporting

  void merge(const FirstSet &other) {
    chars |= other.chars;
    if (other.can_be_empty) { can_be_empty = true; }
    if (other.any_char) { any_char = true; }
    // Note: first_literal/first_rule are NOT merged — per-alternative
  }
};

class PrioritizedChoice : public Ope {
public:
  template <typename... Args>
  PrioritizedChoice(bool for_label, const Args &...args)
      : opes_{static_cast<std::shared_ptr<Ope>>(args)...},
        for_label_(for_label) {
    is_choice_like = true;
  }
  PrioritizedChoice(const std::vector<std::shared_ptr<Ope>> &opes)
      : opes_(opes) {
    is_choice_like = true;
  }
  PrioritizedChoice(std::vector<std::shared_ptr<Ope>> &&opes)
      : opes_(std::move(opes)) {
    is_choice_like = true;
  }

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    size_t len = static_cast<size_t>(-1);

    if (!for_label_) { c.cut_stack.push_back(false); }
    auto se = scope_exit([&]() {
      if (!for_label_) { c.cut_stack.pop_back(); }
    });

    size_t id = 0;
    for (const auto &ope : opes_) {
      // First-Set filtering: skip if next byte cannot start this alternative
      if (n > 0 && id < first_sets_.size()) {
        const auto &fs = first_sets_[id];
        if (!fs.any_char && !fs.can_be_empty &&
            !fs.chars.test(static_cast<unsigned char>(*s))) {
          if ((c.log || c.error_reporter) &&
              (fs.first_literal || fs.first_rule)) {
            if (c.error_info.error_pos <= s) {
              if (c.error_info.error_pos < s || !(id > 0)) {
                c.error_info.error_pos = s;
                c.error_info.expected_tokens.clear();
              }
              if (fs.first_literal) {
                c.error_info.add(fs.first_literal, nullptr);
              } else {
                c.error_info.add(nullptr, fs.first_rule);
              }
            }
          }
          id++;
          continue;
        }
      }

      if (!c.cut_stack.empty()) { c.cut_stack.back() = false; }

      auto snap = c.snapshot(vs);
      c.error_info.keep_previous_token = id > 0;

      len = ope->parse(s, n, vs, c, dt);

      if (success(len)) {
        vs.choice_count_ = opes_.size();
        vs.choice_ = id;
        break;
      }

      c.rollback(vs, snap);

      if (!c.cut_stack.empty() && c.cut_stack.back()) { break; }

      id++;
    }

    c.error_info.keep_previous_token = false;
    return len;
  }

  void accept(Visitor &v) override;

  size_t size() const { return opes_.size(); }

  std::vector<std::shared_ptr<Ope>> opes_;
  bool for_label_ = false;
  std::vector<FirstSet> first_sets_;
};

class Repetition : public Ope {
public:
  Repetition(const std::shared_ptr<Ope> &ope, size_t min, size_t max)
      : ope_(ope), min_(min), max_(max) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    // ISpan fast path: tight loop for ASCII CharacterClass repetition.
    // Safe because each ASCII match is exactly 1 byte, so byte count == match
    // count.
    if (span_bitset_) {
      const auto &bitset = *span_bitset_;
      size_t i = 0;
      if (max_ == std::numeric_limits<size_t>::max()) {
        // Unbounded repetition (*, +): no per-iteration max check
        while (i < n && bitset.test(static_cast<unsigned char>(s[i]))) {
          i++;
        }
      } else {
        auto limit = std::min(n, max_);
        while (i < limit && bitset.test(static_cast<unsigned char>(s[i]))) {
          i++;
        }
      }
      if (i < min_) {
        c.set_error_pos(s + i);
        return static_cast<size_t>(-1);
      }
      return i;
    }

    size_t count = 0;
    size_t i = 0;
    while (count < min_) {
      auto len = ope_->parse(s + i, n - i, vs, c, dt);
      if (fail(len)) { return len; }
      i += len;
      count++;
    }

    while (count < max_) {
      auto snap = c.snapshot(vs);
      auto len = ope_->parse(s + i, n - i, vs, c, dt);
      if (fail(len)) {
        c.rollback(vs, snap);
        break;
      }
      i += len;
      count++;
    }
    return i;
  }

  void accept(Visitor &v) override;

  bool is_zom() const {
    return min_ == 0 && max_ == std::numeric_limits<size_t>::max();
  }

  static std::shared_ptr<Repetition> zom(const std::shared_ptr<Ope> &ope) {
    return std::make_shared<Repetition>(ope, 0,
                                        std::numeric_limits<size_t>::max());
  }

  static std::shared_ptr<Repetition> oom(const std::shared_ptr<Ope> &ope) {
    return std::make_shared<Repetition>(ope, 1,
                                        std::numeric_limits<size_t>::max());
  }

  static std::shared_ptr<Repetition> opt(const std::shared_ptr<Ope> &ope) {
    return std::make_shared<Repetition>(ope, 0, 1);
  }

  std::shared_ptr<Ope> ope_;
  size_t min_;
  size_t max_;
  const std::bitset<256> *span_bitset_ =
      nullptr; // non-owning, set by SetupFirstSets
};

class AndPredicate : public Ope {
public:
  AndPredicate(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    auto snap = c.snapshot(vs);
    auto len = ope_->parse(s, n, vs, c, dt);
    c.rollback(vs, snap); // Always rollback — predicates consume nothing
    if (success(len)) {
      return 0;
    } else {
      return len;
    }
  }

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class NotPredicate : public Ope {
public:
  NotPredicate(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    auto snap = c.snapshot(vs);
    auto len = ope_->parse(s, n, vs, c, dt);
    c.rollback(vs, snap); // Always rollback — predicates consume nothing
    if (success(len)) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    } else {
      return 0;
    }
  }

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class Dictionary : public Ope, public std::enable_shared_from_this<Dictionary> {
public:
  Dictionary(const std::vector<std::string> &v, bool ignore_case)
      : trie_(v, ignore_case) {
    is_choice_like = true;
  }

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  Trie trie_;
};

class LiteralString : public Ope,
                      public std::enable_shared_from_this<LiteralString> {
public:
  LiteralString(std::string &&s, bool ignore_case)
      : lit_(std::move(s)), ignore_case_(ignore_case),
        lower_lit_(ignore_case ? to_lower(lit_) : std::string()),
        is_word_(false) {}

  LiteralString(const std::string &s, bool ignore_case)
      : lit_(s), ignore_case_(ignore_case),
        lower_lit_(ignore_case ? to_lower(lit_) : std::string()),
        is_word_(false) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::string lit_;
  bool ignore_case_;
  std::string lower_lit_; // pre-computed for ignore_case
  mutable std::once_flag init_is_word_;
  mutable bool is_word_;
};

class CharacterClass : public Ope,
                       public std::enable_shared_from_this<CharacterClass> {
public:
  CharacterClass(const std::string &s, bool negated, bool ignore_case)
      : negated_(negated), ignore_case_(ignore_case) {
    auto chars = decode(s.data(), s.length());
    auto i = 0u;
    while (i < chars.size()) {
      if (i + 2 < chars.size() && chars[i + 1] == '-') {
        auto cp1 = chars[i];
        auto cp2 = chars[i + 2];
        ranges_.emplace_back(std::pair(cp1, cp2));
        i += 3;
      } else {
        auto cp = chars[i];
        ranges_.emplace_back(std::pair(cp, cp));
        i += 1;
      }
    }
    assert(!ranges_.empty());
    setup_ascii_bitset();
  }

  CharacterClass(const std::vector<std::pair<char32_t, char32_t>> &ranges,
                 bool negated, bool ignore_case)
      : ranges_(ranges), negated_(negated), ignore_case_(ignore_case) {
    assert(!ranges_.empty());
    setup_ascii_bitset();
  }

  size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const override {
    if (n < 1) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }

    char32_t cp = 0;
    auto len = decode_codepoint(s, n, cp);

    for (const auto &range : ranges_) {
      if (in_range(range, cp)) {
        if (negated_) {
          c.set_error_pos(s);
          return static_cast<size_t>(-1);
        } else {
          return len;
        }
      }
    }

    if (negated_) {
      return len;
    } else {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
  }

  void accept(Visitor &v) override;

  friend struct ComputeFirstSet;
  friend struct GrammarBlob;
  friend struct OpeSignature;

  bool is_ascii_only() const { return is_ascii_only_; }
  const std::bitset<256> &ascii_bitset() const { return ascii_bitset_; }

private:
  bool in_range(const std::pair<char32_t, char32_t> &range, char32_t cp) const {
    if (ignore_case_) {
      auto cpl = std::tolower(cp);
      return std::tolower(range.first) <= cpl &&
             cpl <= std::tolower(range.second);
    } else {
      return range.first <= cp && cp <= range.second;
    }
  }

  void setup_ascii_bitset() {
    if (negated_) { return; } // negated classes can match non-ASCII
    for (const auto &[lo, hi] : ranges_) {
      if (lo > 0x7F || hi > 0x7F) { return; }
    }
    is_ascii_only_ = true;
    for (const auto &[lo, hi] : ranges_) {
      for (auto cp = lo; cp <= hi; cp++) {
        auto ch = static_cast<unsigned char>(cp);
        ascii_bitset_.set(ch);
        if (ignore_case_) {
          ascii_bitset_.set(static_cast<unsigned char>(std::toupper(ch)));
          ascii_bitset_.set(static_cast<unsigned char>(std::tolower(ch)));
        }
      }
    }
  }

  std::vector<std::pair<char32_t, char32_t>> ranges_;
  bool negated_;
  bool ignore_case_;
  std::bitset<256> ascii_bitset_;
  bool is_ascii_only_ = false;
};

class Character : public Ope, public std::enable_shared_from_this<Character> {
public:
  Character(char32_t ch) : ch_(ch) {}

  size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const override {
    if (n < 1) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }

    char32_t cp = 0;
    auto len = decode_codepoint(s, n, cp);

    if (cp != ch_) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
    return len;
  }

  void accept(Visitor &v) override;

  char32_t ch_;
};

class AnyCharacter : public Ope,
                     public std::enable_shared_from_this<AnyCharacter> {
public:
  size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const override {
    auto len = codepoint_length(s, n);
    if (len < 1) {
      c.set_error_pos(s);
      return static_cast<size_t>(-1);
    }
    return len;
  }

  void accept(Visitor &v) override;
};

class CaptureScope : public Ope {
public:
  CaptureScope(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    auto cap_snap = c.capture_entries.size();
    auto len = ope_->parse(s, n, vs, c, dt);
    c.capture_entries.resize(cap_snap); // Always rollback (isolation)
    return len;
  }

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class Capture : public Ope {
public:
  using MatchAction = std::function<void(const char *s, size_t n, Context &c)>;

  Capture(const std::shared_ptr<Ope> &ope, MatchAction ma)
      : ope_(ope), match_action_(ma) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    auto len = ope_->parse(s, n, vs, c, dt);
    if (success(len) && match_action_) { match_action_(s, len, c); }
    return len;
  }

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
  MatchAction match_action_;
};

class TokenBoundary : public Ope {
public:
  TokenBoundary(const std::shared_ptr<Ope> &ope) : ope_(ope) {
    is_token_boundary = true;
  }

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class Ignore : public Ope {
public:
  Ignore(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

  size_t parse_core(const char *s, size_t n, SemanticValues & /*vs*/,
                    Context &c, std::any &dt) const override {
    auto &chvs = c.push_semantic_values_scope();
    auto se = scope_exit([&]() { c.pop_semantic_values_scope(); });
    return ope_->parse(s, n, chvs, c, dt);
  }

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

using Parser = std::function<size_t(const char *s, size_t n, SemanticValues &vs,
                                    std::any &dt)>;

class User : public Ope {
public:
  User(Parser fn) : fn_(fn) {}
  size_t parse_core(const char *s, size_t n, SemanticValues &vs,
                    Context & /*c*/, std::any &dt) const override {
    assert(fn_);
    return fn_(s, n, vs, dt);
  }
  void accept(Visitor &v) override;
  std::function<size_t(const char *s, size_t n, SemanticValues &vs,
                       std::any &dt)>
      fn_;
};

class WeakHolder : public Ope {
public:
  WeakHolder(const std::shared_ptr<Ope> &ope) : weak_(ope) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    auto ope = weak_.lock();
    assert(ope);
    return ope->parse(s, n, vs, c, dt);
  }

  void accept(Visitor &v) override;

  std::weak_ptr<Ope> weak_;
};

class Holder : public Ope {
public:
  Holder(Definition *outer) : outer_(outer) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::any reduce(SemanticValues &vs, std::any &dt,
                  const std::any &predicate_data) const;

  const std::string &name() const;
  const std::string &trace_name() const;

  std::shared_ptr<Ope> ope_;
  Definition *outer_;
  mutable std::once_flag trace_name_init_;
  mutable std::string trace_name_;

  friend class Definition;
};

using Grammar = std::unordered_map<std::string, Definition>;

class Reference : public Ope, public std::enable_shared_from_this<Reference> {
public:
  Reference(const Grammar &grammar, const std::string &name, const char *s,
            bool is_macro, const std::vector<std::shared_ptr<Ope>> &args)
      : grammar_(grammar), name_(name), s_(s), is_macro_(is_macro), args_(args),
        rule_(nullptr), iarg_(0) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> get_core_operator() const;

  const Grammar &grammar_;
  const std::string name_;
  const char *s_;

  const bool is_macro_;
  const std::vector<std::shared_ptr<Ope>> args_;

  Definition *rule_;
  size_t iarg_;
};

class Whitespace : public Ope {
public:
  Whitespace(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    if (c.in_whitespace) { return 0; }
    c.in_whitespace = true;
    auto se = scope_exit([&]() { c.in_whitespace = false; });
    return ope_->parse(s, n, vs, c, dt);
  }

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class BackReference : public Ope {
public:
  BackReference(std::string &&name) : name_(std::move(name)) {}

  BackReference(const std::string &name) : name_(name) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::string name_;
};

class PrecedenceClimbing : public Ope {
public:
  using BinOpeInfo = std::map<std::string_view, std::pair<size_t, char>>;

  PrecedenceClimbing(const std::shared_ptr<Ope> &atom,
                     const std::shared_ptr<Ope> &binop, const BinOpeInfo &info,
                     const Definition &rule)
      : atom_(atom), binop_(binop), info_(info), rule_(rule) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override {
    return parse_expression(s, n, vs, c, dt, 0);
  }

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> atom_;
  std::shared_ptr<Ope> binop_;
  BinOpeInfo info_;
  // Owned backing storage for info_ keys when this node is built by
  // GrammarBlob::deserialize. Grammars parsed from source leave this empty and
  // point info_ keys into the retained grammar text instead.
  std::vector<std::string> info_keys_;
  const Definition &rule_;

private:
  size_t parse_expression(const char *s, size_t n, SemanticValues &vs,
                          Context &c, std::any &dt, size_t min_prec) const;

  Definition &get_reference_for_binop(Context &c) const;
};

class Recovery : public Ope {
public:
  Recovery(const std::shared_ptr<Ope> &ope) : ope_(ope) {}

  size_t parse_core(const char *s, size_t n, SemanticValues &vs, Context &c,
                    std::any &dt) const override;

  void accept(Visitor &v) override;

  std::shared_ptr<Ope> ope_;
};

class Cut : public Ope, public std::enable_shared_from_this<Cut> {
public:
  size_t parse_core(const char * /*s*/, size_t /*n*/, SemanticValues & /*vs*/,
                    Context &c, std::any & /*dt*/) const override {
    if (!c.cut_stack.empty()) { c.cut_stack.back() = true; }
    return 0;
  }

  void accept(Visitor &v) override;
};

/*
 * Factories
 */
template <typename... Args> std::shared_ptr<Ope> seq(Args &&...args) {
  return std::make_shared<Sequence>(static_cast<std::shared_ptr<Ope>>(args)...);
}

template <typename... Args> std::shared_ptr<Ope> cho(Args &&...args) {
  return std::make_shared<PrioritizedChoice>(
      false, static_cast<std::shared_ptr<Ope>>(args)...);
}

template <typename... Args> std::shared_ptr<Ope> cho4label_(Args &&...args) {
  return std::make_shared<PrioritizedChoice>(
      true, static_cast<std::shared_ptr<Ope>>(args)...);
}

inline std::shared_ptr<Ope> zom(const std::shared_ptr<Ope> &ope) {
  return Repetition::zom(ope);
}

inline std::shared_ptr<Ope> oom(const std::shared_ptr<Ope> &ope) {
  return Repetition::oom(ope);
}

inline std::shared_ptr<Ope> opt(const std::shared_ptr<Ope> &ope) {
  return Repetition::opt(ope);
}

inline std::shared_ptr<Ope> rep(const std::shared_ptr<Ope> &ope, size_t min,
                                size_t max) {
  return std::make_shared<Repetition>(ope, min, max);
}

inline std::shared_ptr<Ope> apd(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<AndPredicate>(ope);
}

inline std::shared_ptr<Ope> npd(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<NotPredicate>(ope);
}

inline std::shared_ptr<Ope> dic(const std::vector<std::string> &v,
                                bool ignore_case) {
  return std::make_shared<Dictionary>(v, ignore_case);
}

inline std::shared_ptr<Ope> lit(std::string &&s) {
  return std::make_shared<LiteralString>(s, false);
}

inline std::shared_ptr<Ope> liti(std::string &&s) {
  return std::make_shared<LiteralString>(s, true);
}

inline std::shared_ptr<Ope> cls(const std::string &s) {
  return std::make_shared<CharacterClass>(s, false, false);
}

inline std::shared_ptr<Ope>
cls(const std::vector<std::pair<char32_t, char32_t>> &ranges,
    bool ignore_case = false) {
  return std::make_shared<CharacterClass>(ranges, false, ignore_case);
}

inline std::shared_ptr<Ope> ncls(const std::string &s) {
  return std::make_shared<CharacterClass>(s, true, false);
}

inline std::shared_ptr<Ope>
ncls(const std::vector<std::pair<char32_t, char32_t>> &ranges,
     bool ignore_case = false) {
  return std::make_shared<CharacterClass>(ranges, true, ignore_case);
}

inline std::shared_ptr<Ope> chr(char32_t dt) {
  return std::make_shared<Character>(dt);
}

inline std::shared_ptr<Ope> dot() { return std::make_shared<AnyCharacter>(); }

inline std::shared_ptr<Ope> csc(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<CaptureScope>(ope);
}

inline std::shared_ptr<Ope> cap(const std::shared_ptr<Ope> &ope,
                                Capture::MatchAction ma) {
  return std::make_shared<Capture>(ope, ma);
}

inline std::shared_ptr<Ope> tok(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<TokenBoundary>(ope);
}

inline std::shared_ptr<Ope> ign(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<Ignore>(ope);
}

inline std::shared_ptr<Ope>
usr(std::function<size_t(const char *s, size_t n, SemanticValues &vs,
                         std::any &dt)>
        fn) {
  return std::make_shared<User>(fn);
}

inline std::shared_ptr<Ope> ref(const Grammar &grammar, const std::string &name,
                                const char *s, bool is_macro,
                                const std::vector<std::shared_ptr<Ope>> &args) {
  return std::make_shared<Reference>(grammar, name, s, is_macro, args);
}

inline std::shared_ptr<Ope> wsp(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<Whitespace>(std::make_shared<Ignore>(ope));
}

inline std::shared_ptr<Ope> bkr(std::string &&name) {
  return std::make_shared<BackReference>(name);
}

inline std::shared_ptr<Ope> pre(const std::shared_ptr<Ope> &atom,
                                const std::shared_ptr<Ope> &binop,
                                const PrecedenceClimbing::BinOpeInfo &info,
                                const Definition &rule) {
  return std::make_shared<PrecedenceClimbing>(atom, binop, info, rule);
}

inline std::shared_ptr<Ope> rec(const std::shared_ptr<Ope> &ope) {
  return std::make_shared<Recovery>(ope);
}

inline std::shared_ptr<Ope> cut() { return std::make_shared<Cut>(); }

/*
 * Visitor
 */
struct Ope::Visitor {
  virtual ~Visitor() {}
  virtual void visit(Sequence &) {}
  virtual void visit(PrioritizedChoice &) {}
  virtual void visit(Repetition &) {}
  virtual void visit(AndPredicate &) {}
  virtual void visit(NotPredicate &) {}
  virtual void visit(Dictionary &) {}
  virtual void visit(LiteralString &) {}
  virtual void visit(CharacterClass &) {}
  virtual void visit(Character &) {}
  virtual void visit(AnyCharacter &) {}
  virtual void visit(CaptureScope &) {}
  virtual void visit(Capture &) {}
  virtual void visit(TokenBoundary &) {}
  virtual void visit(Ignore &) {}
  virtual void visit(User &) {}
  virtual void visit(WeakHolder &) {}
  virtual void visit(Holder &) {}
  virtual void visit(Reference &) {}
  virtual void visit(Whitespace &) {}
  virtual void visit(BackReference &) {}
  virtual void visit(PrecedenceClimbing &) {}
  virtual void visit(Recovery &) {}
  virtual void visit(Cut &) {}
};

struct TraversalVisitor : public Ope::Visitor {
  using Ope::Visitor::visit;
  void visit(Sequence &ope) override {
    for (auto &op : ope.opes_) {
      op->accept(*this);
    }
  }
  void visit(PrioritizedChoice &ope) override {
    for (auto &op : ope.opes_) {
      op->accept(*this);
    }
  }
  void visit(Repetition &ope) override { ope.ope_->accept(*this); }
  void visit(AndPredicate &ope) override { ope.ope_->accept(*this); }
  void visit(NotPredicate &ope) override { ope.ope_->accept(*this); }
  void visit(CaptureScope &ope) override { ope.ope_->accept(*this); }
  void visit(Capture &ope) override { ope.ope_->accept(*this); }
  void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
  void visit(Ignore &ope) override { ope.ope_->accept(*this); }
  void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
  void visit(Holder &ope) override { ope.ope_->accept(*this); }
  void visit(Whitespace &ope) override { ope.ope_->accept(*this); }
  void visit(Recovery &ope) override { ope.ope_->accept(*this); }
  void visit(PrecedenceClimbing &ope) override { ope.atom_->accept(*this); }
};

struct TraceOpeName : public Ope::Visitor {
  using Ope::Visitor::visit;

  void visit(Sequence &) override { name_ = "Sequence"; }
  void visit(PrioritizedChoice &) override { name_ = "PrioritizedChoice"; }
  void visit(Repetition &) override { name_ = "Repetition"; }
  void visit(AndPredicate &) override { name_ = "AndPredicate"; }
  void visit(NotPredicate &) override { name_ = "NotPredicate"; }
  void visit(Dictionary &) override { name_ = "Dictionary"; }
  void visit(LiteralString &) override { name_ = "LiteralString"; }
  void visit(CharacterClass &) override { name_ = "CharacterClass"; }
  void visit(Character &) override { name_ = "Character"; }
  void visit(AnyCharacter &) override { name_ = "AnyCharacter"; }
  void visit(CaptureScope &) override { name_ = "CaptureScope"; }
  void visit(Capture &) override { name_ = "Capture"; }
  void visit(TokenBoundary &) override { name_ = "TokenBoundary"; }
  void visit(Ignore &) override { name_ = "Ignore"; }
  void visit(User &) override { name_ = "User"; }
  void visit(WeakHolder &) override { name_ = "WeakHolder"; }
  void visit(Holder &ope) override { name_ = ope.trace_name().data(); }
  void visit(Reference &) override { name_ = "Reference"; }
  void visit(Whitespace &) override { name_ = "Whitespace"; }
  void visit(BackReference &) override { name_ = "BackReference"; }
  void visit(PrecedenceClimbing &) override { name_ = "PrecedenceClimbing"; }
  void visit(Recovery &) override { name_ = "Recovery"; }
  void visit(Cut &) override { name_ = "Cut"; }

  static std::string get(Ope &ope) {
    TraceOpeName vis;
    ope.accept(vis);
    return vis.name_;
  }

private:
  const char *name_ = nullptr;
};

struct AssignIDToDefinition : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(Holder &ope) override;
  void visit(Reference &ope) override;
  void visit(PrecedenceClimbing &ope) override;

  std::unordered_map<void *, size_t> ids;
};

struct IsLiteralToken : public Ope::Visitor {
  using Ope::Visitor::visit;

  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      if (!IsLiteralToken::check(*op)) { return; }
    }
    result_ = true;
  }

  void visit(Dictionary &) override { result_ = true; }
  void visit(LiteralString &) override { result_ = true; }

  static bool check(Ope &ope) {
    IsLiteralToken vis;
    ope.accept(vis);
    return vis.result_;
  }

private:
  bool result_ = false;
};

struct TokenChecker : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(TokenBoundary &) override { has_token_boundary_ = true; }
  void visit(AndPredicate &) override {}
  void visit(NotPredicate &) override {}
  void visit(WeakHolder &) override { has_rule_ = true; }
  void visit(Reference &ope) override;

  static bool is_token(Ope &ope) {
    if (IsLiteralToken::check(ope)) { return true; }

    TokenChecker vis;
    ope.accept(vis);
    return vis.has_token_boundary_ || !vis.has_rule_;
  }

private:
  bool has_token_boundary_ = false;
  bool has_rule_ = false;
};

struct FindLiteralToken : public Ope::Visitor {
  using Ope::Visitor::visit;

  void visit(LiteralString &ope) override { token_ = ope.lit_.data(); }
  void visit(TokenBoundary &ope) override { ope.ope_->accept(*this); }
  void visit(Ignore &ope) override { ope.ope_->accept(*this); }
  void visit(Reference &ope) override;
  void visit(Recovery &ope) override { ope.ope_->accept(*this); }

  static const char *token(Ope &ope) {
    FindLiteralToken vis;
    ope.accept(vis);
    return vis.token_;
  }

private:
  const char *token_ = nullptr;
};

struct DetectLeftRecursion : public TraversalVisitor {
  using TraversalVisitor::visit;

  DetectLeftRecursion(const std::string &name) : name_(name) {}

  void visit(Sequence &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (done_) {
        break;
      } else if (error_s) {
        done_ = true;
        break;
      }
    }
  }
  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (error_s) {
        done_ = true;
        break;
      }
    }
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    done_ = ope.min_ > 0;
  }
  void visit(AndPredicate &ope) override {
    ope.ope_->accept(*this);
    done_ = false;
  }
  void visit(NotPredicate &ope) override {
    ope.ope_->accept(*this);
    done_ = false;
  }
  void visit(Dictionary &) override { done_ = true; }
  void visit(LiteralString &ope) override { done_ = !ope.lit_.empty(); }
  void visit(CharacterClass &) override { done_ = true; }
  void visit(Character &) override { done_ = true; }
  void visit(AnyCharacter &) override { done_ = true; }
  void visit(User &) override { done_ = true; }
  void visit(Reference &ope) override;
  void visit(BackReference &) override { done_ = true; }
  void visit(Cut &) override { done_ = true; }

  const char *error_s = nullptr;

  // What a bare parameter reference denotes, plus the frame it was found at
  // -- see visit_in_defining_scope.
  struct ResolvedArg {
    std::shared_ptr<Ope> ope;
    size_t depth = 0;
  };
  ResolvedArg resolve_macro_arg(size_t iarg) const;
  void visit_in_defining_scope(const ResolvedArg &arg);

  // A macro's body depends on its arguments, so "already visited" has to be
  // per instantiation, not per name: in `A <- W('z') / W(A)`, visiting W with
  // 'z' says nothing about W with A. Instantiations are identified by their
  // resolved arguments, the same way as at parse time.
  size_t intern_macro_inst(const Reference &ope);

  // A macro that instantiates itself with a growing argument
  // (`M(s) <- M(s / 'x')`) has no finite set of instantiations. Stop
  // descending instead of looping forever; the rule is then reported as
  // non-left-recursive, which is what this analysis did for every macro
  // before it became instantiation-aware. The bound is on nesting depth in
  // general, not self-recursion specifically, so it also caps any other
  // chain of nested macro calls -- generously, for real grammars.
  static const size_t max_macro_inst_depth = 32;

private:
  std::string name_;
  std::set<std::pair<const Definition *, size_t>> refs_;
  std::map<std::vector<const void *>, size_t> macro_inst_ids_;
  size_t next_macro_inst_ = 1;
  bool done_ = false;
  std::vector<const std::vector<std::shared_ptr<Ope>> *> macro_args_stack_;
};

struct ComputeCanBeEmpty : public TraversalVisitor {
  using TraversalVisitor::visit;

  bool result = false;

  void visit(Sequence &ope) override {
    result = std::all_of(ope.opes_.begin(), ope.opes_.end(), [](auto &op) {
      ComputeCanBeEmpty vis;
      op->accept(vis);
      return vis.result;
    });
  }
  void visit(PrioritizedChoice &ope) override {
    result = std::any_of(ope.opes_.begin(), ope.opes_.end(), [](auto &op) {
      ComputeCanBeEmpty vis;
      op->accept(vis);
      return vis.result;
    });
  }
  void visit(Repetition &ope) override { result = ope.min_ == 0; }
  void visit(AndPredicate &) override { result = true; }
  void visit(NotPredicate &) override { result = true; }
  void visit(Dictionary &) override { result = false; }
  void visit(LiteralString &ope) override { result = ope.lit_.empty(); }
  void visit(CharacterClass &) override { result = false; }
  void visit(Character &) override { result = false; }
  void visit(AnyCharacter &) override { result = false; }
  void visit(User &) override { result = false; }
  void visit(Reference &ope) override;
  void visit(BackReference &) override { result = false; }
  void visit(Cut &) override { result = false; }
};

// Structural signature of an Ope. Two alternatives whose first k elements
// have equal signatures consume the same text, so their (k+1)-th elements
// start at the same position. Opes whose state cannot be serialized get
// their address instead: that only ever reads as "these differ", which
// costs an optimization rather than adding one.
struct OpeSignature : public Ope::Visitor {
  using Ope::Visitor::visit;
  std::string s;

  void visit(Sequence &ope) override { group("seq", ope.opes_); }
  void visit(PrioritizedChoice &ope) override { group("cho", ope.opes_); }
  void visit(Repetition &ope) override {
    s += "(rep " + std::to_string(ope.min_) + " " +
         (ope.max_ == std::numeric_limits<size_t>::max()
              ? std::string("inf")
              : std::to_string(ope.max_));
    wrap(*ope.ope_);
  }
  void visit(AndPredicate &ope) override { unary("and", *ope.ope_); }
  void visit(NotPredicate &ope) override { unary("not", *ope.ope_); }
  void visit(CaptureScope &ope) override { unary("cps", *ope.ope_); }
  void visit(Capture &ope) override { unary("cap", *ope.ope_); }
  void visit(TokenBoundary &ope) override { unary("tok", *ope.ope_); }
  void visit(Ignore &ope) override { unary("ign", *ope.ope_); }
  void visit(Whitespace &ope) override { unary("wsp", *ope.ope_); }
  void visit(Recovery &ope) override { unary("rec", *ope.ope_); }
  // A rule is named, never expanded — that is what keeps a recursive
  // grammar's signature finite. WeakHolder only ever wraps a Holder, so
  // descending through it lands on a name too.
  void visit(Holder &ope) override { s += "(hld " + ope.name() + ")"; }
  void visit(WeakHolder &ope) override {
    if (auto p = ope.weak_.lock()) {
      unary("wek", *p);
    } else {
      opaque(&ope);
    }
  }
  void visit(Reference &ope) override {
    s += "(ref " + ope.name_;
    for (auto &arg : ope.args_) {
      s += ' ';
      arg->accept(*this);
    }
    s += ')';
  }
  void visit(LiteralString &ope) override {
    s += "(lit " + std::to_string(ope.ignore_case_) + " " + ope.lit_ + ")";
  }
  void visit(CharacterClass &ope) override {
    s += "(cls " + std::to_string(ope.negated_) + " " +
         std::to_string(ope.ignore_case_);
    for (const auto &[lo, hi] : ope.ranges_) {
      s += " " + std::to_string(static_cast<uint32_t>(lo)) + "-" +
           std::to_string(static_cast<uint32_t>(hi));
    }
    s += ')';
  }
  void visit(Character &ope) override {
    s += "(chr " + std::to_string(static_cast<uint32_t>(ope.ch_)) + ")";
  }
  void visit(AnyCharacter &) override { s += "(any)"; }
  void visit(Dictionary &ope) override { opaque(&ope); }
  void visit(User &ope) override { opaque(&ope); }
  void visit(BackReference &ope) override { opaque(&ope); }
  void visit(PrecedenceClimbing &ope) override { opaque(&ope); }
  void visit(Cut &ope) override { opaque(&ope); }

  static std::string get(Ope &ope) {
    OpeSignature vis;
    ope.accept(vis);
    return std::move(vis.s);
  }

private:
  void group(const char *tag, const std::vector<std::shared_ptr<Ope>> &v) {
    s += '(';
    s += tag;
    for (const auto &op : v) {
      s += ' ';
      op->accept(*this);
    }
    s += ')';
  }
  void unary(const char *tag, Ope &inner) {
    s += '(';
    s += tag;
    wrap(inner);
  }
  void wrap(Ope &inner) {
    s += ' ';
    inner.accept(*this);
    s += ')';
  }
  void opaque(const void *p) {
    s += "(opq " + std::to_string(reinterpret_cast<std::uintptr_t>(p)) + ")";
  }
};

struct HasEmptyElement : public TraversalVisitor {
  using TraversalVisitor::visit;

  HasEmptyElement(std::vector<std::pair<const char *, std::string>> &refs,
                  std::unordered_map<std::string, bool> &has_error_cache)
      : refs_(refs), has_error_cache_(has_error_cache) {}

  void visit(Sequence &ope) override;
  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (is_empty) { return; }
    }
  }
  void visit(Repetition &ope) override {
    if (ope.min_ == 0) {
      set_error();
    } else {
      ope.ope_->accept(*this);
    }
  }
  void visit(AndPredicate &) override { set_error(); }
  void visit(NotPredicate &) override { set_error(); }
  void visit(LiteralString &ope) override {
    if (ope.lit_.empty()) { set_error(); }
  }
  void visit(Reference &ope) override;

  bool is_empty = false;
  const char *error_s = nullptr;
  std::string error_name;

private:
  void set_error() {
    is_empty = true;
    tie(error_s, error_name) = refs_.back();
  }
  std::vector<std::pair<const char *, std::string>> &refs_;
  std::unordered_map<std::string, bool> &has_error_cache_;
};

struct DetectInfiniteLoop : public TraversalVisitor {
  using TraversalVisitor::visit;

  DetectInfiniteLoop(const char *s, const std::string &name,
                     std::vector<std::pair<const char *, std::string>> &refs,
                     std::unordered_map<std::string, bool> &has_error_cache)
      : refs_(refs), has_error_cache_(has_error_cache) {
    refs_.emplace_back(s, name);
  }

  DetectInfiniteLoop(std::vector<std::pair<const char *, std::string>> &refs,
                     std::unordered_map<std::string, bool> &has_error_cache)
      : refs_(refs), has_error_cache_(has_error_cache) {}

  void visit(Sequence &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (has_error) { return; }
    }
  }
  void visit(PrioritizedChoice &ope) override {
    for (const auto &op : ope.opes_) {
      op->accept(*this);
      if (has_error) { return; }
    }
  }
  void visit(Repetition &ope) override {
    if (ope.max_ == std::numeric_limits<size_t>::max()) {
      HasEmptyElement vis(refs_, has_error_cache_);
      ope.ope_->accept(vis);
      if (vis.is_empty) {
        has_error = true;
        error_s = vis.error_s;
        error_name = vis.error_name;
      }
    } else {
      ope.ope_->accept(*this);
    }
  }
  void visit(Reference &ope) override;

  bool has_error = false;
  const char *error_s = nullptr;
  std::string error_name;

private:
  std::vector<std::pair<const char *, std::string>> &refs_;
  std::unordered_map<std::string, bool> &has_error_cache_;
};

struct ReferenceChecker : public TraversalVisitor {
  using TraversalVisitor::visit;

  ReferenceChecker(const Grammar &grammar,
                   const std::vector<std::string> &params)
      : grammar_(grammar), params_(params) {}

  void visit(Reference &ope) override;

  std::unordered_map<std::string, const char *> error_s;
  std::unordered_map<std::string, std::string> error_message;
  std::unordered_set<std::string> referenced;

private:
  const Grammar &grammar_;
  const std::vector<std::string> &params_;
};

struct LinkReferences : public TraversalVisitor {
  using TraversalVisitor::visit;

  LinkReferences(Grammar &grammar, const std::vector<std::string> &params)
      : grammar_(grammar), params_(params) {}

  void visit(Reference &ope) override;

private:
  Grammar &grammar_;
  const std::vector<std::string> &params_;
};

struct FindReference : public Ope::Visitor {
  using Ope::Visitor::visit;

  FindReference(const std::vector<std::shared_ptr<Ope>> &args,
                const std::vector<std::string> &params)
      : args_(args), params_(params) {}

  void visit(Sequence &ope) override {
    std::vector<std::shared_ptr<Ope>> opes;
    for (const auto &o : ope.opes_) {
      o->accept(*this);
      opes.emplace_back(std::move(found_ope));
    }
    found_ope = std::make_shared<Sequence>(opes);
  }
  void visit(PrioritizedChoice &ope) override {
    std::vector<std::shared_ptr<Ope>> opes;
    for (const auto &o : ope.opes_) {
      o->accept(*this);
      opes.emplace_back(std::move(found_ope));
    }
    found_ope = std::make_shared<PrioritizedChoice>(opes);
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    found_ope = rep(found_ope, ope.min_, ope.max_);
  }
  void visit(AndPredicate &ope) override {
    ope.ope_->accept(*this);
    found_ope = apd(found_ope);
  }
  void visit(NotPredicate &ope) override {
    ope.ope_->accept(*this);
    found_ope = npd(found_ope);
  }
  void visit(Dictionary &ope) override { found_ope = ope.shared_from_this(); }
  void visit(LiteralString &ope) override {
    found_ope = ope.shared_from_this();
  }
  void visit(CharacterClass &ope) override {
    found_ope = ope.shared_from_this();
  }
  void visit(Character &ope) override { found_ope = ope.shared_from_this(); }
  void visit(AnyCharacter &ope) override { found_ope = ope.shared_from_this(); }
  void visit(CaptureScope &ope) override {
    ope.ope_->accept(*this);
    found_ope = csc(found_ope);
  }
  void visit(Capture &ope) override {
    ope.ope_->accept(*this);
    found_ope = cap(found_ope, ope.match_action_);
  }
  void visit(TokenBoundary &ope) override {
    ope.ope_->accept(*this);
    found_ope = tok(found_ope);
  }
  void visit(Ignore &ope) override {
    ope.ope_->accept(*this);
    found_ope = ign(found_ope);
  }
  void visit(WeakHolder &ope) override { ope.weak_.lock()->accept(*this); }
  void visit(Holder &ope) override { ope.ope_->accept(*this); }
  void visit(Reference &ope) override;
  void visit(Whitespace &ope) override {
    ope.ope_->accept(*this);
    found_ope = wsp(found_ope);
  }
  void visit(PrecedenceClimbing &ope) override {
    ope.atom_->accept(*this);
    found_ope = csc(found_ope);
  }
  void visit(Recovery &ope) override {
    ope.ope_->accept(*this);
    found_ope = rec(found_ope);
  }
  void visit(Cut &ope) override { found_ope = ope.shared_from_this(); }

  std::shared_ptr<Ope> found_ope;

private:
  const std::vector<std::shared_ptr<Ope>> &args_;
  const std::vector<std::string> &params_;
};

/*
 * First-Set computation
 */
struct ComputeFirstSet : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(Sequence &ope) override {
    for (const auto &op : ope.opes_) {
      FirstSet element_fs;
      auto save = result_;
      result_ = FirstSet{};
      op->accept(*this);
      element_fs = result_;
      result_ = save;
      result_.chars |= element_fs.chars;
      if (element_fs.any_char) { result_.any_char = true; }
      if (!result_.first_literal) {
        result_.first_literal = element_fs.first_literal;
      }
      if (!result_.first_rule) { result_.first_rule = element_fs.first_rule; }
      if (!element_fs.can_be_empty) { return; }
      // This element can be empty, continue to next
    }
    result_.can_be_empty = true;
  }
  void visit(PrioritizedChoice &ope) override {
    auto save = result_;
    for (const auto &op : ope.opes_) {
      result_ = FirstSet{};
      op->accept(*this);
      save.merge(result_);
    }
    result_ = save;
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    if (ope.min_ == 0) { result_.can_be_empty = true; }
  }
  void visit(AndPredicate &) override { result_.can_be_empty = true; }
  void visit(NotPredicate &) override { result_.can_be_empty = true; }
  void visit(Dictionary &ope) override {
    for (const auto &[key, info] : ope.trie_.dic_) {
      if (!key.empty()) {
        auto ch = static_cast<unsigned char>(key[0]);
        result_.chars.set(ch);
        if (ope.trie_.ignore_case_) {
          result_.chars.set(static_cast<unsigned char>(std::toupper(ch)));
          result_.chars.set(static_cast<unsigned char>(std::tolower(ch)));
        }
      }
    }
  }
  void visit(LiteralString &ope) override {
    if (ope.lit_.empty()) {
      result_.can_be_empty = true;
    } else {
      auto ch = static_cast<unsigned char>(ope.lit_[0]);
      result_.chars.set(ch);
      if (ope.ignore_case_) {
        result_.chars.set(static_cast<unsigned char>(std::toupper(ch)));
        result_.chars.set(static_cast<unsigned char>(std::tolower(ch)));
      }
      if (!result_.first_literal) { result_.first_literal = ope.lit_.c_str(); }
    }
  }
  void visit(CharacterClass &ope) override {
    for (const auto &range : ope.ranges_) {
      auto cp1 = range.first;
      auto cp2 = range.second;
      if (cp1 > 0x7F || cp2 > 0x7F) {
        // Non-ASCII range: conservative fallback
        result_.any_char = true;
        return;
      }
      for (auto cp = cp1; cp <= cp2; cp++) {
        auto ch = static_cast<unsigned char>(cp);
        result_.chars.set(ch);
        if (ope.ignore_case_) {
          result_.chars.set(static_cast<unsigned char>(std::toupper(ch)));
          result_.chars.set(static_cast<unsigned char>(std::tolower(ch)));
        }
      }
    }
    if (ope.negated_) {
      result_.chars.flip();
      result_.any_char = true; // negated class can match non-ASCII
    }
  }
  void visit(Character &ope) override {
    if (ope.ch_ > 0x7F) {
      result_.any_char = true;
    } else {
      result_.chars.set(static_cast<unsigned char>(ope.ch_));
    }
  }
  void visit(AnyCharacter &) override { result_.any_char = true; }
  void visit(User &) override { result_.any_char = true; }
  void visit(Reference &ope) override;
  void visit(BackReference &) override { result_.any_char = true; }
  void visit(Cut &) override { result_.can_be_empty = true; }

  // Per-rule cache shared across a SetupFirstSets traversal. Without it,
  // every alternative of every PrioritizedChoice re-walks referenced
  // rules — O(refs^depth) work for grammars with many cross-references.
  // Only cycle-free rule computations are cached; results computed under
  // a cycle (left recursion) would be incomplete and unsafe to reuse from
  // a different call context.
  using FirstSetCache = std::unordered_map<const Definition *, FirstSet>;

  explicit ComputeFirstSet(FirstSetCache &cache) : cache_(cache) {}

  FirstSet result_;

private:
  FirstSetCache &cache_;
  std::unordered_set<const Definition *> refs_;
  size_t cycle_count_ = 0;
};

struct SetupFirstSets : public TraversalVisitor {
  using TraversalVisitor::visit;

  void visit(Sequence &ope) override;
  void setup_keyword_guarded_identifier(Sequence &ope);

  void visit(PrioritizedChoice &ope) override {
    ope.first_sets_.clear();
    ope.first_sets_.reserve(ope.opes_.size());
    for (const auto &op : ope.opes_) {
      ComputeFirstSet cfs(first_set_cache_);
      op->accept(cfs);
      ope.first_sets_.push_back(cfs.result_);
    }
    for (const auto &op : ope.opes_) {
      op->accept(*this);
    }
  }
  void visit(Repetition &ope) override {
    ope.ope_->accept(*this);
    // ISpan optimization: detect Repetition + ASCII CharacterClass
    auto cc = dynamic_cast<CharacterClass *>(ope.ope_.get());
    if (cc && cc->is_ascii_only()) { ope.span_bitset_ = &cc->ascii_bitset(); }
  }
  void visit(Reference &ope) override;
  void visit(Holder &ope) override;

private:
  ComputeFirstSet::FirstSetCache first_set_cache_;
  std::unordered_set<const Definition *> visited_rules_;
};

/*
 * Keywords
 */
static const char *WHITESPACE_DEFINITION_NAME = "%whitespace";
static const char *WORD_DEFINITION_NAME = "%word";
static const char *RECOVER_DEFINITION_NAME = "%recover";

/*
 * Definition
 */
class Definition {
public:
  struct Result {
    bool ret;
    bool recovered;
    size_t len;
    ErrorInfo error_info;
  };

  Definition() : holder_(std::make_shared<Holder>(this)) {}

  Definition(const Definition &rhs) : name(rhs.name), holder_(rhs.holder_) {
    holder_->outer_ = this;
  }

  Definition(const std::shared_ptr<Ope> &ope)
      : holder_(std::make_shared<Holder>(this)) {
    *this <= ope;
  }

  operator std::shared_ptr<Ope>() {
    return std::make_shared<WeakHolder>(holder_);
  }

  Definition &operator<=(const std::shared_ptr<Ope> &ope) {
    holder_->ope_ = ope;
    return *this;
  }

  Result parse(const char *s, size_t n, const char *path = nullptr,
               Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const {
    SemanticValues vs;
    std::any dt;
    return parse_core(s, n, vs, dt, path, log, error_reporter);
  }

  Result parse(const char *s, const char *path = nullptr, Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const {
    auto n = strlen(s);
    return parse(s, n, path, log, error_reporter);
  }

  Result parse(const char *s, size_t n, std::any &dt,
               const char *path = nullptr, Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const {
    SemanticValues vs;
    return parse_core(s, n, vs, dt, path, log, error_reporter);
  }

  Result parse(const char *s, std::any &dt, const char *path = nullptr,
               Log log = nullptr,
               ErrorReporter error_reporter = nullptr) const {
    auto n = strlen(s);
    return parse(s, n, dt, path, log, error_reporter);
  }

  template <typename T>
  Result parse_and_get_value(const char *s, size_t n, T &val,
                             const char *path = nullptr, Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    SemanticValues vs;
    std::any dt;
    auto r = parse_core(s, n, vs, dt, path, log, error_reporter);
    if (r.ret && !vs.empty() && vs.front().has_value()) {
      val = std::any_cast<T>(vs[0]);
    }
    return r;
  }

  template <typename T>
  Result parse_and_get_value(const char *s, T &val, const char *path = nullptr,
                             Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    auto n = strlen(s);
    return parse_and_get_value(s, n, val, path, log, error_reporter);
  }

  template <typename T>
  Result parse_and_get_value(const char *s, size_t n, std::any &dt, T &val,
                             const char *path = nullptr, Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    SemanticValues vs;
    auto r = parse_core(s, n, vs, dt, path, log, error_reporter);
    if (r.ret && !vs.empty() && vs.front().has_value()) {
      val = std::any_cast<T>(vs[0]);
    }
    return r;
  }

  template <typename T>
  Result parse_and_get_value(const char *s, std::any &dt, T &val,
                             const char *path = nullptr, Log log = nullptr,
                             ErrorReporter error_reporter = nullptr) const {
    auto n = strlen(s);
    return parse_and_get_value(s, n, dt, val, path, log, error_reporter);
  }

#if defined(__cpp_lib_char8_t)
  Result parse(const char8_t *s, size_t n, const char *path = nullptr,
               Log log = nullptr) const {
    return parse(reinterpret_cast<const char *>(s), n, path, log);
  }

  Result parse(const char8_t *s, const char *path = nullptr,
               Log log = nullptr) const {
    return parse(reinterpret_cast<const char *>(s), path, log);
  }

  Result parse(const char8_t *s, size_t n, std::any &dt,
               const char *path = nullptr, Log log = nullptr) const {
    return parse(reinterpret_cast<const char *>(s), n, dt, path, log);
  }

  Result parse(const char8_t *s, std::any &dt, const char *path = nullptr,
               Log log = nullptr) const {
    return parse(reinterpret_cast<const char *>(s), dt, path, log);
  }

  template <typename T>
  Result parse_and_get_value(const char8_t *s, size_t n, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), n, val, path,
                               log);
  }

  template <typename T>
  Result parse_and_get_value(const char8_t *s, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), val, path,
                               log);
  }

  template <typename T>
  Result parse_and_get_value(const char8_t *s, size_t n, std::any &dt, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), n, dt, val,
                               path, log);
  }

  template <typename T>
  Result parse_and_get_value(const char8_t *s, std::any &dt, T &val,
                             const char *path = nullptr,
                             Log log = nullptr) const {
    return parse_and_get_value(reinterpret_cast<const char *>(s), dt, val, path,
                               log);
  }
#endif

  void operator=(Action a) { action = a; }

  template <typename T> Definition &operator,(T fn) {
    operator=(fn);
    return *this;
  }

  Definition &operator~() {
    ignoreSemanticValue = true;
    return *this;
  }

  void accept(Ope::Visitor &v) { holder_->accept(v); }

  std::shared_ptr<Ope> get_core_operator() const { return holder_->ope_; }

  bool is_token() const {
    std::call_once(is_token_init_, [this]() {
      is_token_ = TokenChecker::is_token(*get_core_operator());
    });
    return is_token_;
  }

  std::string name;
  const char *s_ = nullptr;
  std::pair<size_t, size_t> line_ = {1, 1};

  Predicate predicate;

  size_t id = 0;
  Action action;
  std::function<void(const Context &c, const char *s, size_t n, std::any &dt)>
      enter;
  std::function<void(const Context &c, const char *s, size_t n, size_t matchlen,
                     std::any &value, std::any &dt)>
      leave;
  bool ignoreSemanticValue = false;
  std::shared_ptr<Ope> whitespaceOpe;
  std::shared_ptr<Ope> wordOpe;
  bool enablePackratParsing = false;
  bool is_macro = false;
  std::vector<std::string> params;
  bool disable_action = false;
  bool is_left_recursive = false;
  bool can_be_empty = false;

  TracerEnter tracer_enter;
  TracerLeave tracer_leave;
  bool verbose_trace = false;
  TracerStartOrEnd tracer_start;
  TracerStartOrEnd tracer_end;

  std::string error_message;
  bool no_ast_opt = false;
  bool no_whitespace = false; // Disable %whitespace skipping inside this rule
                              // (like a token boundary, without capturing)
  std::string ast_name; // When non-empty, AST nodes produced by this rule carry
                        // this name/tag instead of the rule's own name

  bool eoi_check = true;

  // Per-rule packrat stats (optional, for profiling)
  mutable bool collect_packrat_stats = false;
  mutable std::vector<Context::PackratStats> packrat_stats_;

private:
  friend class Reference;
  friend class ParserGenerator;

  Definition &operator=(const Definition &rhs);
  Definition &operator=(Definition &&rhs);

  void initialize_definition_ids() const {
    std::call_once(definition_ids_init_, [&]() {
      AssignIDToDefinition vis;
      holder_->accept(vis);
      if (whitespaceOpe) { whitespaceOpe->accept(vis); }
      if (wordOpe) { wordOpe->accept(vis); }
      definition_ids_.swap(vis.ids);
    });
  }

  void initialize_packrat_filter() const;

  Result parse_core(const char *s, size_t n, SemanticValues &vs, std::any &dt,
                    const char *path, Log log,
                    ErrorReporter error_reporter = nullptr) const {
    initialize_definition_ids();

    std::shared_ptr<Ope> ope = holder_;

    std::any trace_data;
    if (tracer_start) { tracer_start(trace_data); }
    auto se = scope_exit([&]() {
      if (tracer_end) { tracer_end(trace_data); }
    });

    const std::vector<int32_t> *packrat_index = nullptr;
    size_t packrat_cached_count = 0;
    if (enablePackratParsing) {
      initialize_packrat_filter();
      if (!packrat_index_.empty()) {
        packrat_index = &packrat_index_;
        packrat_cached_count = packrat_cached_count_;
      } else {
        packrat_cached_count = definition_ids_.size();
      }
    }

    Context c(path, s, n, definition_ids_.size(), whitespaceOpe, wordOpe,
              enablePackratParsing, tracer_enter, tracer_leave, trace_data,
              verbose_trace, log, error_reporter, packrat_index,
              packrat_cached_count);

    if (collect_packrat_stats) {
      packrat_stats_.resize(definition_ids_.size());
      c.packrat_stats = &packrat_stats_;
    }

    size_t i = 0;

    if (whitespaceOpe) {
      auto save_ignore_trace_state = c.ignore_trace_state;
      c.ignore_trace_state = !c.verbose_trace;
      auto se =
          scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

      auto len = whitespaceOpe->parse(s, n, vs, c, dt);
      if (fail(len)) { return Result{false, c.recovered, i, c.error_info}; }

      i = len;
    }

    auto len = ope->parse(s + i, n - i, vs, c, dt);
    auto ret = success(len);
    if (ret) {
      i += len;
      if (eoi_check) {
        if (i < n) {
          if (c.error_info.error_pos - c.s < s + i - c.s) {
            c.error_info.message_pos = s + i;
            c.error_info.message = "expected end of input";
          }
          ret = false;
        }
      }
    }
    return Result{ret, c.recovered, i, c.error_info};
  }

  std::shared_ptr<Holder> holder_;
  mutable std::once_flag is_token_init_;
  mutable bool is_token_ = false;
  mutable std::once_flag assign_id_to_definition_init_;
  mutable std::once_flag definition_ids_init_;
  mutable std::unordered_map<void *, size_t> definition_ids_;
  mutable std::once_flag packrat_filter_init_;
  mutable std::vector<int32_t> packrat_index_; // def_id -> cache slot or -1
  mutable size_t packrat_cached_count_ = 0;
};

/*
 * Implementations
 */

inline size_t parse_literal(const char *s, size_t n, SemanticValues &vs,
                            Context &c, std::any &dt, const std::string &lit,
                            std::once_flag &init_is_word, bool &is_word,
                            bool ignore_case, const std::string &lower_lit) {
  size_t i = 0;
  for (; i < lit.size(); i++) {
    if (i >= n ||
        (ignore_case
             ? (static_cast<char>(
                    c.tolower_table[static_cast<unsigned char>(s[i])]) !=
                lower_lit[i])
             : (s[i] != lit[i]))) {
      c.set_error_pos(s, lit.data());
      return static_cast<size_t>(-1);
    }
  }

  // Word check
  if (c.wordOpe) {
    auto save_ignore_trace_state = c.ignore_trace_state;
    c.ignore_trace_state = !c.verbose_trace;
    auto se =
        scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

    std::call_once(init_is_word, [&]() {
      SemanticValues dummy_vs;
      Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr,
                      nullptr, nullptr, false, nullptr);
      std::any dummy_dt;

      auto len =
          c.wordOpe->parse(lit.data(), lit.size(), dummy_vs, dummy_c, dummy_dt);
      is_word = success(len);
    });

    if (is_word) {
      SemanticValues dummy_vs;
      Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr,
                      nullptr, nullptr, false, nullptr);
      std::any dummy_dt;

      NotPredicate ope(c.wordOpe);
      auto len = ope.parse(s + i, n - i, dummy_vs, dummy_c, dummy_dt);
      if (fail(len)) {
        c.set_error_pos(s, lit.data());
        return len;
      }
      i += len;
    }
  }

  // Skip whitespace
  auto wl = c.skip_whitespace(s + i, n - i, vs, dt);
  if (fail(wl)) { return wl; }
  i += wl;

  return i;
}

inline std::pair<size_t, size_t> SemanticValues::line_info() const {
  assert(c_);
  return c_->line_info(sv_.data());
}

inline void ErrorInfo::output_log(const Log &log, const ErrorReporter &reporter,
                                  const char *s, size_t n) {
  if (message_pos) {
    if (message_pos > last_output_pos) {
      last_output_pos = message_pos;
      auto line = line_info(s, message_pos);
      std::string msg;
      auto unexpected_token = heuristic_error_token(s, n, message_pos);
      if (!unexpected_token.empty()) {
        msg = replace_all(message, "%t", unexpected_token);

        auto unexpected_char = unexpected_token.substr(
            0,
            codepoint_length(unexpected_token.data(), unexpected_token.size()));

        msg = replace_all(msg, "%c", unexpected_char);
      } else {
        msg = message;
      }
      if (reporter) {
        ErrorReport report;
        report.line = line.first;
        report.col = line.second;
        report.position = static_cast<size_t>(message_pos - s);
        report.unexpected_token = unexpected_token;
        report.message = msg;
        report.label = label;
        reporter(report);
      }
      if (log) { log(line.first, line.second, msg, label); }
    }
  } else if (error_pos) {
    if (error_pos > last_output_pos) {
      last_output_pos = error_pos;
      auto line = line_info(s, error_pos);

      ErrorReport report;
      report.line = line.first;
      report.col = line.second;
      report.position = static_cast<size_t>(error_pos - s);

      std::string msg;
      if (expected_tokens.empty()) {
        msg = "syntax error.";
      } else {
        msg = "syntax error";

        // unexpected token
        if (auto unexpected_token = heuristic_error_token(s, n, error_pos);
            !unexpected_token.empty()) {
          msg += ", unexpected '";
          msg += unexpected_token;
          msg += "'";
          report.unexpected_token = unexpected_token;
        }

        auto first_item = true;
        size_t i = 0;
        while (i < expected_tokens.size()) {
          auto [error_literal, error_rule] = expected_tokens[i];

          // Skip rules start with '_'
          if (!(error_rule && error_rule->name[0] == '_')) {
            msg += (first_item ? ", expecting " : ", ");
            if (error_literal) {
              msg += "'";
              msg += error_literal;
              msg += "'";
              report.expected_literals.emplace_back(error_literal);
            } else {
              msg += "<" + error_rule->name + ">";
              if (label.empty()) { label = error_rule->name; }
              report.expected_rules.emplace_back(error_rule->name);
            }
            first_item = false;
          }

          i++;
        }
        msg += ".";
      }
      if (reporter) {
        report.label = label;
        reporter(report);
      }
      if (log) { log(line.first, line.second, msg, label); }
    }
  }
}

inline size_t Context::skip_whitespace(const char *a_s, size_t n,
                                       SemanticValues &vs, std::any &dt) {
  if (in_token_boundary_count || !whitespaceOpe) { return 0; }
  auto save = ignore_trace_state;
  ignore_trace_state = !verbose_trace;
  auto se = scope_exit([&]() { ignore_trace_state = save; });
  return whitespaceOpe->parse(a_s, n, vs, *this, dt);
}

inline void Context::set_error_pos(const char *a_s, const char *literal) {
  if (log || error_reporter) {
    if (error_info.error_pos <= a_s) {
      if (error_info.error_pos < a_s || !error_info.keep_previous_token) {
        error_info.error_pos = a_s;
        error_info.expected_tokens.clear();
      }

      const char *error_literal = nullptr;
      const Definition *error_rule = nullptr;

      if (literal) {
        error_literal = literal;
      } else if (!rule_stack.empty()) {
        auto rule = rule_stack.back();
        auto ope = rule->get_core_operator();
        if (auto token = FindLiteralToken::token(*ope);
            token && token[0] != '\0') {
          error_literal = token;
        }
      }

      for (auto r : rule_stack) {
        error_rule = r;
        if (r->is_token()) { break; }
      }

      if (error_literal || error_rule) {
        error_info.add(error_literal, error_rule);
      }
    }
  }
}

inline void Context::trace_enter(const Ope &ope, const char *a_s, size_t n,
                                 const SemanticValues &vs, std::any &dt) {
  trace_ids.push_back(next_trace_id++);
  tracer_enter(ope, a_s, n, vs, *this, dt, trace_data);
}

inline void Context::trace_leave(const Ope &ope, const char *a_s, size_t n,
                                 const SemanticValues &vs, std::any &dt,
                                 size_t len) {
  tracer_leave(ope, a_s, n, vs, *this, dt, len, trace_data);
  trace_ids.pop_back();
}

inline bool Context::is_traceable(const Ope &ope) const {
  if (has_tracer) {
    if (ignore_trace_state) { return false; }
    return !dynamic_cast<const peg::Reference *>(&ope);
  }
  return false;
}

inline size_t Ope::parse(const char *s, size_t n, SemanticValues &vs,
                         Context &c, std::any &dt) const {
  if (c.is_traceable(*this)) {
    c.trace_enter(*this, s, n, vs, dt);
    auto len = parse_core(s, n, vs, c, dt);
    c.trace_leave(*this, s, n, vs, dt, len);
    return len;
  }
  return parse_core(s, n, vs, c, dt);
}

inline size_t Dictionary::parse_core(const char *s, size_t n,
                                     SemanticValues &vs, Context &c,
                                     std::any &dt) const {
  size_t id;
  auto i = trie_.match(s, n, id);

  if (i == 0) {
    c.set_error_pos(s);
    return static_cast<size_t>(-1);
  }

  vs.choice_count_ = trie_.items_count();
  vs.choice_ = id;

  // Word check
  if (c.wordOpe) {
    auto save_ignore_trace_state = c.ignore_trace_state;
    c.ignore_trace_state = !c.verbose_trace;
    auto se =
        scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

    {
      SemanticValues dummy_vs;
      Context dummy_c(nullptr, c.s, c.l, 0, nullptr, nullptr, false, nullptr,
                      nullptr, nullptr, false, nullptr);
      std::any dummy_dt;

      NotPredicate ope(c.wordOpe);
      auto len = ope.parse(s + i, n - i, dummy_vs, dummy_c, dummy_dt);
      if (fail(len)) {
        c.set_error_pos(s);
        return len;
      }
      i += len;
    }
  }

  // Skip whitespace
  auto wl = c.skip_whitespace(s + i, n - i, vs, dt);
  if (fail(wl)) { return wl; }
  i += wl;

  return i;
}

inline size_t LiteralString::parse_core(const char *s, size_t n,
                                        SemanticValues &vs, Context &c,
                                        std::any &dt) const {
  return parse_literal(s, n, vs, c, dt, lit_, init_is_word_, is_word_,
                       ignore_case_, lower_lit_);
}

inline size_t TokenBoundary::parse_core(const char *s, size_t n,
                                        SemanticValues &vs, Context &c,
                                        std::any &dt) const {
  auto save_ignore_trace_state = c.ignore_trace_state;
  c.ignore_trace_state = !c.verbose_trace;
  auto se =
      scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

  size_t len;
  {
    c.in_token_boundary_count++;
    auto se = scope_exit([&]() { c.in_token_boundary_count--; });
    len = ope_->parse(s, n, vs, c, dt);
  }

  if (success(len)) {
    vs.tokens.emplace_back(std::string_view(s, len));

    auto wl = c.skip_whitespace(s + len, n - len, vs, dt);
    if (fail(wl)) { return wl; }
    len += wl;
  }
  return len;
}

// Resolve `%{name}` placeholders in a custom error message against the
// named captures recorded so far ($name<...>). Unknown names resolve to an
// empty string. `%t` / `%c` are resolved later, at log-output time.
inline std::string resolve_capture_placeholders(const std::string &msg,
                                                const Context &c) {
  auto pos = msg.find("%{");
  if (pos == std::string::npos) { return msg; }

  std::string r;
  size_t i = 0;
  while (pos != std::string::npos) {
    auto end = msg.find('}', pos + 2);
    if (end == std::string::npos) { break; }
    r.append(msg, i, pos - i);
    auto name = std::string_view(msg).substr(pos + 2, end - (pos + 2));
    for (auto it = c.capture_entries.rbegin(); it != c.capture_entries.rend();
         ++it) {
      if (it->first == name) {
        // The captured span can include whitespace skipped after a token
        // boundary; trim it for display.
        auto v = std::string_view(it->second);
        while (!v.empty() &&
               std::isspace(static_cast<unsigned char>(v.back()))) {
          v.remove_suffix(1);
        }
        while (!v.empty() &&
               std::isspace(static_cast<unsigned char>(v.front()))) {
          v.remove_prefix(1);
        }
        r += v;
        break;
      }
    }
    i = end + 1;
    pos = msg.find("%{", i);
  }
  r.append(msg, i, msg.size() - i);
  return r;
}

inline size_t Holder::parse_core(const char *s, size_t n, SemanticValues &vs,
                                 Context &c, std::any &dt) const {
  if (!ope_) {
    throw std::logic_error("Uninitialized definition ope was used...");
  }

  // Macro reference. A left-recursive macro cannot take this path: it needs
  // the seed-growing below, which in turn needs its own semantic value scope
  // to memoise. Such a macro forms a scope like a plain rule does.
  if (outer_->is_macro && !outer_->is_left_recursive) {
    c.rule_stack.push_back(outer_);
    auto len = ope_->parse(s, n, vs, c, dt);
    c.rule_stack.pop_back();
    return len;
  }

  size_t len;
  std::any val;

  // Shared parse body: invokes enter/leave callbacks, parses the rule's
  // operator, handles actions/predicates/errors, and calls reduce.
  // Returns {parse_len, parse_val}.
  auto do_parse = [&]() {
    size_t parse_len;
    std::any parse_val;

    if (outer_->enter) { outer_->enter(c, s, n, dt); }
    auto &chvs = c.push_semantic_values_scope();
    auto se = scope_exit([&]() {
      c.pop_semantic_values_scope();
      if (outer_->leave) { outer_->leave(c, s, n, parse_len, parse_val, dt); }
    });

    c.rule_stack.push_back(outer_);
    if (outer_->no_whitespace) {
      {
        c.in_token_boundary_count++;
        auto se2 = scope_exit([&]() { c.in_token_boundary_count--; });
        parse_len = ope_->parse(s, n, chvs, c, dt);
      }
      if (success(parse_len)) {
        auto wl = c.skip_whitespace(s + parse_len, n - parse_len, chvs, dt);
        if (fail(wl)) {
          parse_len = wl;
        } else {
          parse_len += wl;
        }
      }
    } else {
      parse_len = ope_->parse(s, n, chvs, c, dt);
    }
    c.rule_stack.pop_back();

    if (success(parse_len)) {
      chvs.sv_ = std::string_view(s, parse_len);
      chvs.name_ = outer_->name;

      auto ope_ptr = ope_.get();
      if (ope_ptr->is_token_boundary) {
        ope_ptr = static_cast<const peg::TokenBoundary *>(ope_ptr)->ope_.get();
      }
      if (!ope_ptr->is_choice_like) {
        chvs.choice_count_ = 0;
        chvs.choice_ = 0;
      }

      std::string msg;
      std::any predicate_data;
      if (outer_->predicate) {
        if (!outer_->predicate(chvs, dt, msg, predicate_data)) {
          if ((c.log || c.error_reporter) && !msg.empty() &&
              c.error_info.message_pos < s) {
            c.error_info.message_pos = s;
            c.error_info.message = msg;
            c.error_info.label = outer_->name;
          }
          parse_len = static_cast<size_t>(-1);
        }
      }

      if (success(parse_len)) {
        if (!c.recovered) { parse_val = reduce(chvs, dt, predicate_data); }
      } else {
        if ((c.log || c.error_reporter) && !msg.empty() &&
            c.error_info.message_pos < s) {
          c.error_info.message_pos = s;
          c.error_info.message = msg;
          c.error_info.label = outer_->name;
        }
      }
    } else {
      if ((c.log || c.error_reporter) && !outer_->error_message.empty() &&
          c.error_info.message_pos < s) {
        c.error_info.message_pos = s;
        c.error_info.message =
            resolve_capture_placeholders(outer_->error_message, c);
        c.error_info.label = outer_->name;
      }
    }

    return std::make_pair(parse_len, std::move(parse_val));
  };

  if (outer_->is_left_recursive) {
    // A macro grows one seed per instantiation: Sum(D) and Sum(L) are
    // different rules as far as the memo is concerned.
    auto lr_rule = Context::LRRule(outer_, c.top_macro_inst());
    auto lr_key = Context::LRKey(lr_rule, s);

    // Check LR memo first
    auto it = c.lr_memo.find(lr_key);
    if (it != c.lr_memo.end()) {
      if (success(it->second.len)) {
        len = it->second.len;
        val = it->second.val;
      } else {
        len = static_cast<size_t>(-1);
      }
      // Record that this rule's lr_memo was accessed.
      // Any LR rule currently seeding will know we're in its cycle.
      c.lr_refs_hit.insert(lr_rule);
    } else {
      // Seed with FAIL
      c.lr_memo[lr_key] = {static_cast<size_t>(-1), {}};

      // Mark as active seed (protects our lr_memo from inner growers)
      c.lr_active_seeds.insert(lr_key);
      auto seed_guard = scope_exit([&]() { c.lr_active_seeds.erase(lr_key); });

      // Track which LR rules are referenced during our parse
      // to identify cycle members
      auto saved_refs = std::move(c.lr_refs_hit);
      c.lr_refs_hit.clear();

      // Initial parse (self-references will hit the FAIL seed)
      auto [initial_len, initial_val] = do_parse();

      // Rules whose lr_memo was hit during our parse are in our cycle.
      // If we detected cycle members, we ourselves are also part of
      // the cycle, so add self — this lets parent seeders see us as
      // a transitive cycle member.
      auto cycle_rules = c.lr_refs_hit;
      if (!cycle_rules.empty()) { cycle_rules.insert(lr_rule); }

      // Restore parent's refs and propagate cycle info upward
      c.lr_refs_hit = std::move(saved_refs);
      c.lr_refs_hit.insert(cycle_rules.begin(), cycle_rules.end());

      if (!success(initial_len)) {
        // Keep FAIL in lr_memo so we don't re-seed
        len = static_cast<size_t>(-1);
      } else {
        // Got initial seed, now grow
        len = initial_len;
        val = std::move(initial_val);
        c.lr_memo[lr_key] = {len, val};

        while (true) {
          // Clear this rule's packrat cache. A macro is never written there
          // (that cache is keyed by rule id alone, which cannot tell two
          // instantiations apart), so there is nothing to clear for one.
          if (!outer_->is_macro) { c.clear_packrat_cache(s, outer_->id); }

          // Clear lr_memo for cycle-dependent rules at this position,
          // but NOT for rules currently in their own seeding phase
          // (lr_active_seeds) — those are outer growers we must not
          // interfere with.
          for (auto memo_it = c.lr_memo.begin(); memo_it != c.lr_memo.end();) {
            if (memo_it->first.second == s && memo_it->first.first != lr_rule &&
                cycle_rules.count(memo_it->first.first) &&
                !c.lr_active_seeds.count(memo_it->first)) {
              memo_it = c.lr_memo.erase(memo_it);
            } else {
              ++memo_it;
            }
          }

          auto [new_len, new_val] = do_parse();

          if (!success(new_len) || new_len <= len) {
            break; // No improvement, done growing
          }

          len = new_len;
          val = std::move(new_val);
          c.lr_memo[lr_key] = {len, val};
        }
      }

      // Write final result to packrat cache (lr_memo entry is kept as
      // the primary lookup for LR rules at this position)
      if (success(len) && !outer_->is_macro) {
        c.write_packrat_cache(s, outer_->id, len, val);
      }
    }
  } else {
    if (c.enablePackratParsing) {
      // Packrat cache acts as re-entry guard (pre-registered as
      // failure before fn is called).
      c.packrat(s, outer_->id, len, val, [&](std::any &a_val) {
        auto [parse_len, parse_val] = do_parse();
        len = parse_len;
        if (success(len)) { a_val = std::move(parse_val); }
      });
    } else {
      // Without packrat, use lr_memo as re-entry guard to prevent
      // stack overflow from undetected left recursion.
      auto guard_key = Context::LRKey({outer_, c.top_macro_inst()}, s);
      if (c.lr_memo.count(guard_key)) {
        len = static_cast<size_t>(-1);
      } else {
        c.lr_memo[guard_key] = {static_cast<size_t>(-1), {}};
        auto [parse_len, parse_val] = do_parse();
        len = parse_len;
        val = std::move(parse_val);
        c.lr_memo.erase(guard_key);
      }
    }
  }

  if (success(len)) {
    if (!outer_->ignoreSemanticValue) {
      vs.emplace_back(std::move(val));
      vs.tags.emplace_back(str2tag(outer_->name));
    }
  }

  return len;
}

inline std::any Holder::reduce(SemanticValues &vs, std::any &dt,
                               const std::any &predicate_data) const {
  if (outer_->action && !outer_->disable_action) {
    return outer_->action(vs, dt, predicate_data);
  } else if (vs.empty()) {
    return std::any();
  } else {
    return std::move(vs.front());
  }
}

inline const std::string &Holder::name() const { return outer_->name; }

inline const std::string &Holder::trace_name() const {
  std::call_once(trace_name_init_,
                 [this]() { trace_name_ = "[" + outer_->name + "]"; });
  return trace_name_;
}

// Key a macro instantiation by what each argument denotes rather than by the
// node that spells it: `M(N)` written at two call sites builds two Reference
// nodes for the same rule N, and those are the same instantiation.
inline std::vector<const void *>
macro_inst_key(const Definition *def,
               const std::vector<std::shared_ptr<Ope>> &args) {
  std::vector<const void *> key;
  key.reserve(args.size() + 1);
  key.push_back(def);
  for (const auto &arg : args) {
    auto ref = dynamic_cast<Reference *>(arg.get());
    key.push_back(ref && ref->rule_ ? static_cast<const void *>(ref->rule_)
                                    : static_cast<const void *>(arg.get()));
  }
  return key;
}

inline size_t Reference::parse_core(const char *s, size_t n, SemanticValues &vs,
                                    Context &c, std::any &dt) const {
  auto save_ignore_trace_state = c.ignore_trace_state;
  if (rule_ && rule_->ignoreSemanticValue) {
    c.ignore_trace_state = !c.verbose_trace;
  }
  auto se =
      scope_exit([&]() { c.ignore_trace_state = save_ignore_trace_state; });

  if (rule_) {
    // Reference rule
    if (rule_->is_macro) {
      // Macro
      FindReference vis(c.top_args(), c.rule_stack.back()->params);

      // Collect arguments
      std::vector<std::shared_ptr<Ope>> args;
      for (const auto &arg : args_) {
        arg->accept(vis);
        args.emplace_back(std::move(vis.found_ope));
      }

      auto inst = rule_->is_left_recursive
                      ? c.intern_macro_inst(macro_inst_key(rule_, args))
                      : 0;
      c.push_args(std::move(args), inst);
      auto se = scope_exit([&]() { c.pop_args(); });
      return rule_->holder_->parse(s, n, vs, c, dt);
    } else {
      // Definition
      c.push_args(std::vector<std::shared_ptr<Ope>>());
      auto se2 = scope_exit([&]() { c.pop_args(); });
      return rule_->holder_->parse(s, n, vs, c, dt);
    }
  } else {
    // Reference parameter in macro
    const auto &args = c.top_args();
    return args[iarg_]->parse(s, n, vs, c, dt);
  }
}

inline std::shared_ptr<Ope> Reference::get_core_operator() const {
  return rule_->holder_;
}

inline size_t BackReference::parse_core(const char *s, size_t n,
                                        SemanticValues &vs, Context &c,
                                        std::any &dt) const {
  for (auto it = c.capture_entries.rbegin(); it != c.capture_entries.rend();
       ++it) {
    if (it->first == name_) {
      const auto &lit = it->second;
      std::once_flag init_is_word;
      auto is_word = false;
      static const std::string empty;
      return parse_literal(s, n, vs, c, dt, lit, init_is_word, is_word, false,
                           empty);
    }
  }

  c.error_info.message_pos = s;
  c.error_info.message = "undefined back reference '$" + name_ + "'...";
  return static_cast<size_t>(-1);
}

inline Definition &
PrecedenceClimbing::get_reference_for_binop(Context &c) const {
  if (rule_.is_macro) {
    // Reference parameter in macro
    const auto &args = c.top_args();
    auto iarg = dynamic_cast<Reference &>(*binop_).iarg_;
    auto arg = args[iarg];
    return *dynamic_cast<Reference &>(*arg).rule_;
  }

  return *dynamic_cast<Reference &>(*binop_).rule_;
}

inline size_t PrecedenceClimbing::parse_expression(const char *s, size_t n,
                                                   SemanticValues &vs,
                                                   Context &c, std::any &dt,
                                                   size_t min_prec) const {
  auto len = atom_->parse(s, n, vs, c, dt);
  if (fail(len)) { return len; }

  std::string tok;
  auto &rule = get_reference_for_binop(c);
  auto action = std::move(rule.action);

  rule.action = [&](SemanticValues &vs2, std::any &dt2,
                    const std::any &predicate_data2) {
    tok = vs2.token();
    if (action) {
      return action(vs2, dt2, predicate_data2);
    } else if (!vs2.empty()) {
      return vs2[0];
    }
    return std::any();
  };
  auto action_se = scope_exit([&]() { rule.action = std::move(action); });

  auto i = len;
  while (i < n) {
    std::vector<std::any> save_values(vs.begin(), vs.end());
    auto save_tokens = vs.tokens;

    auto chvs = c.push_semantic_values_scope();
    auto chlen = binop_->parse(s + i, n - i, chvs, c, dt);
    c.pop_semantic_values_scope();

    if (fail(chlen)) { break; }

    auto it = info_.find(tok);
    if (it == info_.end()) { break; }

    auto level = std::get<0>(it->second);
    auto assoc = std::get<1>(it->second);

    if (level < min_prec) { break; }

    vs.emplace_back(std::move(chvs[0]));
    i += chlen;

    auto next_min_prec = level;
    if (assoc == 'L') { next_min_prec = level + 1; }

    chvs = c.push_semantic_values_scope();
    chlen = parse_expression(s + i, n - i, chvs, c, dt, next_min_prec);
    c.pop_semantic_values_scope();

    if (fail(chlen)) {
      vs.assign(save_values.begin(), save_values.end());
      vs.tokens = save_tokens;
      i = chlen;
      break;
    }

    vs.emplace_back(std::move(chvs[0]));
    i += chlen;

    std::any val;
    if (rule_.action) {
      vs.sv_ = std::string_view(s, i);
      static const std::any empty_predicate_data;
      val = rule_.action(vs, dt, empty_predicate_data);
    } else if (!vs.empty()) {
      val = vs[0];
    }
    vs.clear();
    vs.emplace_back(std::move(val));
  }

  return i;
}

inline size_t Recovery::parse_core(const char *s, size_t n,
                                   SemanticValues & /*vs*/, Context &c,
                                   std::any & /*dt*/) const {
  const auto &rule = dynamic_cast<Reference &>(*ope_);

  // Custom error message
  if (c.log || c.error_reporter) {
    auto label = dynamic_cast<Reference *>(rule.args_[0].get());
    if (label && !label->rule_->error_message.empty()) {
      c.error_info.message_pos = s;
      c.error_info.message =
          resolve_capture_placeholders(label->rule_->error_message, c);
      c.error_info.label = label->rule_->name;
    }
  }

  // Recovery
  auto len = static_cast<size_t>(-1);
  {
    auto save_log = c.log;
    auto save_reporter = c.error_reporter;
    c.log = nullptr;
    c.error_reporter = nullptr;
    auto se = scope_exit([&]() {
      c.log = save_log;
      c.error_reporter = save_reporter;
    });

    SemanticValues dummy_vs;
    std::any dummy_dt;

    len = rule.parse(s, n, dummy_vs, c, dummy_dt);
  }

  if (success(len)) {
    c.recovered = true;

    if (c.log || c.error_reporter) {
      c.error_info.output_log(c.log, c.error_reporter, c.s, c.l);
      c.error_info.clear();
    }
  }

  // Cut
  if (!c.cut_stack.empty()) {
    c.cut_stack.back() = true;

    if (c.cut_stack.size() == 1) {
      // TODO: Remove unneeded entries in packrat memoise table
    }
  }

  return len;
}

inline void Sequence::accept(Visitor &v) { v.visit(*this); }
inline void PrioritizedChoice::accept(Visitor &v) { v.visit(*this); }
inline void Repetition::accept(Visitor &v) { v.visit(*this); }
inline void AndPredicate::accept(Visitor &v) { v.visit(*this); }
inline void NotPredicate::accept(Visitor &v) { v.visit(*this); }
inline void Dictionary::accept(Visitor &v) { v.visit(*this); }
inline void LiteralString::accept(Visitor &v) { v.visit(*this); }
inline void CharacterClass::accept(Visitor &v) { v.visit(*this); }
inline void Character::accept(Visitor &v) { v.visit(*this); }
inline void AnyCharacter::accept(Visitor &v) { v.visit(*this); }
inline void CaptureScope::accept(Visitor &v) { v.visit(*this); }
inline void Capture::accept(Visitor &v) { v.visit(*this); }
inline void TokenBoundary::accept(Visitor &v) { v.visit(*this); }
inline void Ignore::accept(Visitor &v) { v.visit(*this); }
inline void User::accept(Visitor &v) { v.visit(*this); }
inline void WeakHolder::accept(Visitor &v) { v.visit(*this); }
inline void Holder::accept(Visitor &v) { v.visit(*this); }
inline void Reference::accept(Visitor &v) { v.visit(*this); }
inline void Whitespace::accept(Visitor &v) { v.visit(*this); }
inline void BackReference::accept(Visitor &v) { v.visit(*this); }
inline void PrecedenceClimbing::accept(Visitor &v) { v.visit(*this); }
inline void Recovery::accept(Visitor &v) { v.visit(*this); }
inline void Cut::accept(Visitor &v) { v.visit(*this); }

inline void AssignIDToDefinition::visit(Holder &ope) {
  auto p = static_cast<void *>(ope.outer_);
  if (ids.count(p)) { return; }
  auto id = ids.size();
  ids[p] = id;
  ope.outer_->id = id;
  ope.ope_->accept(*this);
}

inline void AssignIDToDefinition::visit(Reference &ope) {
  if (ope.rule_) {
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
    ope.rule_->accept(*this);
  }
}

inline void AssignIDToDefinition::visit(PrecedenceClimbing &ope) {
  ope.atom_->accept(*this);
  ope.binop_->accept(*this);
}

inline void TokenChecker::visit(Reference &ope) {
  if (ope.is_macro_) {
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  } else {
    has_rule_ = true;
  }
}

inline void FindLiteralToken::visit(Reference &ope) {
  if (ope.is_macro_) {
    ope.rule_->accept(*this);
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  }
}

inline void ComputeCanBeEmpty::visit(Reference &ope) {
  result = ope.rule_ && ope.rule_->can_be_empty;
}

inline void DetectLeftRecursion::visit(Reference &ope) {
  // Macro parameter reference: what it denotes lives in an enclosing
  // instantiation (e.g. B(X) <- C(X) where X is itself a param ref).
  auto param = !ope.rule_ && !macro_args_stack_.empty()
                   ? resolve_macro_arg(ope.iarg_)
                   : ResolvedArg{};

  if (ope.name_ == name_) {
    error_s = ope.s_;
  } else if (param.ope) {
    visit_in_defining_scope(param);
    if (done_ == false) { return; }
  } else if (ope.is_macro_ &&
             macro_args_stack_.size() >= max_macro_inst_depth) {
    // Unbounded instantiation chain; stop descending.
  } else if (ope.rule_ &&
             refs_
                 .emplace(ope.rule_, ope.is_macro_ ? intern_macro_inst(ope) : 0)
                 .second) {
    if (ope.is_macro_) { macro_args_stack_.push_back(&ope.args_); }
    ope.rule_->accept(*this);
    if (ope.is_macro_) { macro_args_stack_.pop_back(); }
    if (done_ == false) { return; }
  }
  // If the referenced rule can match empty, don't mark as done —
  // the sequence may continue past this element to find LR.
  if (!ope.rule_ && !macro_args_stack_.empty()) {
    if (param.ope) {
      // ComputeCanBeEmpty never consults the frame stack, so the scope it
      // runs in cannot matter.
      ComputeCanBeEmpty cbe;
      param.ope->accept(cbe);
      done_ = !cbe.result;
    } else {
      done_ = true;
    }
  } else {
    done_ = !(ope.rule_ && ope.rule_->can_be_empty);
  }
}

inline size_t DetectLeftRecursion::intern_macro_inst(const Reference &ope) {
  // Resolve bare parameter references to what the enclosing instantiation was
  // given, so a macro passing its own parameter through interns to the same
  // instantiation instead of a fresh one at every nesting level.
  std::vector<std::shared_ptr<Ope>> args;
  args.reserve(ope.args_.size());
  for (const auto &arg : ope.args_) {
    auto ref = dynamic_cast<Reference *>(arg.get());
    auto resolved = ref && !ref->rule_ && !macro_args_stack_.empty()
                        ? resolve_macro_arg(ref->iarg_).ope
                        : nullptr;
    args.push_back(resolved ? resolved : arg);
  }
  auto [it, inserted] = macro_inst_ids_.emplace(macro_inst_key(ope.rule_, args),
                                                next_macro_inst_);
  if (inserted) { next_macro_inst_++; }
  return it->second;
}

inline void
DetectLeftRecursion::visit_in_defining_scope(const ResolvedArg &arg) {
  // The frames below the one holding it are the scope it was written in.
  // `W(X) <- Y(X / 'x')` passes Y an argument whose own `X` means W's
  // parameter, not Y's -- leaving Y's frame visible would resolve that `X`
  // right back to `X / 'x'`, forever.
  auto saved = macro_args_stack_;
  auto se = scope_exit([&]() { macro_args_stack_ = std::move(saved); });
  macro_args_stack_.resize(arg.depth);
  arg.ope->accept(*this);
}

inline DetectLeftRecursion::ResolvedArg
DetectLeftRecursion::resolve_macro_arg(size_t iarg) const {
  for (int i = static_cast<int>(macro_args_stack_.size()) - 1; i >= 0; i--) {
    auto &args = *macro_args_stack_[i];
    if (iarg >= args.size()) { return {}; }
    auto ref = dynamic_cast<Reference *>(args[iarg].get());
    if (ref && !ref->rule_) {
      // Another param ref — resolve using parent level's args
      iarg = ref->iarg_;
      continue;
    }
    return {args[iarg], static_cast<size_t>(i)};
  }
  return {};
}

inline void HasEmptyElement::visit(Sequence &ope) {
  auto save_is_empty = false;
  const char *save_error_s = nullptr;
  std::string save_error_name;

  auto it = ope.opes_.begin();
  while (it != ope.opes_.end()) {
    (*it)->accept(*this);
    if (!is_empty) {
      ++it;
      while (it != ope.opes_.end()) {
        DetectInfiniteLoop vis(refs_, has_error_cache_);
        (*it)->accept(vis);
        if (vis.has_error) {
          is_empty = true;
          error_s = vis.error_s;
          error_name = vis.error_name;
        }
        ++it;
      }
      return;
    }

    save_is_empty = is_empty;
    save_error_s = error_s;
    save_error_name = error_name;

    is_empty = false;
    error_name.clear();
    ++it;
  }

  is_empty = save_is_empty;
  error_s = save_error_s;
  error_name = save_error_name;
}

inline void HasEmptyElement::visit(Reference &ope) {
  auto it = std::find_if(refs_.begin(), refs_.end(),
                         [&](const std::pair<const char *, std::string> &ref) {
                           return ope.name_ == ref.second;
                         });
  if (it != refs_.end()) { return; }

  if (ope.rule_) {
    refs_.emplace_back(ope.s_, ope.name_);
    ope.rule_->accept(*this);
    refs_.pop_back();
  }
}

inline void DetectInfiniteLoop::visit(Reference &ope) {
  auto it = std::find_if(refs_.begin(), refs_.end(),
                         [&](const std::pair<const char *, std::string> &ref) {
                           return ope.name_ == ref.second;
                         });
  if (it != refs_.end()) { return; }

  if (ope.rule_) {
    auto it = has_error_cache_.find(ope.name_);
    if (it != has_error_cache_.end()) {
      has_error = it->second;
    } else {
      refs_.emplace_back(ope.s_, ope.name_);
      ope.rule_->accept(*this);
      refs_.pop_back();
      has_error_cache_[ope.name_] = has_error;
    }
  }

  if (ope.is_macro_) {
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  }
}

inline void ReferenceChecker::visit(Reference &ope) {
  auto it = std::find(params_.begin(), params_.end(), ope.name_);
  if (it != params_.end()) { return; }

  if (!grammar_.count(ope.name_)) {
    error_s[ope.name_] = ope.s_;
    error_message[ope.name_] = "'" + ope.name_ + "' is not defined.";
  } else {
    if (!referenced.count(ope.name_)) { referenced.insert(ope.name_); }
    const auto &rule = grammar_.at(ope.name_);
    if (rule.is_macro) {
      if (!ope.is_macro_ || ope.args_.size() != rule.params.size()) {
        error_s[ope.name_] = ope.s_;
        error_message[ope.name_] = "incorrect number of arguments.";
      }
    } else if (ope.is_macro_) {
      error_s[ope.name_] = ope.s_;
      error_message[ope.name_] = "'" + ope.name_ + "' is not macro.";
    }
    for (const auto &arg : ope.args_) {
      arg->accept(*this);
    }
  }
}

inline void ComputeFirstSet::visit(Reference &ope) {
  if (!ope.rule_) {
    // Macro parameter reference — can't predict what it will match
    result_.any_char = true;
    return;
  }

  auto it = cache_.find(ope.rule_);
  FirstSet computed;
  const FirstSet *rule_fs;
  if (it != cache_.end()) {
    rule_fs = &it->second;
  } else {
    if (!refs_.insert(ope.rule_).second) {
      cycle_count_++; // cycle / left recursion
      return;
    }
    auto save = std::exchange(result_, FirstSet{});
    auto saved_cycle_count = cycle_count_;
    ope.rule_->accept(*this);
    computed = std::move(result_);
    result_ = std::move(save);
    refs_.erase(ope.rule_);
    if (cycle_count_ == saved_cycle_count) {
      // Cycle-free: cached value is complete and safe to reuse.
      it = cache_.try_emplace(ope.rule_, std::move(computed)).first;
      rule_fs = &it->second;
    } else {
      // Cycle was hit during this rule's computation — its result may be
      // missing contributions from rules that were on the call stack.
      // Use the value here but do not cache it for other call contexts.
      rule_fs = &computed;
    }
  }

  result_.merge(*rule_fs);
  if (!result_.first_literal) {
    result_.first_literal = rule_fs->first_literal;
  }
  if (!result_.first_rule) {
    result_.first_rule = rule_fs->first_rule
                             ? rule_fs->first_rule
                             : (ope.rule_->is_token() ? ope.rule_ : nullptr);
  }
}

inline void SetupFirstSets::visit(Reference &ope) {
  if (!ope.rule_) { return; }
  ope.rule_->accept(*this); // re-entry is guarded at the rule's Holder
}

// Guard rule setup by Definition so a SetupFirstSets shared across all rules
// visits each rule's body at most once for the whole grammar. Without this the
// per-rule setup re-walks every reachable rule once per referencing rule, which
// is O(N^2) for grammars with dense cross-references.
inline void SetupFirstSets::visit(Holder &ope) {
  if (!visited_rules_.insert(ope.outer_).second) { return; }
  ope.ope_->accept(*this);
}

inline void SetupFirstSets::visit(Sequence &ope) {
  ope.kw_guard_.reset();
  setup_keyword_guarded_identifier(ope);
  for (const auto &op : ope.opes_) {
    op->accept(*this);
  }
}

inline void SetupFirstSets::setup_keyword_guarded_identifier(Sequence &seq) {
  // Detect pattern: NotPredicate(Reference→PrioritizedChoice<literals>)
  //                 TokenBoundary(Sequence[CharacterClass,
  //                 Repetition(CharacterClass)])
  // This is the pattern used by: PlainIdentifier <- !ReservedKeyword
  // <[a-z_]i[a-z0-9_]i*>
  if (seq.opes_.size() != 2) { return; }

  // Child 0 must be NotPredicate
  auto *not_pred = dynamic_cast<NotPredicate *>(seq.opes_[0].get());
  if (!not_pred) { return; }

  // NotPredicate's child must be Reference to a rule
  auto *ref = dynamic_cast<Reference *>(not_pred->ope_.get());
  if (!ref || !ref->rule_) { return; }

  // The referenced rule's inner operator (Holder) must contain
  // PrioritizedChoice
  auto *holder = dynamic_cast<Holder *>(ref->get_core_operator().get());
  if (!holder) { return; }
  auto *choice = dynamic_cast<PrioritizedChoice *>(holder->ope_.get());
  if (!choice) { return; }

  // Extract keywords from PrioritizedChoice alternatives
  std::vector<std::string> exact_keywords;
  std::vector<std::string> prefix_keywords;

  for (const auto &alt : choice->opes_) {
    auto *lit = dynamic_cast<LiteralString *>(alt.get());
    if (lit) {
      if (!lit->ignore_case_) { return; }
      exact_keywords.push_back(to_lower(lit->lit_));
      continue;
    }
    // Check for compound keyword (Sequence of LiteralStrings)
    auto *sub_seq = dynamic_cast<Sequence *>(alt.get());
    if (sub_seq && !sub_seq->opes_.empty()) {
      auto *first_lit = dynamic_cast<LiteralString *>(sub_seq->opes_[0].get());
      if (first_lit) {
        auto all_ignore_case_lits =
            std::all_of(sub_seq->opes_.begin(), sub_seq->opes_.end(),
                        [](const auto &child) {
                          auto *l = dynamic_cast<LiteralString *>(child.get());
                          return l && l->ignore_case_;
                        });
        if (all_ignore_case_lits) {
          prefix_keywords.push_back(to_lower(first_lit->lit_));
          continue;
        }
      }
    }
    // Unrecognized alternative — bail out
    return;
  }

  if (exact_keywords.empty()) { return; }

  // Child 1 must be TokenBoundary
  auto *tb = dynamic_cast<TokenBoundary *>(seq.opes_[1].get());
  if (!tb) { return; }

  // TokenBoundary content: Sequence[CharacterClass, Repetition(CharacterClass)]
  // or just CharacterClass (single char identifier)
  CharacterClass *first_cc = nullptr;
  CharacterClass *rest_cc = nullptr;

  auto *inner_seq = dynamic_cast<Sequence *>(tb->ope_.get());
  if (inner_seq && inner_seq->opes_.size() == 2) {
    first_cc = dynamic_cast<CharacterClass *>(inner_seq->opes_[0].get());
    auto *rep = dynamic_cast<Repetition *>(inner_seq->opes_[1].get());
    if (rep) { rest_cc = dynamic_cast<CharacterClass *>(rep->ope_.get()); }
  }

  if (!first_cc || !rest_cc) { return; }
  if (!first_cc->is_ascii_only() || !rest_cc->is_ascii_only()) { return; }

  // All conditions met — set up the fast path
  auto kw = std::make_unique<KeywordGuardData>();
  kw->identifier_first = first_cc->ascii_bitset();
  kw->identifier_rest = rest_cc->ascii_bitset();

  // Compute keyword length range for early-out in hot path
  size_t min_len = SIZE_MAX, max_len = 0;
  for (const auto &k : exact_keywords) {
    min_len = std::min(min_len, k.size());
    max_len = std::max(max_len, k.size());
  }
  for (const auto &k : prefix_keywords) {
    min_len = std::min(min_len, k.size());
    max_len = std::max(max_len, k.size());
  }
  kw->min_keyword_len = min_len;
  kw->max_keyword_len = max_len;

  kw->exact_keywords = std::move(exact_keywords);
  kw->prefix_keywords = std::move(prefix_keywords);
  seq.kw_guard_ = std::move(kw);
}

// Compute which rules benefit from packrat memoization.
// A rule benefits if it's reachable from 2+ alternatives of the same
// PrioritizedChoice (backtracking will re-visit it at the same position).
inline void Definition::initialize_packrat_filter() const {
  std::call_once(packrat_filter_init_, [&]() {
    auto def_count = definition_ids_.size();
    if (def_count == 0) { return; }

    // Collect rule IDs that can be invoked at the *same start position* as
    // the given Ope subtree (leftmost reachability). A packrat cache hit
    // requires the same rule to be queried twice at the same position, and
    // in a PEG that only happens when alternatives of a choice share a
    // leftmost prefix — rules reachable only past a consuming element can
    // never be re-queried by a sibling alternative.
    struct CollectLeftmostRules : public TraversalVisitor {
      using TraversalVisitor::visit;
      std::vector<bool> reachable; // indexed by def_id
      std::vector<bool>
          visited_rules; // indexed by def_id; guards Holder cycles

      CollectLeftmostRules(size_t n)
          : reachable(n, false), visited_rules(n, false) {}

      // Collect from the position element `from` starts at: element `from`
      // itself, plus what follows for as long as elements can match empty —
      // only up to (and including) the first one that must consume input.
      void collect(const std::vector<std::shared_ptr<Ope>> &opes, size_t from) {
        for (auto i = from; i < opes.size(); i++) {
          opes[i]->accept(*this);
          ComputeCanBeEmpty empty_vis;
          opes[i]->accept(empty_vis);
          if (!empty_vis.result) { break; }
        }
      }

      void visit(Sequence &ope) override { collect(ope.opes_, 0); }
      void visit(Holder &ope) override {
        auto id = ope.outer_->id;
        if (id < reachable.size()) {
          reachable[id] = true;

          // Grammars built directly via the combinator API embed rules through
          // WeakHolder rather than Reference, so a recursive rule forms a
          // Holder cycle with no Reference to break it. Guard re-entry to avoid
          // infinite recursion (reachability is monotone, so revisiting a rule
          // we have already traversed adds nothing).
          if (visited_rules[id]) { return; }
          visited_rules[id] = true;
        }
        ope.ope_->accept(*this);
      }
      void visit(Reference &ope) override {
        if (ope.rule_ && ope.rule_->id < reachable.size() &&
            !reachable[ope.rule_->id]) {
          reachable[ope.rule_->id] = true;
          ope.rule_->accept(*this);
        }
      }
    };

    // Find rules that benefit: queried by 2+ alternatives of the same choice
    // at the same position
    std::vector<bool> benefits(def_count, false);

    struct FindBacktrackRules : public TraversalVisitor {
      using TraversalVisitor::visit;
      std::vector<bool> &benefits;
      size_t def_count;
      std::vector<bool> visited_rules; // indexed by def_id

      FindBacktrackRules(std::vector<bool> &b, size_t n)
          : benefits(b), def_count(n), visited_rules(n, false) {}

      using Elements = std::vector<std::shared_ptr<Ope>>;

      // An alternative's top-level elements, so a shared prefix can be walked
      // element by element. By value: this runs once per grammar.
      static Elements elements_of(const std::shared_ptr<Ope> &alt) {
        if (auto *seq = dynamic_cast<Sequence *>(alt.get())) {
          return seq->opes_;
        }
        return {alt};
      }

      // `group` holds alternatives that agree on their first `k` elements, so
      // every one of them reaches element k at the same input position — that
      // is exactly when a packrat cache entry can hit. k == 0 is the plain
      // "alternatives of one choice" case; deeper k is what a shared prefix
      // like `'(' _ PATTERN _ ',' _` hides.
      void mark_aligned(const std::vector<Elements> &group, size_t k) {
        if (group.size() < 2) { return; }

        std::vector<std::vector<bool>> reachable;
        reachable.reserve(group.size());
        for (const auto &seq : group) {
          CollectLeftmostRules clr(def_count);
          clr.collect(seq, k);
          reachable.push_back(std::move(clr.reachable));
        }
        for (size_t id = 0; id < def_count; id++) {
          size_t count = 0;
          for (const auto &alt : reachable) {
            if (alt[id]) { count++; }
          }
          if (count >= 2) { benefits[id] = true; }
        }

        // Only alternatives that also agree on element k stay aligned past it.
        std::map<std::string, std::vector<Elements>> aligned;
        for (const auto &seq : group) {
          if (k < seq.size()) {
            aligned[OpeSignature::get(*seq[k])].push_back(seq);
          }
        }
        for (const auto &[sig, sub] : aligned) {
          mark_aligned(sub, k + 1);
        }
      }

      void visit(PrioritizedChoice &ope) override {
        std::vector<Elements> group;
        group.reserve(ope.opes_.size());
        for (const auto &op : ope.opes_) {
          group.push_back(elements_of(op));
        }
        mark_aligned(group, 0);

        // Recurse into alternatives
        for (auto &op : ope.opes_) {
          op->accept(*this);
        }
      }
      void visit(Holder &ope) override {
        auto id = ope.outer_->id;
        if (id < visited_rules.size() && !visited_rules[id]) {
          visited_rules[id] = true;
          ope.ope_->accept(*this);
        }
      }
      void visit(Reference &ope) override {
        if (ope.rule_) { ope.rule_->accept(*this); }
      }
    };

    FindBacktrackRules finder(benefits, def_count);
    holder_->accept(finder);
    if (whitespaceOpe) { whitespaceOpe->accept(finder); }
    if (wordOpe) { wordOpe->accept(finder); }

    // Left-recursive rules read and write the packrat cache directly during
    // seed-growing, so they must stay in the cached set. Macros are the
    // exception: they use lr_memo only, keyed by instantiation.
    for (const auto &[ptr, id] : definition_ids_) {
      auto *def = static_cast<Definition *>(ptr);
      if (def->is_left_recursive && !def->is_macro && id < def_count) {
        benefits[id] = true;
      }
    }

    // Compact index: def_id -> slot in the cache tables (-1 = guard only)
    packrat_index_.assign(def_count, -1);
    int32_t k = 0;
    for (size_t id = 0; id < def_count; id++) {
      if (benefits[id]) { packrat_index_[id] = k++; }
    }
    packrat_cached_count_ = static_cast<size_t>(k);
  });
}

inline void LinkReferences::visit(Reference &ope) {
  // Check if the reference is a macro parameter
  auto found_param = false;
  for (size_t i = 0; i < params_.size(); i++) {
    const auto &param = params_[i];
    if (param == ope.name_) {
      ope.iarg_ = i;
      found_param = true;
      break;
    }
  }

  // Check if the reference is a definition rule
  if (!found_param && grammar_.count(ope.name_)) {
    auto &rule = grammar_.at(ope.name_);
    ope.rule_ = &rule;
  }

  for (const auto &arg : ope.args_) {
    arg->accept(*this);
  }
}

inline void FindReference::visit(Reference &ope) {
  for (size_t i = 0; i < args_.size(); i++) {
    const auto &name = params_[i];
    if (name == ope.name_) {
      found_ope = args_[i];
      return;
    }
  }
  found_ope = ope.shared_from_this();
}

/*-----------------------------------------------------------------------------
 *  Grammar serialization
 *
 *  Serialize a compiled Grammar (the operator tree) to a byte blob and back,
 *  letting an application skip the meta-parse on startup by embedding a
 *  prebuilt blob. Structure only: semantic callbacks (actions / enter / leave /
 *  predicate, attached by enable_ast() etc.) are NOT serialized and must be
 *  re-applied after deserialize. References resolve by name (no pointer fixup);
 *  first-sets and keyword guards are recomputed on load (O(N)). The
 *  `precedence` instruction is supported (its operator table is structural).
 *  Grammars using the `User` operator or a Capture with a match action are
 *  rejected. The blob is specific to this peglib version's layout.
 *---------------------------------------------------------------------------*/

struct GrammarBlob {
  enum Tag : uint8_t {
    T_Sequence,
    T_Choice,
    T_Repetition,
    T_And,
    T_Not,
    T_Dictionary,
    T_Literal,
    T_CharClass,
    T_Char,
    T_AnyChar,
    T_CaptureScope,
    T_Capture,
    T_TokenBoundary,
    T_Ignore,
    T_BackRef,
    T_Reference,
    T_Whitespace,
    T_Recovery,
    T_Cut,
    T_PrecedenceClimbing,
    T_Null
  };

  struct Writer {
    std::vector<uint8_t> b;
    void u8(uint8_t v) { b.push_back(v); }
    void u32(uint32_t v) {
      for (int i = 0; i < 4; i++)
        b.push_back((v >> (8 * i)) & 0xff);
    }
    void u64(uint64_t v) {
      for (int i = 0; i < 8; i++)
        b.push_back((v >> (8 * i)) & 0xff);
    }
    void str(const std::string &s) {
      u32((uint32_t)s.size());
      b.insert(b.end(), s.begin(), s.end());
    }
  };

  static void write_ope(Writer &w, const std::shared_ptr<Ope> &o) {
    if (!o) {
      w.u8(T_Null);
      return;
    }
    Ope *p = o.get();
    if (auto x = dynamic_cast<Sequence *>(p)) {
      w.u8(T_Sequence);
      w.u32((uint32_t)x->opes_.size());
      for (auto &c : x->opes_)
        write_ope(w, c);
    } else if (auto x = dynamic_cast<PrioritizedChoice *>(p)) {
      w.u8(T_Choice);
      w.u8(x->for_label_ ? 1 : 0);
      w.u32((uint32_t)x->opes_.size());
      for (auto &c : x->opes_)
        write_ope(w, c);
    } else if (auto x = dynamic_cast<Repetition *>(p)) {
      w.u8(T_Repetition);
      w.u64(x->min_);
      w.u64(x->max_);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<AndPredicate *>(p)) {
      w.u8(T_And);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<NotPredicate *>(p)) {
      w.u8(T_Not);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Dictionary *>(p)) {
      w.u8(T_Dictionary);
      w.u8(x->trie_.ignore_case_ ? 1 : 0);
      // Recover words in their original choice-index order. The Trie stores
      // each full word's id (its index in the constructor vector), which
      // parse_core reports as vs.choice(). Iterating dic_ directly yields
      // sorted key order and would renumber the choices, so place each word at
      // its id.
      std::vector<std::string> words(x->trie_.items_count());
      for (auto &kv : x->trie_.dic_)
        if (kv.second.match && kv.second.id < words.size())
          words[kv.second.id] = kv.first;
      w.u32((uint32_t)words.size());
      for (auto &s : words)
        w.str(s);
    } else if (auto x = dynamic_cast<LiteralString *>(p)) {
      w.u8(T_Literal);
      w.u8(x->ignore_case_ ? 1 : 0);
      w.str(x->lit_);
    } else if (auto x = dynamic_cast<CharacterClass *>(p)) {
      w.u8(T_CharClass);
      w.u8(x->negated_ ? 1 : 0);
      w.u8(x->ignore_case_ ? 1 : 0);
      w.u32((uint32_t)x->ranges_.size());
      for (auto &r : x->ranges_) {
        w.u32((uint32_t)r.first);
        w.u32((uint32_t)r.second);
      }
    } else if (auto x = dynamic_cast<Character *>(p)) {
      w.u8(T_Char);
      w.u32((uint32_t)x->ch_);
    } else if (dynamic_cast<AnyCharacter *>(p)) {
      w.u8(T_AnyChar);
    } else if (auto x = dynamic_cast<CaptureScope *>(p)) {
      w.u8(T_CaptureScope);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Capture *>(p)) {
      if (x->match_action_) {
        throw std::runtime_error(
            "GrammarBlob: Capture with a match action is not serializable");
      }
      w.u8(T_Capture);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<TokenBoundary *>(p)) {
      w.u8(T_TokenBoundary);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Ignore *>(p)) {
      w.u8(T_Ignore);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<BackReference *>(p)) {
      w.u8(T_BackRef);
      w.str(x->name_);
    } else if (auto x = dynamic_cast<Reference *>(p)) {
      w.u8(T_Reference);
      w.u8(x->is_macro_ ? 1 : 0);
      w.str(x->name_);
      w.u32((uint32_t)x->args_.size());
      for (auto &a : x->args_)
        write_ope(w, a);
    } else if (auto x = dynamic_cast<Whitespace *>(p)) {
      w.u8(T_Whitespace);
      write_ope(w, x->ope_);
    } else if (auto x = dynamic_cast<Recovery *>(p)) {
      w.u8(T_Recovery);
      write_ope(w, x->ope_);
    } else if (dynamic_cast<Cut *>(p)) {
      w.u8(T_Cut);
    } else if (auto x = dynamic_cast<PrecedenceClimbing *>(p)) {
      w.u8(T_PrecedenceClimbing);
      write_ope(w, x->atom_);
      write_ope(w, x->binop_);
      w.u32((uint32_t)x->info_.size());
      for (auto &[key, pri] : x->info_) {
        w.str(std::string(key));
        w.u64((uint64_t)pri.first);
        w.u8((uint8_t)pri.second);
      }
    } else {
      throw std::runtime_error(
          "GrammarBlob: operator not serializable (a custom User operator or "
          "a Capture with a match action)");
    }
  }

  struct Reader {
    const uint8_t *p, *end;
    uint8_t u8() {
      if (p >= end)
        throw std::runtime_error("GrammarBlob: unexpected end of blob");
      return *p++;
    }
    uint32_t u32() {
      uint32_t v = 0;
      for (int i = 0; i < 4; i++)
        v |= (uint32_t)u8() << (8 * i);
      return v;
    }
    uint64_t u64() {
      uint64_t v = 0;
      for (int i = 0; i < 8; i++)
        v |= (uint64_t)u8() << (8 * i);
      return v;
    }
    std::string str() {
      uint32_t n = u32();
      std::string s((const char *)p, (const char *)p + n);
      p += n;
      return s;
    }
  };

  static std::shared_ptr<Ope> read_ope(Reader &r, Grammar &g,
                                       Definition *owner) {
    switch (r.u8()) {
    case T_Null: return nullptr;
    case T_Sequence: {
      uint32_t n = r.u32();
      std::vector<std::shared_ptr<Ope>> v;
      for (uint32_t i = 0; i < n; i++)
        v.push_back(read_ope(r, g, owner));
      return std::make_shared<Sequence>(std::move(v));
    }
    case T_Choice: {
      bool fl = r.u8();
      uint32_t n = r.u32();
      std::vector<std::shared_ptr<Ope>> v;
      for (uint32_t i = 0; i < n; i++)
        v.push_back(read_ope(r, g, owner));
      auto c = std::make_shared<PrioritizedChoice>(std::move(v));
      c->for_label_ = fl;
      return c;
    }
    case T_Repetition: {
      uint64_t mn = r.u64(), mx = r.u64();
      auto o = read_ope(r, g, owner);
      return std::make_shared<Repetition>(o, mn, mx);
    }
    case T_And: return std::make_shared<AndPredicate>(read_ope(r, g, owner));
    case T_Not: return std::make_shared<NotPredicate>(read_ope(r, g, owner));
    case T_Dictionary: {
      bool ic = r.u8();
      uint32_t n = r.u32();
      std::vector<std::string> words;
      for (uint32_t i = 0; i < n; i++)
        words.push_back(r.str());
      return std::make_shared<Dictionary>(words, ic);
    }
    case T_Literal: {
      bool ic = r.u8();
      std::string s = r.str();
      return std::make_shared<LiteralString>(std::move(s), ic);
    }
    case T_CharClass: {
      bool neg = r.u8(), ic = r.u8();
      uint32_t n = r.u32();
      std::vector<std::pair<char32_t, char32_t>> ranges;
      for (uint32_t i = 0; i < n; i++) {
        auto lo = r.u32(), hi = r.u32();
        ranges.emplace_back((char32_t)lo, (char32_t)hi);
      }
      return std::make_shared<CharacterClass>(ranges, neg, ic);
    }
    case T_Char: return std::make_shared<Character>((char32_t)r.u32());
    case T_AnyChar: return std::make_shared<AnyCharacter>();
    case T_CaptureScope:
      return std::make_shared<CaptureScope>(read_ope(r, g, owner));
    case T_Capture: {
      auto o = read_ope(r, g, owner);
      return std::make_shared<Capture>(o, nullptr);
    }
    case T_TokenBoundary:
      return std::make_shared<TokenBoundary>(read_ope(r, g, owner));
    case T_Ignore: return std::make_shared<Ignore>(read_ope(r, g, owner));
    case T_BackRef: return std::make_shared<BackReference>(r.str());
    case T_Reference: {
      bool im = r.u8();
      std::string nm = r.str();
      uint32_t n = r.u32();
      std::vector<std::shared_ptr<Ope>> args;
      for (uint32_t i = 0; i < n; i++)
        args.push_back(read_ope(r, g, owner));
      return std::make_shared<Reference>(g, nm, nullptr, im, args);
    }
    case T_Whitespace:
      return std::make_shared<Whitespace>(read_ope(r, g, owner));
    case T_Recovery: return std::make_shared<Recovery>(read_ope(r, g, owner));
    case T_Cut: return std::make_shared<Cut>();
    case T_PrecedenceClimbing: {
      if (!owner) {
        throw std::runtime_error(
            "GrammarBlob: 'precedence' operator outside a rule body");
      }
      auto atom = read_ope(r, g, owner);
      auto binop = read_ope(r, g, owner);
      uint32_t n = r.u32();
      auto pc = std::make_shared<PrecedenceClimbing>(
          atom, binop, PrecedenceClimbing::BinOpeInfo{}, *owner);
      // info_ keys are string_views; back them with owned strings whose
      // addresses stay stable (reserve avoids reallocation, and the node is
      // never moved once held by shared_ptr).
      pc->info_keys_.reserve(n);
      for (uint32_t i = 0; i < n; i++) {
        std::string key = r.str();
        auto level = (size_t)r.u64();
        auto assoc = (char)r.u8();
        pc->info_keys_.push_back(std::move(key));
        pc->info_[pc->info_keys_.back()] = std::pair(level, assoc);
      }
      return pc;
    }
    default: throw std::runtime_error("GrammarBlob: bad operator tag");
    }
  }

  static const uint32_t MAGIC = 0x50454732; // "PEG2"

  static std::vector<uint8_t> serialize(const Grammar &g,
                                        const std::string &start) {
    Writer w;
    w.u32(MAGIC);
    w.str(start);
    w.u32((uint32_t)g.size());
    // Grammar is an unordered_map, whose iteration order is implementation
    // defined: walking it directly yields different bytes for the same grammar
    // on different standard libraries, so a blob generated on one platform
    // cannot be byte-compared on another. Emit the definitions by name.
    // deserialize() rebuilds the map from the names, so the order carries no
    // meaning of its own.
    std::vector<const Grammar::value_type *> defs;
    defs.reserve(g.size());
    for (auto &kv : g)
      defs.push_back(&kv);
    std::sort(defs.begin(), defs.end(),
              [](const auto *a, const auto *b) { return a->first < b->first; });
    for (auto *kv : defs) {
      const auto &name = kv->first;
      const auto &def = kv->second;
      w.str(name);
      uint8_t flags =
          (def.ignoreSemanticValue ? 1 : 0) | (def.is_macro ? 2 : 0) |
          (def.no_ast_opt ? 4 : 0) | (def.eoi_check ? 8 : 0) |
          (def.enablePackratParsing ? 16 : 0) |
          (def.is_left_recursive ? 32 : 0) | (def.can_be_empty ? 64 : 0) |
          (def.disable_action ? 128 : 0);
      w.u8(flags);
      uint8_t flags2 = (def.no_whitespace ? 1 : 0);
      w.u8(flags2);
      w.u32((uint32_t)def.params.size());
      for (auto &s : def.params)
        w.str(s);
      w.str(def.ast_name);
      w.str(def.error_message);
      write_ope(w, const_cast<Definition &>(def).get_core_operator());
    }
    return std::move(w.b);
  }

  static std::shared_ptr<Grammar> deserialize(const std::vector<uint8_t> &blob,
                                              std::string &start_out) {
    Reader r{blob.data(), blob.data() + blob.size()};
    if (r.u32() != MAGIC)
      throw std::runtime_error("GrammarBlob: bad magic / not a grammar blob");
    start_out = r.str();
    uint32_t ndef = r.u32();
    auto g = std::make_shared<Grammar>();
    // Create each Definition before reading its body: a PrecedenceClimbing node
    // needs a stable reference to its owning rule at construction. Grammar is a
    // node-based map, so references stay valid as later rules are inserted.
    for (uint32_t i = 0; i < ndef; i++) {
      std::string name = r.str();
      uint8_t flags = r.u8();
      uint8_t flags2 = r.u8();
      uint32_t np = r.u32();
      std::vector<std::string> params;
      for (uint32_t k = 0; k < np; k++)
        params.push_back(r.str());
      std::string ast_name = r.str();
      std::string err = r.str();

      auto &def = (*g)[name];
      def.name = name;
      def.ignoreSemanticValue = flags & 1;
      def.is_macro = flags & 2;
      def.no_ast_opt = flags & 4;
      def.eoi_check = flags & 8;
      def.enablePackratParsing = flags & 16;
      def.is_left_recursive = flags & 32;
      def.can_be_empty = flags & 64;
      def.disable_action = flags & 128;
      def.no_whitespace = flags2 & 1;
      def.params = std::move(params);
      def.ast_name = std::move(ast_name);
      def.error_message = std::move(err);

      auto body = read_ope(r, *g, &def);
      def <= body;
    }
    for (auto &x : *g) {
      LinkReferences vis(*g, x.second.params);
      x.second.accept(vis);
      // TraversalVisitor descends only into a PrecedenceClimbing's atom_. In
      // the from-source path binop_ is linked while the body is still a
      // Sequence, before precedence lowering; a deserialized node is built
      // already lowered so its binop_ reference must be linked explicitly here.
      auto core = x.second.get_core_operator();
      if (auto pc = std::dynamic_pointer_cast<PrecedenceClimbing>(core)) {
        pc->binop_->accept(vis);
      }
    }
    {
      SetupFirstSets vis; // shared across rules -> O(N)
      for (auto &x : *g)
        x.second.accept(vis);
    }
    // Re-derive automatic whitespace/word skipping on the start rule from the
    // %whitespace / %word definitions, exactly as ParserGenerator does. Sharing
    // the (already linked and first-set) definition operators avoids leaving
    // references inside the skipping ope unlinked, and keeps the blob smaller.
    if (g->count(WHITESPACE_DEFINITION_NAME)) {
      (*g)[start_out].whitespaceOpe =
          wsp((*g)[WHITESPACE_DEFINITION_NAME].get_core_operator());
    }
    if (g->count(WORD_DEFINITION_NAME)) {
      (*g)[start_out].wordOpe = (*g)[WORD_DEFINITION_NAME].get_core_operator();
    }
    return g;
  }
};

/*-----------------------------------------------------------------------------
 *  PEG parser generator
 *---------------------------------------------------------------------------*/

using Rules = std::unordered_map<std::string, std::shared_ptr<Ope>>;

class ParserGenerator {
public:
  struct ParserContext {
    std::shared_ptr<Grammar> grammar;
    std::string start;
    bool enablePackratParsing = false;
  };

  static ParserContext parse(const char *s, size_t n, const Rules &rules,
                             Log log, std::string_view start,
                             bool enable_left_recursion = true) {
    return get_instance().perform_core(s, n, rules, log, std::string(start),
                                       enable_left_recursion);
  }

  // For debugging purpose
  static bool parse_test(const char *d, const char *s) {
    Data data;
    std::any dt = &data;

    auto n = strlen(s);
    auto r = get_instance().g[d].parse(s, n, dt);
    return r.ret && r.len == n;
  }

#if defined(__cpp_lib_char8_t)
  static bool parse_test(const char *d, const char8_t *s) {
    return parse_test(d, reinterpret_cast<const char *>(s));
  }
#endif

private:
  static ParserGenerator &get_instance() {
    static ParserGenerator instance;
    return instance;
  }

  ParserGenerator() {
    make_grammar();
    setup_actions();
    // Apply First-Set filtering to the bootstrap meta-grammar itself so that
    // parsing a grammar (the bulk of load_grammar) skips alternatives whose
    // next byte cannot match. This is safe -- First-Set filtering only skips
    // alternatives that would have failed anyway, so no semantic action that
    // would have committed is skipped (unlike packrat, which is unsound here).
    {
      SetupFirstSets vis;
      for (auto &x : g) {
        x.second.accept(vis);
      }
    }
  }

  struct Instruction {
    std::string type;
    std::any data;
    std::string_view sv;
  };

  struct Data {
    std::shared_ptr<Grammar> grammar;
    std::string start;
    const char *start_pos = nullptr;

    std::vector<std::pair<std::string, const char *>> duplicates_of_definition;

    std::vector<std::pair<std::string, const char *>> duplicates_of_instruction;
    std::map<std::string, std::vector<Instruction>> instructions;

    std::vector<std::pair<std::string, const char *>> undefined_back_references;
    std::vector<std::set<std::string_view>> captures_stack{{}};

    std::set<std::string_view> captures_in_current_definition;
    bool enablePackratParsing = true;

    Data() : grammar(std::make_shared<Grammar>()) {}
  };

  class SyntaxErrorException : public std::runtime_error {
  public:
    SyntaxErrorException(const char *what_arg, std::pair<size_t, size_t> r)
        : std::runtime_error(what_arg), r_(r) {}

    std::pair<size_t, size_t> line_info() const { return r_; }

  private:
    std::pair<size_t, size_t> r_;
  };

  void make_grammar() {
    // Setup PEG syntax parser
    g["Grammar"] <= seq(g["Spacing"], oom(g["Definition"]), g["EndOfFile"]);
    // Left-factored: parse the rule name (IdentCont) once, then optionally the
    // macro parameter list. `opt(Parameters)` pushes a value only for a macro
    // (so the value layout matches the old two-alternative form), and Spacing
    // (~, no value) consumes the gap before LEFTARROW that Identifier used to.
    g["Definition"] <= seq(g["Ignore"], g["IdentCont"], opt(g["Parameters"]),
                           g["Spacing"], g["LEFTARROW"], g["Expression"],
                           opt(g["Instruction"]));
    g["Expression"] <= seq(g["Sequence"], zom(seq(g["SLASH"], g["Sequence"])));
    g["Sequence"] <= zom(cho(g["CUT"], g["Prefix"]));
    g["Prefix"] <= seq(opt(cho(g["AND"], g["NOT"])), g["SuffixWithLabel"]);
    g["SuffixWithLabel"] <=
        seq(g["Suffix"], opt(seq(g["LABEL"], g["Identifier"])));
    g["Suffix"] <= seq(g["Primary"], opt(g["Loop"]));
    g["Loop"] <= cho(g["QUESTION"], g["STAR"], g["PLUS"], g["Repetition"]);
    // Left-factored: a macro reference (`Name(args)`) and a plain reference
    // (`Name`) share the leading `Ignore IdentCont`, so parse it once and let
    // `opt(Arguments)` decide. opt() pushes the argument list only for a macro
    // reference, so vs.size() distinguishes the two in the action.
    g["Primary"] <=
        cho(seq(g["Ignore"], g["IdentCont"], opt(g["Arguments"]), g["Spacing"],
                npd(seq(opt(g["Parameters"]), g["LEFTARROW"]))),
            seq(g["OPEN"], g["Expression"], g["CLOSE"]),
            seq(g["BeginTok"], g["Expression"], g["EndTok"]), g["CapScope"],
            seq(g["BeginCap"], g["Expression"], g["EndCap"]), g["BackRef"],
            g["DictionaryI"], g["LiteralI"], g["Dictionary"], g["Literal"],
            g["NegatedClassI"], g["NegatedClass"], g["ClassI"], g["Class"],
            g["DOT"]);

    g["Identifier"] <= seq(g["IdentCont"], g["Spacing"]);
    g["IdentCont"] <= tok(seq(g["IdentStart"], zom(g["IdentRest"])));

    const static std::vector<std::pair<char32_t, char32_t>> range = {
        {0x0080, 0xFFFF}};
    g["IdentStart"] <= seq(npd(lit(u8(u8"↑"))), npd(lit(u8(u8"⇑"))),
                           cho(cls("a-zA-Z_%"), cls(range)));

    g["IdentRest"] <= cho(g["IdentStart"], cls("0-9"));

    g["Dictionary"] <= seq(g["LiteralD"], oom(seq(g["PIPE"], g["LiteralD"])));

    g["DictionaryI"] <=
        seq(g["LiteralID"], oom(seq(g["PIPE"], g["LiteralID"])));

    auto lit_ope = cho(seq(cls("'"), tok(zom(seq(npd(cls("'")), g["Char"]))),
                           cls("'"), g["Spacing"]),
                       seq(cls("\""), tok(zom(seq(npd(cls("\"")), g["Char"]))),
                           cls("\""), g["Spacing"]));
    g["Literal"] <= lit_ope;
    g["LiteralD"] <= lit_ope;

    auto lit_case_ignore_ope =
        cho(seq(cls("'"), tok(zom(seq(npd(cls("'")), g["Char"]))), lit("'i"),
                g["Spacing"]),
            seq(cls("\""), tok(zom(seq(npd(cls("\"")), g["Char"]))), lit("\"i"),
                g["Spacing"]));
    g["LiteralI"] <= lit_case_ignore_ope;
    g["LiteralID"] <= lit_case_ignore_ope;

    // NOTE: The original Brian Ford's paper uses 'zom' instead of 'oom'.
    g["Class"] <= seq(chr('['), npd(chr('^')),
                      tok(oom(seq(npd(chr(']')), g["Range"]))), chr(']'),
                      g["Spacing"]);
    g["ClassI"] <= seq(chr('['), npd(chr('^')),
                       tok(oom(seq(npd(chr(']')), g["Range"]))), lit("]i"),
                       g["Spacing"]);

    g["NegatedClass"] <= seq(lit("[^"),
                             tok(oom(seq(npd(chr(']')), g["Range"]))), chr(']'),
                             g["Spacing"]);
    g["NegatedClassI"] <= seq(lit("[^"),
                              tok(oom(seq(npd(chr(']')), g["Range"]))),
                              lit("]i"), g["Spacing"]);

    // NOTE: This is different from The original Brian Ford's paper, and this
    // modification allows us to specify `[+-]` as a valid char class.
    g["Range"] <= cho(seq(g["Char"], chr('-'), npd(chr(']')), g["Char"]),
                      g["ClassEscape"], g["PosixClass"], g["Char"]);

    g["ClassEscape"] <= seq(chr('\\'), cls("dDwWsS"));
    g["PosixClass"] <=
        seq(lit("[:"), opt(chr('^')), oom(cls("a-z")), lit(":]"));

    g["Char"] <=
        cho(seq(chr('\\'), cls("fnrtv'\"[]\\^-")),
            seq(chr('\\'), cls("0-3"), cls("0-7"), cls("0-7")),
            seq(chr('\\'), cls("0-7"), opt(cls("0-7"))),
            seq(lit("\\x"), cls("0-9a-fA-F"), opt(cls("0-9a-fA-F"))),
            seq(lit("\\u"),
                cho(seq(cho(seq(chr('0'), cls("0-9a-fA-F")), lit("10")),
                        rep(cls("0-9a-fA-F"), 4, 4)),
                    rep(cls("0-9a-fA-F"), 4, 5))),
            seq(npd(chr('\\')), dot()));

    g["Repetition"] <=
        seq(g["BeginBracket"], g["RepetitionRange"], g["EndBracket"]);
    g["RepetitionRange"] <= cho(seq(g["Number"], g["COMMA"], g["Number"]),
                                seq(g["Number"], g["COMMA"]), g["Number"],
                                seq(g["COMMA"], g["Number"]));
    g["Number"] <= seq(oom(cls("0-9")), g["Spacing"]);

    g["CapScope"] <= seq(g["BeginCapScope"], g["Expression"], g["EndCapScope"]);

    g["LEFTARROW"] <= seq(cho(lit("<-"), lit(u8(u8"←"))), g["Spacing"]);
    ~g["SLASH"] <= seq(chr('/'), g["Spacing"]);
    ~g["PIPE"] <= seq(chr('|'), g["Spacing"]);
    g["AND"] <= seq(chr('&'), g["Spacing"]);
    g["NOT"] <= seq(chr('!'), g["Spacing"]);
    g["QUESTION"] <= seq(chr('?'), g["Spacing"]);
    g["STAR"] <= seq(chr('*'), g["Spacing"]);
    g["PLUS"] <= seq(chr('+'), g["Spacing"]);
    ~g["OPEN"] <= seq(chr('('), g["Spacing"]);
    ~g["CLOSE"] <= seq(chr(')'), g["Spacing"]);
    g["DOT"] <= seq(chr('.'), g["Spacing"]);

    g["CUT"] <= seq(lit(u8(u8"↑")), g["Spacing"]);
    ~g["LABEL"] <= seq(cho(chr('^'), lit(u8(u8"⇑"))), g["Spacing"]);

    ~g["Spacing"] <= zom(cho(g["Space"], g["Comment"]));
    g["Comment"] <= seq(chr('#'), zom(seq(npd(g["EndOfLine"]), dot())),
                        opt(g["EndOfLine"]));
    g["Space"] <= cho(chr(' '), chr('\t'), g["EndOfLine"]);
    g["EndOfLine"] <= cho(lit("\r\n"), chr('\n'), chr('\r'));
    g["EndOfFile"] <= npd(dot());

    ~g["BeginTok"] <= seq(chr('<'), g["Spacing"]);
    ~g["EndTok"] <= seq(chr('>'), g["Spacing"]);

    ~g["BeginCapScope"] <= seq(chr('$'), chr('('), g["Spacing"]);
    ~g["EndCapScope"] <= seq(chr(')'), g["Spacing"]);

    g["BeginCap"] <= seq(chr('$'), tok(g["IdentCont"]), chr('<'), g["Spacing"]);
    ~g["EndCap"] <= seq(chr('>'), g["Spacing"]);

    g["BackRef"] <= seq(chr('$'), tok(g["IdentCont"]), g["Spacing"]);

    g["IGNORE"] <= chr('~');

    g["Ignore"] <= opt(g["IGNORE"]);
    g["Parameters"] <= seq(g["OPEN"], g["Identifier"],
                           zom(seq(g["COMMA"], g["Identifier"])), g["CLOSE"]);
    g["Arguments"] <= seq(g["OPEN"], g["Expression"],
                          zom(seq(g["COMMA"], g["Expression"])), g["CLOSE"]);
    ~g["COMMA"] <= seq(chr(','), g["Spacing"]);

    // Instruction grammars
    g["Instruction"] <=
        seq(g["BeginBracket"],
            opt(seq(g["InstructionItem"], zom(seq(g["InstructionItemSeparator"],
                                                  g["InstructionItem"])))),
            g["EndBracket"]);
    g["InstructionItem"] <= cho(g["PrecedenceClimbing"], g["ErrorMessage"],
                                g["NoAstOpt"], g["NoWhitespace"], g["AstName"]);
    ~g["InstructionItemSeparator"] <= seq(chr(';'), g["Spacing"]);

    ~g["SpacesZom"] <= zom(g["Space"]);
    ~g["SpacesOom"] <= oom(g["Space"]);
    ~g["BeginBracket"] <= seq(chr('{'), g["Spacing"]);
    ~g["EndBracket"] <= seq(chr('}'), g["Spacing"]);

    // PrecedenceClimbing instruction
    g["PrecedenceClimbing"] <=
        seq(lit("precedence"), g["SpacesOom"], g["PrecedenceInfo"],
            zom(seq(g["SpacesOom"], g["PrecedenceInfo"])), g["SpacesZom"]);
    g["PrecedenceInfo"] <=
        seq(g["PrecedenceAssoc"],
            oom(seq(ign(g["SpacesOom"]), g["PrecedenceOpe"])));
    g["PrecedenceOpe"] <=
        cho(seq(cls("'"),
                tok(zom(seq(npd(cho(g["Space"], cls("'"))), g["Char"]))),
                cls("'")),
            seq(cls("\""),
                tok(zom(seq(npd(cho(g["Space"], cls("\""))), g["Char"]))),
                cls("\"")),
            tok(oom(seq(npd(cho(g["PrecedenceAssoc"], g["Space"], chr('}'))),
                        dot()))));
    g["PrecedenceAssoc"] <= cls("LR");

    // Error message instruction
    g["ErrorMessage"] <= seq(lit("error_message"), g["SpacesOom"],
                             g["LiteralD"], g["SpacesZom"]);

    // No Ast node optimization instruction
    g["NoAstOpt"] <= seq(lit("no_ast_opt"), g["SpacesZom"]);

    // No whitespace skipping instruction
    g["NoWhitespace"] <= seq(lit("no_whitespace"), g["SpacesZom"]);

    // AST node name override instruction: `{ ast_name: NodeTag }`
    g["AstName"] <= seq(lit("ast_name"), g["SpacesZom"], lit(":"),
                        g["SpacesZom"], g["Identifier"], g["SpacesZom"]);

    // Set definition names
    for (auto &x : g) {
      x.second.name = x.first;
    }
  }

  void setup_actions() {
    g["Definition"] = [&](const SemanticValues &vs, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);

      // Macro iff the optional Parameters matched: its value (the parameter
      // name list) then sits at vs[2]. A plain definition has LEFTARROW's value
      // there instead.
      auto is_macro = vs[2].type() == typeid(std::vector<std::string>);
      auto ignore = std::any_cast<bool>(vs[0]);
      auto name = std::any_cast<std::string>(vs[1]);

      std::vector<std::string> params;
      std::shared_ptr<Ope> ope;
      auto has_instructions = false;

      if (is_macro) {
        params = std::any_cast<std::vector<std::string>>(vs[2]);
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[4]);
        if (vs.size() == 6) { has_instructions = true; }
      } else {
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[3]);
        if (vs.size() == 5) { has_instructions = true; }
      }

      if (has_instructions) {
        auto index = is_macro ? 5 : 4;
        std::unordered_set<std::string> types;
        for (const auto &instruction :
             std::any_cast<std::vector<Instruction>>(vs[index])) {
          const auto &type = instruction.type;
          if (types.find(type) == types.end()) {
            data.instructions[name].push_back(instruction);
            types.insert(instruction.type);
          } else {
            data.duplicates_of_instruction.emplace_back(type,
                                                        instruction.sv.data());
          }
        }
      }

      auto &grammar = *data.grammar;
      if (!grammar.count(name)) {
        auto &rule = grammar[name];
        rule <= ope;
        rule.name = name;
        rule.s_ = vs.sv().data();
        rule.line_ = line_info(vs.ss, rule.s_);
        rule.ignoreSemanticValue = ignore;
        rule.is_macro = is_macro;
        rule.params = params;

        // Reserved `%`-prefixed rules (%whitespace, %word, ...) are directives,
        // not parseable entry points, so they must not become the start rule.
        if (data.start.empty() && name[0] != '%') {
          data.start = rule.name;
          data.start_pos = rule.s_;
        }
      } else {
        data.duplicates_of_definition.emplace_back(name, vs.sv().data());
      }
    };

    g["Definition"].enter = [](const Context & /*c*/, const char * /*s*/,
                               size_t /*n*/, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);
      data.captures_in_current_definition.clear();
    };

    g["Expression"] = [&](const SemanticValues &vs) {
      if (vs.size() == 1) {
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      } else {
        std::vector<std::shared_ptr<Ope>> opes;
        for (auto i = 0u; i < vs.size(); i++) {
          opes.emplace_back(std::any_cast<std::shared_ptr<Ope>>(vs[i]));
        }
        const std::shared_ptr<Ope> ope =
            std::make_shared<PrioritizedChoice>(opes);
        return ope;
      }
    };

    g["Sequence"] = [&](const SemanticValues &vs) {
      if (vs.empty()) {
        return npd(lit(""));
      } else if (vs.size() == 1) {
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      } else {
        std::vector<std::shared_ptr<Ope>> opes;
        for (const auto &x : vs) {
          opes.emplace_back(std::any_cast<std::shared_ptr<Ope>>(x));
        }
        const std::shared_ptr<Ope> ope = std::make_shared<Sequence>(opes);
        return ope;
      }
    };

    g["Prefix"] = [&](const SemanticValues &vs) {
      std::shared_ptr<Ope> ope;
      if (vs.size() == 1) {
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      } else {
        assert(vs.size() == 2);
        auto tok = std::any_cast<char>(vs[0]);
        ope = std::any_cast<std::shared_ptr<Ope>>(vs[1]);
        if (tok == '&') {
          ope = apd(ope);
        } else { // '!'
          ope = npd(ope);
        }
      }
      return ope;
    };

    g["SuffixWithLabel"] = [&](const SemanticValues &vs, std::any &dt) {
      auto ope = std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      if (vs.size() == 1) {
        return ope;
      } else {
        assert(vs.size() == 2);
        auto &data = *std::any_cast<Data *>(dt);
        const auto &ident = std::any_cast<std::string>(vs[1]);
        auto label = ref(*data.grammar, ident, vs.sv().data(), false, {});
        auto recovery = rec(ref(*data.grammar, RECOVER_DEFINITION_NAME,
                                vs.sv().data(), true, {label}));
        return cho4label_(ope, recovery);
      }
    };

    struct Loop {
      enum class Type { opt = 0, zom, oom, rep };
      Type type;
      std::pair<size_t, size_t> range;
    };

    g["Suffix"] = [&](const SemanticValues &vs) {
      auto ope = std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      if (vs.size() == 1) {
        return ope;
      } else {
        assert(vs.size() == 2);
        auto loop = std::any_cast<Loop>(vs[1]);
        switch (loop.type) {
        case Loop::Type::opt: return opt(ope);
        case Loop::Type::zom: return zom(ope);
        case Loop::Type::oom: return oom(ope);
        default: // Regex-like repetition
          return rep(ope, loop.range.first, loop.range.second);
        }
      }
    };

    g["Loop"] = [&](const SemanticValues &vs) {
      switch (vs.choice()) {
      case 0: // Option
        return Loop{Loop::Type::opt, std::pair<size_t, size_t>()};
      case 1: // Zero or More
        return Loop{Loop::Type::zom, std::pair<size_t, size_t>()};
      case 2: // One or More
        return Loop{Loop::Type::oom, std::pair<size_t, size_t>()};
      default: // Regex-like repetition
        return Loop{Loop::Type::rep,
                    std::any_cast<std::pair<size_t, size_t>>(vs[0])};
      }
    };

    g["Primary"] = [&](const SemanticValues &vs, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);

      switch (vs.choice()) {
      case 0: { // Reference / Macro reference (left-factored)
        // Macro reference iff opt(Arguments) matched and pushed the arg list.
        auto is_macro = vs.size() > 2;
        auto ignore = std::any_cast<bool>(vs[0]);
        const auto &ident = std::any_cast<std::string>(vs[1]);

        std::vector<std::shared_ptr<Ope>> args;
        if (is_macro) {
          args = std::any_cast<std::vector<std::shared_ptr<Ope>>>(vs[2]);
        }

        auto ope = ref(*data.grammar, ident, vs.sv().data(), is_macro, args);
        if (ident == RECOVER_DEFINITION_NAME) { ope = rec(ope); }

        if (ignore) {
          return ign(ope);
        } else {
          return ope;
        }
      }
      case 1: { // (Expression)
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      }
      case 2: { // TokenBoundary
        return tok(std::any_cast<std::shared_ptr<Ope>>(vs[0]));
      }
      case 3: { // CaptureScope
        return csc(std::any_cast<std::shared_ptr<Ope>>(vs[0]));
      }
      case 4: { // Capture
        const auto &name = std::any_cast<std::string_view>(vs[0]);
        auto ope = std::any_cast<std::shared_ptr<Ope>>(vs[1]);

        data.captures_stack.back().insert(name);
        data.captures_in_current_definition.insert(name);

        return cap(ope, [name](const char *a_s, size_t a_n, Context &c) {
          c.capture_entries.emplace_back(name, std::string(a_s, a_n));
        });
      }
      default: {
        return std::any_cast<std::shared_ptr<Ope>>(vs[0]);
      }
      }
    };

    g["IdentCont"] = [](const SemanticValues &vs) {
      return std::string(vs.sv().data(), vs.sv().length());
    };

    g["Dictionary"] = [](const SemanticValues &vs) {
      auto items = vs.transform<std::string>();
      return dic(items, false);
    };
    g["DictionaryI"] = [](const SemanticValues &vs) {
      auto items = vs.transform<std::string>();
      return dic(items, true);
    };

    g["Literal"] = [](const SemanticValues &vs) {
      const auto &tok = vs.tokens.front();
      return lit(resolve_escape_sequence(tok.data(), tok.size()));
    };
    g["LiteralI"] = [](const SemanticValues &vs) {
      const auto &tok = vs.tokens.front();
      return liti(resolve_escape_sequence(tok.data(), tok.size()));
    };
    g["LiteralD"] = [](const SemanticValues &vs) {
      auto &tok = vs.tokens.front();
      return resolve_escape_sequence(tok.data(), tok.size());
    };
    g["LiteralID"] = [](const SemanticValues &vs) {
      auto &tok = vs.tokens.front();
      return resolve_escape_sequence(tok.data(), tok.size());
    };

    // A Range produces either a single range (std::pair) or a range list
    // (std::vector<std::pair>) for `\d`-style escapes and POSIX classes.
    auto collect_ranges = [](const SemanticValues &vs) {
      std::vector<std::pair<char32_t, char32_t>> ranges;
      for (const auto &v : vs) {
        if (v.type() == typeid(std::pair<char32_t, char32_t>)) {
          ranges.push_back(std::any_cast<std::pair<char32_t, char32_t>>(v));
        } else {
          const auto &vec =
              std::any_cast<const std::vector<std::pair<char32_t, char32_t>> &>(
                  v);
          ranges.insert(ranges.end(), vec.begin(), vec.end());
        }
      }
      return ranges;
    };

    g["Class"] = [collect_ranges](const SemanticValues &vs) {
      return cls(collect_ranges(vs));
    };
    g["ClassI"] = [collect_ranges](const SemanticValues &vs) {
      return cls(collect_ranges(vs), true);
    };
    g["NegatedClass"] = [collect_ranges](const SemanticValues &vs) {
      return ncls(collect_ranges(vs));
    };
    g["NegatedClassI"] = [collect_ranges](const SemanticValues &vs) {
      return ncls(collect_ranges(vs), true);
    };
    g["Range"] = [](const SemanticValues &vs) -> std::any {
      switch (vs.choice()) {
      case 0: {
        auto s1 = std::any_cast<std::string>(vs[0]);
        auto s2 = std::any_cast<std::string>(vs[1]);
        auto cp1 = decode_codepoint(s1.data(), s1.length());
        auto cp2 = decode_codepoint(s2.data(), s2.length());
        if (cp1 > cp2) {
          throw SyntaxErrorException("characer range is out of order...",
                                     vs.line_info());
        }
        return std::pair(cp1, cp2);
      }
      case 1: // ClassEscape
      case 2: // PosixClass
        return vs[0];
      case 3: {
        auto s = std::any_cast<std::string>(vs[0]);
        auto cp = decode_codepoint(s.data(), s.length());
        return std::pair(cp, cp);
      }
      }
      return std::pair<char32_t, char32_t>(0, 0);
    };
    g["ClassEscape"] = [](const SemanticValues &vs) {
      auto ch = vs.sv()[1];
      const char *name = nullptr;
      switch (ch) {
      case 'd':
      case 'D': name = "digit"; break;
      case 's':
      case 'S': name = "space"; break;
      default: name = "word"; break;
      }
      auto ranges = *predefined_character_class(name);
      if (ch == 'D' || ch == 'S' || ch == 'W') {
        ranges = complement_character_ranges(ranges);
      }
      return ranges;
    };
    g["PosixClass"] = [](const SemanticValues &vs) {
      auto sv = vs.sv(); // `[:name:]` or `[:^name:]`
      auto negated = sv[2] == '^';
      auto name = sv.substr(negated ? 3 : 2, sv.size() - (negated ? 5 : 4));
      auto ranges = predefined_character_class(name);
      if (!ranges) {
        auto msg = "invalid POSIX character class '" + std::string(name) + "'";
        throw SyntaxErrorException(msg.c_str(), vs.line_info());
      }
      return negated ? complement_character_ranges(*ranges) : *ranges;
    };
    g["Char"] = [](const SemanticValues &vs) {
      return resolve_escape_sequence(vs.sv().data(), vs.sv().length());
    };

    g["RepetitionRange"] = [&](const SemanticValues &vs) {
      switch (vs.choice()) {
      case 0: { // Number COMMA Number
        auto min = std::any_cast<size_t>(vs[0]);
        auto max = std::any_cast<size_t>(vs[1]);
        return std::pair(min, max);
      }
      case 1: // Number COMMA
        return std::pair(std::any_cast<size_t>(vs[0]),
                         std::numeric_limits<size_t>::max());
      case 2: { // Number
        auto n = std::any_cast<size_t>(vs[0]);
        return std::pair(n, n);
      }
      default: // COMMA Number
        return std::pair(std::numeric_limits<size_t>::min(),
                         std::any_cast<size_t>(vs[0]));
      }
    };
    g["Number"] = [&](const SemanticValues &vs) {
      return vs.token_to_number<size_t>();
    };

    g["CapScope"].enter = [](const Context & /*c*/, const char * /*s*/,
                             size_t /*n*/, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);
      data.captures_stack.emplace_back();
    };
    g["CapScope"].leave = [](const Context & /*c*/, const char * /*s*/,
                             size_t /*n*/, size_t /*matchlen*/,
                             std::any & /*value*/, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);
      data.captures_stack.pop_back();
    };

    g["AND"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["NOT"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["QUESTION"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["STAR"] = [](const SemanticValues &vs) { return *vs.sv().data(); };
    g["PLUS"] = [](const SemanticValues &vs) { return *vs.sv().data(); };

    g["DOT"] = [](const SemanticValues & /*vs*/) { return dot(); };

    g["CUT"] = [](const SemanticValues & /*vs*/) { return cut(); };

    g["BeginCap"] = [](const SemanticValues &vs) { return vs.token(); };

    g["BackRef"] = [&](const SemanticValues &vs, std::any &dt) {
      auto &data = *std::any_cast<Data *>(dt);

      // Undefined back reference check
      {
        auto found = false;
        auto it = data.captures_stack.rbegin();
        while (it != data.captures_stack.rend()) {
          if (it->find(vs.token()) != it->end()) {
            found = true;
            break;
          }
          ++it;
        }
        if (!found) {
          auto ptr = vs.token().data() - 1; // include '$' symbol
          data.undefined_back_references.emplace_back(vs.token(), ptr);
        }
      }

      // NOTE: Disable packrat parsing if a back reference is not defined in
      // captures in the current definition rule.
      if (data.captures_in_current_definition.find(vs.token()) ==
          data.captures_in_current_definition.end()) {
        data.enablePackratParsing = false;
      }

      return bkr(vs.token_to_string());
    };

    g["Ignore"] = [](const SemanticValues &vs) { return vs.size() > 0; };

    g["Parameters"] = [](const SemanticValues &vs) {
      return vs.transform<std::string>();
    };

    g["Arguments"] = [](const SemanticValues &vs) {
      return vs.transform<std::shared_ptr<Ope>>();
    };

    g["PrecedenceClimbing"] = [](const SemanticValues &vs) {
      PrecedenceClimbing::BinOpeInfo binOpeInfo;
      size_t level = 1;
      for (const auto &v : vs) {
        auto tokens = std::any_cast<std::vector<std::string_view>>(v);
        auto assoc = tokens[0][0];
        for (size_t i = 1; i < tokens.size(); i++) {
          binOpeInfo[tokens[i]] = std::pair(level, assoc);
        }
        level++;
      }
      Instruction instruction;
      instruction.type = "precedence";
      instruction.data = binOpeInfo;
      instruction.sv = vs.sv();
      return instruction;
    };
    g["PrecedenceInfo"] = [](const SemanticValues &vs) {
      return vs.transform<std::string_view>();
    };
    g["PrecedenceOpe"] = [](const SemanticValues &vs) { return vs.token(); };
    g["PrecedenceAssoc"] = [](const SemanticValues &vs) { return vs.token(); };

    g["ErrorMessage"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "error_message";
      instruction.data = std::any_cast<std::string>(vs[0]);
      instruction.sv = vs.sv();
      return instruction;
    };

    g["NoAstOpt"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "no_ast_opt";
      instruction.sv = vs.sv();
      return instruction;
    };

    g["NoWhitespace"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "no_whitespace";
      instruction.sv = vs.sv();
      return instruction;
    };

    g["AstName"] = [](const SemanticValues &vs) {
      Instruction instruction;
      instruction.type = "ast_name";
      instruction.data = std::any_cast<std::string>(vs[0]);
      instruction.sv = vs.sv();
      return instruction;
    };

    g["Instruction"] = [](const SemanticValues &vs) {
      return vs.transform<Instruction>();
    };
  }

  bool apply_precedence_instruction(Definition &rule,
                                    const PrecedenceClimbing::BinOpeInfo &info,
                                    const char *s, Log log) {
    try {
      auto &seq = dynamic_cast<Sequence &>(*rule.get_core_operator());
      auto atom = seq.opes_[0];
      auto &rep = dynamic_cast<Repetition &>(*seq.opes_[1]);
      auto &seq1 = dynamic_cast<Sequence &>(*rep.ope_);
      auto binop = seq1.opes_[0];
      auto atom1 = seq1.opes_[1];

      auto atom_name = dynamic_cast<Reference &>(*atom).name_;
      auto binop_name = dynamic_cast<Reference &>(*binop).name_;
      auto atom1_name = dynamic_cast<Reference &>(*atom1).name_;

      if (!rep.is_zom() || atom_name != atom1_name || atom_name == binop_name) {
        if (log) {
          auto line = line_info(s, rule.s_);
          log(line.first, line.second,
              "'precedence' instruction cannot be applied to '" + rule.name +
                  "'.",
              "");
        }
        return false;
      }

      rule.holder_->ope_ = pre(atom, binop, info, rule);
      rule.disable_action = true;
    } catch (...) {
      if (log) {
        auto line = line_info(s, rule.s_);
        log(line.first, line.second,
            "'precedence' instruction cannot be applied to '" + rule.name +
                "'.",
            "");
      }
      return false;
    }
    return true;
  }

  ParserContext perform_core(const char *s, size_t n, const Rules &rules,
                             Log log, std::string requested_start,
                             bool enable_left_recursion = true) {
    Data data;
    auto &grammar = *data.grammar;

    // Built-in macros
    {
      // `%recover`
      {
        auto &rule = grammar[RECOVER_DEFINITION_NAME];
        rule <= ref(grammar, "x", "", false, {});
        rule.name = RECOVER_DEFINITION_NAME;
        rule.s_ = "[native]";
        rule.ignoreSemanticValue = true;
        rule.is_macro = true;
        rule.params = {"x"};
      }
    }

    try {
      std::any dt = &data;
      auto r = g["Grammar"].parse(s, n, dt, nullptr, log);

      if (!r.ret) {
        if (log) {
          if (r.error_info.message_pos) {
            auto line = line_info(s, r.error_info.message_pos);
            log(line.first, line.second, r.error_info.message,
                r.error_info.label);
          } else {
            auto line = line_info(s, r.error_info.error_pos);
            log(line.first, line.second, "syntax error", r.error_info.label);
          }
        }
        return {};
      }
    } catch (const SyntaxErrorException &e) {
      if (log) {
        auto line = e.line_info();
        log(line.first, line.second, e.what(), "");
      }
      return {};
    }

    // User provided rules
    for (auto [user_name, user_rule] : rules) {
      auto name = user_name;
      auto ignore = false;
      if (!name.empty() && name[0] == '~') {
        ignore = true;
        name.erase(0, 1);
      }
      if (!name.empty()) {
        auto &rule = grammar[name];
        rule <= user_rule;
        rule.name = name;
        rule.ignoreSemanticValue = ignore;
      }
    }

    // Check duplicated definitions
    auto ret = true;

    if (!data.duplicates_of_definition.empty()) {
      for (const auto &[name, ptr] : data.duplicates_of_definition) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second,
              "the definition '" + name + "' is already defined.", "");
        }
      }
      ret = false;
    }

    // Check duplicated instructions
    if (!data.duplicates_of_instruction.empty()) {
      for (const auto &[type, ptr] : data.duplicates_of_instruction) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second,
              "the instruction '" + type + "' is already defined.", "");
        }
      }
      ret = false;
    }

    // Check undefined back references
    if (!data.undefined_back_references.empty()) {
      for (const auto &[name, ptr] : data.undefined_back_references) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second,
              "the back reference '" + name + "' is undefined.", "");
        }
      }
      ret = false;
    }

    // Set root definition
    auto start = data.start;

    if (!requested_start.empty()) {
      if (grammar.count(requested_start)) {
        start = requested_start;
      } else {
        if (log) {
          auto line = line_info(s, s);
          log(line.first, line.second,
              "the specified start rule '" + requested_start +
                  "' is undefined.",
              "");
        }
        ret = false;
      }
    }

    if (!ret) { return {}; }

    auto &start_rule = grammar[start];

    // Check if the start rule has ignore operator
    {
      if (start_rule.ignoreSemanticValue) {
        if (log) {
          auto line = line_info(s, start_rule.s_);
          log(line.first, line.second,
              "ignore operator cannot be applied to '" + start_rule.name + "'.",
              "");
        }
        ret = false;
      }
    }

    if (!ret) { return {}; }

    // Check missing definitions
    auto referenced = std::unordered_set<std::string>{
        WHITESPACE_DEFINITION_NAME,
        WORD_DEFINITION_NAME,
        RECOVER_DEFINITION_NAME,
        start_rule.name,
    };

    for (auto &[_, rule] : grammar) {
      ReferenceChecker vis(grammar, rule.params);
      rule.accept(vis);
      referenced.insert(vis.referenced.begin(), vis.referenced.end());
      for (const auto &[name, ptr] : vis.error_s) {
        if (log) {
          auto line = line_info(s, ptr);
          log(line.first, line.second, vis.error_message[name], "");
        }
        ret = false;
      }
    }

    for (auto &[name, rule] : grammar) {
      if (!referenced.count(name)) {
        if (log) {
          auto line = line_info(s, rule.s_);
          auto msg = "'" + name + "' is not referenced.";
          log(line.first, line.second, msg, "");
        }
      }
    }

    if (!ret) { return {}; }

    // Link references
    for (auto &x : grammar) {
      auto &rule = x.second;
      LinkReferences vis(grammar, rule.params);
      rule.accept(vis);
    }

    // Compute can_be_empty for each rule (fixed-point iteration)
    {
      bool changed = true;
      while (changed) {
        changed = false;
        for (auto &[name, rule] : grammar) {
          ComputeCanBeEmpty vis;
          rule.accept(vis);
          if (vis.result != rule.can_be_empty) {
            rule.can_be_empty = vis.result;
            changed = true;
          }
        }
      }
    }

    // Check left recursion
    if (enable_left_recursion) {
      for (auto &[name, rule] : grammar) {
        DetectLeftRecursion vis(name);
        rule.accept(vis);
        if (vis.error_s) { rule.is_left_recursive = true; }
      }
    } else {
      ret = true;

      for (auto &[name, rule] : grammar) {
        DetectLeftRecursion vis(name);
        rule.accept(vis);
        if (vis.error_s) {
          if (log) {
            auto line = line_info(s, vis.error_s);
            log(line.first, line.second, "'" + name + "' is left recursive.",
                "");
          }
          ret = false;
        }
      }

      if (!ret) { return {}; }
    }

    // Check infinite loop
    if (detect_infiniteLoop(data, start_rule, log, s)) { return {}; }

    // Automatic whitespace skipping
    if (grammar.count(WHITESPACE_DEFINITION_NAME)) {
      for (auto &x : grammar) {
        auto &rule = x.second;
        auto ope = rule.get_core_operator();
        if (IsLiteralToken::check(*ope)) { rule <= tok(ope); }
      }

      auto &rule = grammar[WHITESPACE_DEFINITION_NAME];
      start_rule.whitespaceOpe = wsp(rule.get_core_operator());

      if (detect_infiniteLoop(data, rule, log, s)) { return {}; }
    }

    // Word expression
    if (grammar.count(WORD_DEFINITION_NAME)) {
      auto &rule = grammar[WORD_DEFINITION_NAME];
      start_rule.wordOpe = rule.get_core_operator();

      if (detect_infiniteLoop(data, rule, log, s)) { return {}; }
    }

    // Apply instructions
    for (const auto &[name, instructions] : data.instructions) {
      auto &rule = grammar[name];

      for (const auto &instruction : instructions) {
        if (instruction.type == "precedence") {
          const auto &info =
              std::any_cast<PrecedenceClimbing::BinOpeInfo>(instruction.data);

          if (!apply_precedence_instruction(rule, info, s, log)) { return {}; }
        } else if (instruction.type == "error_message") {
          rule.error_message = std::any_cast<std::string>(instruction.data);
        } else if (instruction.type == "no_ast_opt") {
          rule.no_ast_opt = true;
        } else if (instruction.type == "no_whitespace") {
          rule.no_whitespace = true;
        } else if (instruction.type == "ast_name") {
          rule.ast_name = std::any_cast<std::string>(instruction.data);
        }
      }
    }

    // Setup First-Set and ISpan optimizations. A single visitor is shared
    // across all rules so its first-set cache and visited-rule set persist:
    // each rule's first-sets are computed once (O(N)) instead of re-walking
    // every reachable rule once per referencing rule (O(N^2)).
    {
      SetupFirstSets vis;
      for (auto &x : grammar) {
        x.second.accept(vis);
      }
    }

    return {data.grammar, start, data.enablePackratParsing};
  }

  bool detect_infiniteLoop(const Data &data, Definition &rule, const Log &log,
                           const char *s) const {
    std::vector<std::pair<const char *, std::string>> refs;
    std::unordered_map<std::string, bool> has_error_cache;
    DetectInfiniteLoop vis(data.start_pos, rule.name, refs, has_error_cache);
    rule.accept(vis);
    if (vis.has_error) {
      if (log) {
        auto line = line_info(s, vis.error_s);
        log(line.first, line.second,
            "infinite loop is detected in '" + vis.error_name + "'.", "");
      }
      return true;
    }
    return false;
  }

  Grammar g;
};

/*-----------------------------------------------------------------------------
 *  AST
 *---------------------------------------------------------------------------*/

template <typename Annotation> struct AstBase : public Annotation {
  AstBase(const char *path, size_t line, size_t column, const char *name,
          const std::vector<std::shared_ptr<AstBase>> &nodes,
          size_t position = 0, size_t length = 0, size_t choice_count = 0,
          size_t choice = 0, bool preserve_position = false)
      : path(path ? path : ""), line(line), column(column), name(name),
        position(position), length(length), choice_count(choice_count),
        choice(choice), original_name(name),
        original_choice_count(choice_count), original_choice(choice),
        tag(str2tag(name)), original_tag(tag), is_token(false),
        preserve_position(preserve_position), nodes(nodes) {}

  AstBase(const char *path, size_t line, size_t column, const char *name,
          const std::string_view &token, size_t position = 0, size_t length = 0,
          size_t choice_count = 0, size_t choice = 0,
          bool preserve_position = false)
      : path(path ? path : ""), line(line), column(column), name(name),
        position(position), length(length), choice_count(choice_count),
        choice(choice), original_name(name),
        original_choice_count(choice_count), original_choice(choice),
        tag(str2tag(name)), original_tag(tag), is_token(true),
        preserve_position(preserve_position), token(token) {}

  AstBase(const AstBase &ast, const char *original_name, size_t position = 0,
          size_t length = 0, size_t original_choice_count = 0,
          size_t original_choice = 0)
      : path(ast.path), line(ast.line), column(ast.column), name(ast.name),
        position(position), length(length), choice_count(ast.choice_count),
        choice(ast.choice), original_name(original_name),
        original_choice_count(original_choice_count),
        original_choice(original_choice), tag(ast.tag),
        original_tag(str2tag(original_name)), is_token(ast.is_token),
        preserve_position(ast.preserve_position), token(ast.token),
        nodes(ast.nodes), parent(ast.parent) {}

  const std::string path;
  const size_t line = 1;
  const size_t column = 1;

  const std::string name;
  size_t position;
  size_t length;
  const size_t choice_count;
  const size_t choice;
  const std::string original_name;
  const size_t original_choice_count;
  const size_t original_choice;
  const unsigned int tag;
  const unsigned int original_tag;

  const bool is_token;
  const bool preserve_position;
  const std::string_view token;

  std::vector<std::shared_ptr<AstBase<Annotation>>> nodes;
  std::weak_ptr<AstBase<Annotation>> parent;

  std::string token_to_string() const {
    assert(is_token);
    return std::string(token);
  }

  template <typename T> T token_to_number() const {
    return token_to_number_<T>(token);
  }
};

template <typename T>
void ast_to_s_core(const std::shared_ptr<T> &ptr, std::string &s, int level,
                   std::function<std::string(const T &ast, int level)> fn) {
  const auto &ast = *ptr;
  for (auto i = 0; i < level; i++) {
    s += "  ";
  }
  auto name = ast.original_name;
  if (ast.original_choice_count > 0) {
    name += "/" + std::to_string(ast.original_choice);
  }
  if (ast.name != ast.original_name) { name += "[" + ast.name + "]"; }
  if (ast.is_token) {
    s += "- " + name + " (";
    s += ast.token;
    s += ")\n";
  } else {
    s += "+ " + name + "\n";
  }
  if (fn) { s += fn(ast, level + 1); }
  for (const auto &node : ast.nodes) {
    ast_to_s_core(node, s, level + 1, fn);
  }
}

template <typename T>
std::string
ast_to_s(const std::shared_ptr<T> &ptr,
         std::function<std::string(const T &ast, int level)> fn = nullptr) {
  std::string s;
  ast_to_s_core(ptr, s, 0, fn);
  return s;
}

struct AstOptimizer {
  AstOptimizer(bool mode, const std::vector<std::string> &rules = {})
      : mode_(mode), rules_(rules) {}

  template <typename T>
  std::shared_ptr<T> optimize(std::shared_ptr<T> original,
                              std::shared_ptr<T> parent = nullptr) {
    auto found =
        std::find(rules_.begin(), rules_.end(), original->name) != rules_.end();
    auto opt = mode_ ? !found : found;

    if (opt && original->nodes.size() == 1) {
      auto child = optimize(original->nodes[0], parent);
      auto pos =
          child->preserve_position ? child->position : original->position;
      auto len = child->preserve_position ? child->length : original->length;
      auto ast = std::make_shared<T>(*child, original->name.data(), pos, len,
                                     original->choice_count, original->choice);
      for (auto &node : ast->nodes) {
        node->parent = ast;
      }
      return ast;
    }

    auto ast = std::make_shared<T>(*original);
    ast->parent = parent;
    ast->nodes.clear();
    for (const auto &node : original->nodes) {
      auto child = optimize(node, ast);
      ast->nodes.push_back(child);
    }
    return ast;
  }

private:
  const bool mode_;
  const std::vector<std::string> rules_;
};

struct EmptyType {};
using Ast = AstBase<EmptyType>;

template <typename T = Ast> void add_ast_action(Definition &rule) {
  rule.action = [&](const SemanticValues &vs) {
    auto line = vs.line_info();

    // `{ ast_name: X }` overrides the node's name/tag (falls back to the
    // rule's own name when unset).
    const char *node_name =
        rule.ast_name.empty() ? rule.name.data() : rule.ast_name.data();

    if (rule.is_token()) {
      return std::make_shared<T>(
          vs.path, line.first, line.second, node_name, vs.token(),
          std::distance(vs.ss, vs.sv().data()), vs.sv().length(),
          vs.choice_count(), vs.choice(), rule.no_ast_opt);
    }

    auto ast = std::make_shared<T>(vs.path, line.first, line.second, node_name,
                                   vs.transform<std::shared_ptr<T>>(),
                                   std::distance(vs.ss, vs.sv().data()),
                                   vs.sv().length(), vs.choice_count(),
                                   vs.choice(), rule.no_ast_opt);

    for (auto &node : ast->nodes) {
      node->parent = ast;
    }
    return ast;
  };
}

#define PEG_EXPAND(...) __VA_ARGS__
#define PEG_CONCAT(a, b) a##b
#define PEG_CONCAT2(a, b) PEG_CONCAT(a, b)

#define PEG_PICK(                                                              \
    a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, \
    a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, \
    a32, a33, a34, a35, a36, a37, a38, a39, a40, a41, a42, a43, a44, a45, a46, \
    a47, a48, a49, a50, a51, a52, a53, a54, a55, a56, a57, a58, a59, a60, a61, \
    a62, a63, a64, a65, a66, a67, a68, a69, a70, a71, a72, a73, a74, a75, a76, \
    a77, a78, a79, a80, a81, a82, a83, a84, a85, a86, a87, a88, a89, a90, a91, \
    a92, a93, a94, a95, a96, a97, a98, a99, a100, ...)                         \
  a100

#define PEG_COUNT(...)                                                         \
  PEG_EXPAND(PEG_PICK(                                                         \
      __VA_ARGS__, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87,    \
      86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69,  \
      68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51,  \
      50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,  \
      32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15,  \
      14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define PEG_DEF_1(r)                                                           \
  peg::Definition r;                                                           \
  r.name = #r;                                                                 \
  peg::add_ast_action(r);

#define PEG_DEF_2(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_1(__VA_ARGS__))
#define PEG_DEF_3(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_2(__VA_ARGS__))
#define PEG_DEF_4(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_3(__VA_ARGS__))
#define PEG_DEF_5(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_4(__VA_ARGS__))
#define PEG_DEF_6(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_5(__VA_ARGS__))
#define PEG_DEF_7(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_6(__VA_ARGS__))
#define PEG_DEF_8(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_7(__VA_ARGS__))
#define PEG_DEF_9(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_8(__VA_ARGS__))
#define PEG_DEF_10(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_9(__VA_ARGS__))
#define PEG_DEF_11(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_10(__VA_ARGS__))
#define PEG_DEF_12(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_11(__VA_ARGS__))
#define PEG_DEF_13(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_12(__VA_ARGS__))
#define PEG_DEF_14(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_13(__VA_ARGS__))
#define PEG_DEF_15(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_14(__VA_ARGS__))
#define PEG_DEF_16(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_15(__VA_ARGS__))
#define PEG_DEF_17(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_16(__VA_ARGS__))
#define PEG_DEF_18(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_17(__VA_ARGS__))
#define PEG_DEF_19(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_18(__VA_ARGS__))
#define PEG_DEF_20(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_19(__VA_ARGS__))
#define PEG_DEF_21(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_20(__VA_ARGS__))
#define PEG_DEF_22(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_21(__VA_ARGS__))
#define PEG_DEF_23(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_22(__VA_ARGS__))
#define PEG_DEF_24(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_23(__VA_ARGS__))
#define PEG_DEF_25(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_24(__VA_ARGS__))
#define PEG_DEF_26(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_25(__VA_ARGS__))
#define PEG_DEF_27(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_26(__VA_ARGS__))
#define PEG_DEF_28(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_27(__VA_ARGS__))
#define PEG_DEF_29(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_28(__VA_ARGS__))
#define PEG_DEF_30(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_29(__VA_ARGS__))
#define PEG_DEF_31(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_30(__VA_ARGS__))
#define PEG_DEF_32(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_31(__VA_ARGS__))
#define PEG_DEF_33(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_32(__VA_ARGS__))
#define PEG_DEF_34(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_33(__VA_ARGS__))
#define PEG_DEF_35(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_34(__VA_ARGS__))
#define PEG_DEF_36(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_35(__VA_ARGS__))
#define PEG_DEF_37(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_36(__VA_ARGS__))
#define PEG_DEF_38(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_37(__VA_ARGS__))
#define PEG_DEF_39(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_38(__VA_ARGS__))
#define PEG_DEF_40(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_39(__VA_ARGS__))
#define PEG_DEF_41(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_40(__VA_ARGS__))
#define PEG_DEF_42(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_41(__VA_ARGS__))
#define PEG_DEF_43(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_42(__VA_ARGS__))
#define PEG_DEF_44(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_43(__VA_ARGS__))
#define PEG_DEF_45(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_44(__VA_ARGS__))
#define PEG_DEF_46(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_45(__VA_ARGS__))
#define PEG_DEF_47(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_46(__VA_ARGS__))
#define PEG_DEF_48(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_47(__VA_ARGS__))
#define PEG_DEF_49(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_48(__VA_ARGS__))
#define PEG_DEF_50(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_49(__VA_ARGS__))
#define PEG_DEF_51(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_50(__VA_ARGS__))
#define PEG_DEF_52(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_51(__VA_ARGS__))
#define PEG_DEF_53(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_52(__VA_ARGS__))
#define PEG_DEF_54(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_53(__VA_ARGS__))
#define PEG_DEF_55(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_54(__VA_ARGS__))
#define PEG_DEF_56(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_55(__VA_ARGS__))
#define PEG_DEF_57(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_56(__VA_ARGS__))
#define PEG_DEF_58(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_57(__VA_ARGS__))
#define PEG_DEF_59(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_58(__VA_ARGS__))
#define PEG_DEF_60(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_59(__VA_ARGS__))
#define PEG_DEF_61(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_60(__VA_ARGS__))
#define PEG_DEF_62(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_61(__VA_ARGS__))
#define PEG_DEF_63(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_62(__VA_ARGS__))
#define PEG_DEF_64(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_63(__VA_ARGS__))
#define PEG_DEF_65(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_64(__VA_ARGS__))
#define PEG_DEF_66(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_65(__VA_ARGS__))
#define PEG_DEF_67(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_66(__VA_ARGS__))
#define PEG_DEF_68(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_67(__VA_ARGS__))
#define PEG_DEF_69(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_68(__VA_ARGS__))
#define PEG_DEF_70(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_69(__VA_ARGS__))
#define PEG_DEF_71(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_70(__VA_ARGS__))
#define PEG_DEF_72(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_71(__VA_ARGS__))
#define PEG_DEF_73(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_72(__VA_ARGS__))
#define PEG_DEF_74(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_73(__VA_ARGS__))
#define PEG_DEF_75(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_74(__VA_ARGS__))
#define PEG_DEF_76(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_75(__VA_ARGS__))
#define PEG_DEF_77(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_76(__VA_ARGS__))
#define PEG_DEF_78(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_77(__VA_ARGS__))
#define PEG_DEF_79(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_78(__VA_ARGS__))
#define PEG_DEF_80(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_79(__VA_ARGS__))
#define PEG_DEF_81(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_80(__VA_ARGS__))
#define PEG_DEF_82(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_81(__VA_ARGS__))
#define PEG_DEF_83(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_82(__VA_ARGS__))
#define PEG_DEF_84(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_83(__VA_ARGS__))
#define PEG_DEF_85(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_84(__VA_ARGS__))
#define PEG_DEF_86(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_85(__VA_ARGS__))
#define PEG_DEF_87(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_86(__VA_ARGS__))
#define PEG_DEF_88(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_87(__VA_ARGS__))
#define PEG_DEF_89(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_88(__VA_ARGS__))
#define PEG_DEF_90(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_89(__VA_ARGS__))
#define PEG_DEF_91(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_90(__VA_ARGS__))
#define PEG_DEF_92(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_91(__VA_ARGS__))
#define PEG_DEF_93(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_92(__VA_ARGS__))
#define PEG_DEF_94(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_93(__VA_ARGS__))
#define PEG_DEF_95(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_94(__VA_ARGS__))
#define PEG_DEF_96(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_95(__VA_ARGS__))
#define PEG_DEF_97(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_96(__VA_ARGS__))
#define PEG_DEF_98(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_97(__VA_ARGS__))
#define PEG_DEF_99(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_98(__VA_ARGS__))
#define PEG_DEF_100(r1, ...) PEG_EXPAND(PEG_DEF_1(r1) PEG_DEF_99(__VA_ARGS__))

#define AST_DEFINITIONS(...)                                                   \
  PEG_EXPAND(PEG_CONCAT2(PEG_DEF_, PEG_COUNT(__VA_ARGS__))(__VA_ARGS__))

/*-----------------------------------------------------------------------------
 *  parser
 *---------------------------------------------------------------------------*/

class parser {
public:
  parser() = default;

  parser(const char *s, size_t n, const Rules &rules,
         std::string_view start = {}) {
    load_grammar(s, n, rules, start);
  }

  parser(const char *s, size_t n, std::string_view start = {})
      : parser(s, n, Rules(), start) {}

  parser(std::string_view sv, const Rules &rules, std::string_view start = {})
      : parser(sv.data(), sv.size(), rules, start) {}

  parser(std::string_view sv, std::string_view start = {})
      : parser(sv.data(), sv.size(), Rules(), start) {}

#if defined(__cpp_lib_char8_t)
  parser(std::u8string_view sv, const Rules &rules, std::string_view start = {})
      : parser(reinterpret_cast<const char *>(sv.data()), sv.size(), rules,
               start) {}

  parser(std::u8string_view sv, std::string_view start = {})
      : parser(reinterpret_cast<const char *>(sv.data()), sv.size(), Rules(),
               start) {}
#endif

  operator bool() const { return grammar_ != nullptr; }

  bool load_grammar(const char *s, size_t n, const Rules &rules,
                    std::string_view start = {}) {
    auto cxt =
        ParserGenerator::parse(s, n, rules, log_, start, enableLeftRecursion_);
    grammar_ = cxt.grammar;
    start_ = cxt.start;
    enablePackratParsing_ = cxt.enablePackratParsing;
    return grammar_ != nullptr;
  }

  bool load_grammar(const char *s, size_t n, std::string_view start = {}) {
    return load_grammar(s, n, Rules(), start);
  }

  bool load_grammar(std::string_view sv, const Rules &rules,
                    std::string_view start = {}) {
    return load_grammar(sv.data(), sv.size(), rules, start);
  }

  bool load_grammar(std::string_view sv, std::string_view start = {}) {
    return load_grammar(sv.data(), sv.size(), Rules(), start);
  }

  // Serialize the loaded grammar to a portable byte blob (see GrammarBlob).
  // Semantic callbacks are not included; throws if the grammar is not
  // serializable (uses the `User` operator or a Capture with a match action).
  std::vector<uint8_t> serialize_grammar() const {
    return GrammarBlob::serialize(*grammar_, start_);
  }

  // Load a grammar from a blob produced by serialize_grammar() / GrammarBlob,
  // skipping the meta-parse. Re-apply enable_ast() etc. afterwards as needed.
  bool load_blob(const std::vector<uint8_t> &blob) {
    try {
      grammar_ = GrammarBlob::deserialize(blob, start_);
    } catch (const std::exception &) { return false; }
    if (grammar_ != nullptr) {
      // Symmetry with load_grammar(): restore the parser-level packrat flag
      // from the blob so a later enable_packrat_parsing() re-applies it
      // instead of resetting the start rule to the false member default.
      enablePackratParsing_ = (*grammar_)[start_].enablePackratParsing;
    }
    return grammar_ != nullptr;
  }

  bool parse_n(const char *s, size_t n, const char *path = nullptr) const {
    if (grammar_ != nullptr) {
      const auto &rule = (*grammar_)[start_];
      auto result = rule.parse(s, n, path, log_, error_reporter_);
      return post_process(s, n, result);
    }
    return false;
  }

  bool parse_n(const char *s, size_t n, std::any &dt,
               const char *path = nullptr) const {
    if (grammar_ != nullptr) {
      const auto &rule = (*grammar_)[start_];
      auto result = rule.parse(s, n, dt, path, log_, error_reporter_);
      return post_process(s, n, result);
    }
    return false;
  }

  template <typename T>
  bool parse_n(const char *s, size_t n, T &val,
               const char *path = nullptr) const {
    if (grammar_ != nullptr) {
      const auto &rule = (*grammar_)[start_];
      auto result =
          rule.parse_and_get_value(s, n, val, path, log_, error_reporter_);
      return post_process(s, n, result);
    }
    return false;
  }

  template <typename T>
  bool parse_n(const char *s, size_t n, std::any &dt, T &val,
               const char *path = nullptr) const {
    if (grammar_ != nullptr) {
      const auto &rule = (*grammar_)[start_];
      auto result =
          rule.parse_and_get_value(s, n, dt, val, path, log_, error_reporter_);
      return post_process(s, n, result);
    }
    return false;
  }

  bool parse(std::string_view sv, const char *path = nullptr) const {
    return parse_n(sv.data(), sv.size(), path);
  }

  bool parse(std::string_view sv, std::any &dt,
             const char *path = nullptr) const {
    return parse_n(sv.data(), sv.size(), dt, path);
  }

  template <typename T>
  bool parse(std::string_view sv, T &val, const char *path = nullptr) const {
    return parse_n(sv.data(), sv.size(), val, path);
  }

  template <typename T>
  bool parse(std::string_view sv, std::any &dt, T &val,
             const char *path = nullptr) const {
    return parse_n(sv.data(), sv.size(), dt, val, path);
  }

#if defined(__cpp_lib_char8_t)
  bool parse(std::u8string_view sv, const char *path = nullptr) const {
    return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), path);
  }

  bool parse(std::u8string_view sv, std::any &dt,
             const char *path = nullptr) const {
    return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), dt,
                   path);
  }

  template <typename T>
  bool parse(std::u8string_view sv, T &val, const char *path = nullptr) const {
    return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), val,
                   path);
  }

  template <typename T>
  bool parse(std::u8string_view sv, std::any &dt, T &val,
             const char *path = nullptr) const {
    return parse_n(reinterpret_cast<const char *>(sv.data()), sv.size(), dt,
                   val, path);
  }
#endif

  Definition &operator[](const char *s) { return (*grammar_)[s]; }

  const Definition &operator[](const char *s) const { return (*grammar_)[s]; }

  const Grammar &get_grammar() const { return *grammar_; }

  void disable_eoi_check() {
    if (grammar_ != nullptr) {
      auto &rule = (*grammar_)[start_];
      rule.eoi_check = false;
    }
  }

  void enable_left_recursion(bool enable = true) {
    enableLeftRecursion_ = enable;
  }

  void enable_packrat_parsing() {
    if (grammar_ != nullptr) {
      auto &rule = (*grammar_)[start_];
      rule.enablePackratParsing = enablePackratParsing_;
    }
  }

  void enable_trace(TracerEnter tracer_enter, TracerLeave tracer_leave) {
    if (grammar_ != nullptr) {
      auto &rule = (*grammar_)[start_];
      rule.tracer_enter = tracer_enter;
      rule.tracer_leave = tracer_leave;
    }
  }

  void enable_trace(TracerEnter tracer_enter, TracerLeave tracer_leave,
                    TracerStartOrEnd tracer_start,
                    TracerStartOrEnd tracer_end) {
    if (grammar_ != nullptr) {
      auto &rule = (*grammar_)[start_];
      rule.tracer_enter = tracer_enter;
      rule.tracer_leave = tracer_leave;
      rule.tracer_start = tracer_start;
      rule.tracer_end = tracer_end;
    }
  }

  void set_verbose_trace(bool verbose_trace) {
    if (grammar_ != nullptr) {
      auto &rule = (*grammar_)[start_];
      rule.verbose_trace = verbose_trace;
    }
  }

  template <typename T = Ast> parser &enable_ast() {
    for (auto &[_, rule] : *grammar_) {
      if (!rule.action) { add_ast_action<T>(rule); }
    }
    return *this;
  }

  template <typename T>
  std::shared_ptr<T> optimize_ast(std::shared_ptr<T> ast,
                                  bool opt_mode = true) const {
    return AstOptimizer(opt_mode, get_no_ast_opt_rules()).optimize(ast);
  }

  void set_logger(Log log) { log_ = log; }

  // Receive structured error information instead of (or in addition to) the
  // formatted string passed to the logger.
  void set_error_reporter(ErrorReporter reporter) {
    error_reporter_ = reporter;
  }

  void set_logger(
      std::function<void(size_t line, size_t col, const std::string &msg)>
          log) {
    log_ = [log](size_t line, size_t col, const std::string &msg,
                 const std::string & /*rule*/) { log(line, col, msg); };
  }

private:
  bool post_process(const char *s, size_t n, Definition::Result &r) const {
    if ((log_ || error_reporter_) && !r.ret) {
      r.error_info.output_log(log_, error_reporter_, s, n);
    }
    return r.ret && !r.recovered;
  }

  std::vector<std::string> get_no_ast_opt_rules() const {
    std::vector<std::string> rules;
    for (auto &[name, rule] : *grammar_) {
      // The optimizer keeps nodes by their emitted name, so honor the
      // `ast_name` override when present (else the rule's own name).
      if (rule.no_ast_opt) {
        rules.push_back(rule.ast_name.empty() ? name : rule.ast_name);
      }
    }
    return rules;
  }

  std::shared_ptr<Grammar> grammar_;
  std::string start_;
  bool enableLeftRecursion_ = true;
  bool enablePackratParsing_ = false;
  Log log_;
  ErrorReporter error_reporter_;
};

/*-----------------------------------------------------------------------------
 *  enable_tracing
 *---------------------------------------------------------------------------*/

inline void enable_tracing(parser &parser, std::ostream &os) {
  parser.enable_trace(
      [&](auto &ope, auto s, auto, auto &, auto &c, auto &, auto &trace_data) {
        auto prev_pos = std::any_cast<size_t>(trace_data);
        auto pos = static_cast<size_t>(s - c.s);
        auto backtrack = (pos < prev_pos ? "*" : "");
        std::string indent;
        auto level = c.trace_ids.size() - 1;
        while (level--) {
          indent += "│";
        }
        std::string name;
        {
          name = peg::TraceOpeName::get(const_cast<peg::Ope &>(ope));

          auto lit = dynamic_cast<const peg::LiteralString *>(&ope);
          if (lit) { name += " '" + peg::escape_characters(lit->lit_) + "'"; }
        }
        os << "E " << pos + 1 << backtrack << "\t" << indent << "┌" << name
           << " #" << c.trace_ids.back() << std::endl;
        trace_data = static_cast<size_t>(pos);
      },
      [&](auto &ope, auto s, auto, auto &sv, auto &c, auto &, auto len,
          auto &) {
        auto pos = static_cast<size_t>(s - c.s);
        if (len != static_cast<size_t>(-1)) { pos += len; }
        std::string indent;
        auto level = c.trace_ids.size() - 1;
        while (level--) {
          indent += "│";
        }
        auto ret = len != static_cast<size_t>(-1) ? "└o " : "└x ";
        auto name = peg::TraceOpeName::get(const_cast<peg::Ope &>(ope));
        std::stringstream choice;
        if (sv.choice_count() > 0) {
          choice << " " << sv.choice() << "/" << sv.choice_count();
        }
        std::string token;
        if (!sv.tokens.empty()) {
          token += ", token '";
          token += sv.tokens[0];
          token += "'";
        }
        std::string matched;
        if (peg::success(len) &&
            peg::TokenChecker::is_token(const_cast<peg::Ope &>(ope))) {
          matched = ", match '" + peg::escape_characters(s, len) + "'";
        }
        os << "L " << pos + 1 << "\t" << indent << ret << name << " #"
           << c.trace_ids.back() << choice.str() << token << matched
           << std::endl;
      },
      [&](auto &trace_data) { trace_data = static_cast<size_t>(0); },
      [&](auto &) {});
}

/*-----------------------------------------------------------------------------
 *  enable_profiling
 *---------------------------------------------------------------------------*/

inline void enable_profiling(parser &parser, std::ostream &os) {
  struct Stats {
    struct Item {
      std::string name;
      size_t success;
      size_t fail;
    };
    std::vector<Item> items;
    std::map<std::string, size_t> index;
    size_t total = 0;
    std::chrono::steady_clock::time_point start;
  };

  parser.enable_trace(
      [&](auto &ope, auto, auto, auto &, auto &, auto &, std::any &trace_data) {
        if (auto holder = dynamic_cast<const peg::Holder *>(&ope)) {
          auto &stats = *std::any_cast<Stats *>(trace_data);

          auto &name = holder->name();
          if (stats.index.find(name) == stats.index.end()) {
            stats.index[name] = stats.index.size();
            stats.items.push_back({name, 0, 0});
          }
          stats.total++;
        }
      },
      [&](auto &ope, auto, auto, auto &, auto &, auto &, auto len,
          std::any &trace_data) {
        if (auto holder = dynamic_cast<const peg::Holder *>(&ope)) {
          auto &stats = *std::any_cast<Stats *>(trace_data);

          auto &name = holder->name();
          auto index = stats.index[name];
          auto &stat = stats.items[index];
          if (len != static_cast<size_t>(-1)) {
            stat.success++;
          } else {
            stat.fail++;
          }

          if (index == 0) {
            auto end = std::chrono::steady_clock::now();
            auto nano = std::chrono::duration_cast<std::chrono::microseconds>(
                            end - stats.start)
                            .count();
            auto sec = nano / 1000000.0;
            os << "duration: " << sec << "s (" << nano << "µs)" << std::endl
               << std::endl;

            char buff[BUFSIZ];
            size_t total_success = 0;
            size_t total_fail = 0;
            for (auto &[name, success, fail] : stats.items) {
              total_success += success;
              total_fail += fail;
            }

            os << "  id       total      %     success        fail  "
                  "definition"
               << std::endl;

            auto grand_total = total_success + total_fail;
            snprintf(buff, BUFSIZ, "%4s  %10zu  %5s  %10zu  %10zu  %s", "",
                     grand_total, "", total_success, total_fail,
                     "Total counters");
            os << buff << std::endl;

            snprintf(buff, BUFSIZ, "%4s  %10s  %5s  %10.2f  %10.2f  %s", "", "",
                     "", total_success * 100.0 / grand_total,
                     total_fail * 100.0 / grand_total, "% success/fail");
            os << buff << std::endl << std::endl;
            ;

            size_t id = 0;
            for (auto &[name, success, fail] : stats.items) {
              auto total = success + fail;
              auto ratio = total * 100.0 / stats.total;
              snprintf(buff, BUFSIZ, "%4zu  %10zu  %5.2f  %10zu  %10zu  %s", id,
                       total, ratio, success, fail, name.c_str());
              os << buff << std::endl;
              id++;
            }
          }
        }
      },
      [&](auto &trace_data) {
        auto stats = new Stats{};
        stats->start = std::chrono::steady_clock::now();
        trace_data = stats;
      },
      [&](auto &trace_data) {
        auto stats = std::any_cast<Stats *>(trace_data);
        delete stats;
      });
}
} // namespace peg
