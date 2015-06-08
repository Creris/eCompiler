#ifndef _JH_HEADER_LEXER2_
#define _JH_HEADER_LEXER2_

#include <string>
#include <vector>
#include "../Core/Token.hpp"

namespace jh{
	namespace expr{
		//end should always be one higher than the last character we want to check
		bool compareString(const std::string& input, size_t start, size_t end, const std::string& withWhat);

		class Lexer{
		public:
			using TokenList = std::vector<Token>;
			using IdPair = std::pair<size_t, size_t>;
			using KeywordList = std::vector<std::string>;
			using TokenType = std::vector<Token::Type>;
		private:
			TokenList tokens;
			size_t currentLine = 1;

			//returns the position after evading complete newline(1 and only 1)
			size_t getAfterNewline(const std::string& input, size_t startAt);

			//returns whether given char is considered new line or not
			bool _isNewline(char c);

			//checks whether c is potentially a beginning of another token
			bool isTokenStarting(char c) const;

			//returns position of first character after startAt inside input that returns
			//true for isTokenStarting
			size_t getNextStartingPos(const std::string& input, size_t startAt);

			//add Id token with contents starting at startAt all the way to another valid token-starting
			//character
			int addIdToken(const std::string& input, size_t startAt);

			/*
			Checks whether input at [start, end-1] is integer literal(not rawcode,
			this is stored separately)(end should be one bigger than last character we want to check)

			If end == std::string::npos, checks until a white space is found, otherwise
			checks the range strictly

			Returns the length of given integer literal, so 0 if it is not
			*/
			int _isIntegerLiteral(const std::string& input, size_t start, size_t end = std::string::npos);

			/*
			Checks whether input at [start, end-1] is real literal(end should be one bigger than
			last character we want to check)

			If end == std::string::npos, checks until a white space is found,
			otherwise checks the range strictly

			Returns the length of given integer literal, so 0 if it is not

			Every integer literal is also real literal
			*/
			int _isRealLiteral(const std::string& input, size_t start, size_t end = std::string::npos);

			//finds substring of what starting at startingPos up to next token separating
			//character inside keywords
			size_t _findKeyword(const std::string& what, size_t startingPos,
								const Lexer::KeywordList& keywords);
		public:
			//default constructor for Lexer
			Lexer() = default;

			//Lexer shall not be copyable or movable, since there is no need to do so,
			//therefore, disable copy and move constructor and assignment
			Lexer(const Lexer&) = delete;
			Lexer& operator=(const Lexer&) = delete;
			Lexer(Lexer&&) = delete;
			Lexer& operator=(Lexer&&) = delete;

			//tokenize given input
			//result is stored inside internal memory buffer, and after tokenizing is also returned
			TokenList& tokenize(const std::string& input,
								const KeywordList& keywords,
								const Lexer::TokenType& exampleTokens);

			//returns constant reference to the underlying memory buffer which holds
			//all so far parsed tokens
			const TokenList& getTokens() const;
		};
	}
}

#endif	//_JH_HEADER_LEXER2_