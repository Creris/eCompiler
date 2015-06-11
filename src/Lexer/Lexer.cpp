#include "Lexer.hpp"
#include <algorithm>

namespace jh{
	bool compareString(const std::string& input, size_t start, size_t end, const std::string& withWhat)
	{
		//if we are checking past end of input
		//or the range we want to check is not exactly as long as argument provided
		//the latter is because if we compare string with length X to sting with different length
		//they can never be lexicographically equal
		if(end > input.size() || end - start != withWhat.size())
			return false;

		//i < withWhat.size() just in case
		for(int i = 0; start < end && i < withWhat.size(); ++start, ++i)
		{
			if(input[start] != withWhat[i])
				return false;
		}

		return true;
	}

	size_t Lexer::getAfterNewline(const std::string& input, size_t startAt)
	{
		//assert for out of range
		if(startAt < 0 || startAt >= input.size())
			return -1;

		if(input[startAt] == '\n')
			//jump one by default, since there is no newline that has \nX
			return startAt + 1;
		else if(input[startAt] == '\r')
		{
			//if in range and is \n, jump it too
			if(startAt + 1 < input.size() && input[startAt + 1] == '\n')
				return startAt + 2;
			//otherwise(either not \n, or would go out of range), jump one
			else
				return startAt + 1;
		}

		//not even newline, but be kind and return the same position
		return startAt;
	}

	bool Lexer::_isNewline(char c)
	{
		return c == '\n' || c == '\r';
	}

	//returns position of first character after startAt inside input that returns
	//true for isTokenStarting
	size_t Lexer::getNextStartingPos(const std::string& input, size_t startAt)
	{
		size_t end = startAt;

		while(end < input.size())
		{
			if(isTokenStarting(input[end]))
				break;

			++end;
		}

		return end;
	}
		
	//add Id token with contents starting at startAt all the way to another valid token-starting
	//character
	int Lexer::addIdToken(const std::string& input, size_t startAt)
	{
		auto end = getNextStartingPos(input, startAt);
		auto distance = end - startAt;

		tokens.emplace_back(Token::Type::Id, startAt, currentLine, distance);

		return end;
	}

	//checks whether c is potentially a beginning of another token
	bool Lexer::isTokenStarting(char c) const
	{
		//if it is not [0-9a-zA-Z_] it is potentially token starting
		return !(isalnum(c) || c == '_');
	}

	//returns the length of integer literal
	//if it is not real literal, returns 0
	int Lexer::_isIntegerLiteral(const std::string& input, size_t start, size_t end)
	{
		//NOTE!
		//rawcode literals are stored as separate tokens

		//valid integer literals:  octal(regex form:		/0[0-7]*/),
		//						   decimal(regex form:		/[0-9]+/),
		//						   hexadecimal(regex form:	/0[xX][0-9a-fA-F]+/)

		//Bound checking
		//if the first character we want to check is past the string's size or the
		//ending character is lower or equal to start, return 0
		//if it is exactly the last character, return if it is /[0-9]/
		if(start >= input.size() || start >= end)
			return 0;
		else if(start == input.size() - 1)
			return input[start] >= '0' && input[start] <= '9';

		//get the real end, because 'end' can be std::string::npos
		auto realEnd = std::min(input.size(), end);
		bool isHex = tolower(input[start + 1]) == 'x';
		bool isOct = input[start] == '0' && !isHex;

		if(end == std::string::npos)
		{
			//relaxed check
			//check if char is whitespace on every iteration

			//it is potentially hex, check for that
			if(isHex && realEnd - start > 2)
			{
				//input[start] = 0, input[start + 1] = x/X, input[start + 2] = first char to check
				for(int i = start + 2; i < realEnd; ++i)
				{
					char c = tolower(input[i]);
					//if it is smaller than 0, or bigger than 9 but at the same time smaller than a, or
					//bigger than f, it is not hexadecimal, so it is not integer literal
					if(isTokenStarting(c))
						return i - start;
					else if(c < '0' || (c > '9' && (c < 'a' || c > 'f')))
						return 0;
				}
			}
			//it is potentially octal or decimal, check for that
			else
			{
				int barrier = isOct ? 7 : 9;
				for(int i = start; i < realEnd; ++i)
				{
					//check the distance from input[start] to 0
					int distance = input[i] - '0';
					//if it is bigger than barrier or it is negative, it is not literal
					//negative = lower ascii than '0'
					//positive above barrier = either 8/9 inside octal, or invalid character inside decimal
					if(isTokenStarting(input[i]))
						return i - start;
					else if(distance > barrier || distance < 0)
						return 0;
				}
			}
		}
		else
		{
			//it is strict check
			//loop until realEnd

			//it is potentially hex, check for that
			if(isHex && realEnd - start > 2)
			{
				//input[start] = 0, input[start + 1] = x/X, input[start + 2] = first char to check
				for(int i = start + 2; i < realEnd; ++i)
				{
					char c = tolower(input[i]);
					//if it is smaller than 0, or bigger than 9 but at the same time smaller than a, or
					//bigger than f, it is not hexadecimal, so it is not integer literal
					if(c < '0' || (c > '9' && (c < 'a' || c > 'f')))
						return 0;
				}
			}
			//it is potentially octal or decimal, check for that
			else
			{
				int barrier = isOct ? 7 : 9;
				for(int i = start; i < realEnd; ++i)
				{
					//check the distance from input[start] to 0
					int distance = input[i] - '0';
					//if it is bigger than barrier or it is negative, it is not literal
					//negative = lower ascii than '0'
					//positive above barrier = either 8/9 inside octal, or invalid character inside decimal
					if(distance > barrier || distance < 0)
						return 0;
				}
			}
		}

		//return length of integer literal
		return realEnd - start;
	}

	//returns the length of real literal
	//if it is not real literal, returns 0
	int Lexer::_isRealLiteral(const std::string& input, size_t start, size_t end)
	{
		//valid real literals:	Double numbered(0.0), regex:	/[0-9]+\.[0-9]+/
		//						before-numbered(0.),  regex:	/[0-9]+\./
		//						after-numbered(.0),   regex:	/\.[0-9]/

		//Bound checking
		//if the first character we want to check is past the string's size or the
		//ending character is lower or equal to start, return 0
		//if it is exactly the last character, return if it is /[0-9]/
		if(start >= input.size() || start >= end)
			return 0;
		else if(start == input.size() - 1)
			return input[start] >= '0' && input[start] <= '9';

		bool wasBefore = false;
		bool wasAfter = false;
		auto realEnd = std::min(input.size(), end);
		decltype(realEnd) dotAt = std::string::npos;

		if(end == std::string::npos)
		{
			//relaxed check, go to realEnd or next white space

			//check possible numbers before the dot
			for(int i = start; i < realEnd; ++i)
			{
				char c = input[i];

				//if it is dot
				if(c == '.')
				{
					//set boolean to "has any value before dot"
					wasBefore = i != start;
					//dummy
					dotAt = i;
					break;
				}
				//if it is token-init, it is real/integer literal
				if(isTokenStarting(c))
					return i - start;
				//if it is not in range of [0, 9] it is not valid real literal number
				else if(c < '0' || c > '9')
					return 0;
			}

			//check after dot
			{
				//trick to let i live after the for loop, but not in the rest of the script
				int i = dotAt + 1;

				//check before the for loop, to save some
				//cpu work for the evaluation or branch prediction
				//if we are right after dot, and there is something that is not number
				if(input[i] < '0' || input[i] > '9')
				{
					//if there was something before
					//it for sure is real literal
					//if there was nothing, not literal
					return wasBefore ? i - start : 0;
				}

				for(; i < realEnd; ++i)
				{
					char c = input[i];

					//if we found space, return i - start
					//it is real literal only if there was at least one number or dot and one number
					if(isTokenStarting(c))
					{
						//if there was nothing before
						if(!wasBefore)
						{
							//if it failed at first position after dot, it is not real literal
							//otherwise(there was number before), it is real literal
							return i == dotAt + 1 ? 0 : i - start;
						}
						//if there was something before, it automatically is real literal
						else
							return i - start;
					}
					//if it is not in range of [0, 9] it is not valid real literal
					else if(c < '0' || c > '9')
						return 0;

					//if we reach this point, there is at least one number after dot
					wasAfter = true;
				}
			}
		}
		else
		{
			//strict check, go all the way until realEnd

			//check possible numbers before the dot
			for(int i = start; i < realEnd; ++i)
			{
				char c = input[i];

				//if it is dot
				if(c == '.')
				{
					//set boolean to "has any value before dot"
					wasBefore = i != start;
					//dummy
					dotAt = i;
					break;
				}
				//if it is not in range of [0, 9] it is not valid real literal number
				else if(c < '0' || c > '9')
					return 0;
			}

			//check after dot
			{
				//trick to let i live after the for loop, but not in the rest of the script
				int i = dotAt + 1;
				for(; i < realEnd; ++i)
				{
					char c = input[i];

					//if it is not in range of [0, 9] it is not valid real literal
					if(c < '0' || c > '9')
						return 0;

					//if we reach this point, there is at least one number after dot
					wasAfter = true;
				}
			}
		}

		if(wasAfter || wasBefore)
			return realEnd - start;
		else
			return 0;
	}

	std::pair<size_t, size_t> _narrowRange(const std::pair<size_t, size_t>& in,
											const std::string& str, size_t startPos,
											const std::vector<std::string>& keywords, size_t offset)
	{
		std::pair<size_t, size_t> tRet{ -1, in.second };

		for(auto i = in.first; i < in.second; ++i)
		{
			//<=, because we will try to subscript array with offset
			//not offset-1, so it needs to be at least size+1 long
			//because of 0 based indexing
			if(keywords[i].size() <= offset)
				continue;

			if(str[startPos] < keywords[i][offset])
			{
				return { -1, -1 };
			}
			else if(str[startPos] == keywords[i][offset])
			{
				tRet.first = i;
				break;
			}
		}

		for(auto i = tRet.first + 1; i < in.second; ++i)
		{
			if(keywords[i].size() < offset || (keywords[i].size() >= offset &&
												keywords[i][offset] != str[startPos]))
			{
				tRet.second = i;
				break;
			}
		}

		return tRet;
	}

	//Im going to call this, Narrow search
	//Returns the position of what inside keywords
	//if a string starting at inWho is not inside keyword list, returns -1
	size_t Lexer::_findKeyword(const std::string& inWho, size_t startingPos,
								const Lexer::KeywordList& keywords)
	{
		auto endPos = getNextStartingPos(inWho, startingPos);
		std::pair<size_t, size_t> keywordsRange{ 0, keywords.size() };

		auto realStart = startingPos;

		auto iters = 0;
		for(int i = 0; startingPos < endPos; ++startingPos, ++i)
		{
			keywordsRange = _narrowRange(keywordsRange, inWho, startingPos, keywords, i);
				
			if(keywordsRange.first == -1)
				return -1;
			else if(std::abs(static_cast<int>(keywordsRange.first - keywordsRange.second)) == 1)
			{
				//at this point, there is only one candidate, but it is not
				//guaranteed to be it
				//consider: contents of keywords: [a bb c], searching for: b
				//would say it is equal to bb, because the distance is == 1
				//so we need to additionally check for equality
				if(compareString(inWho, realStart, endPos, keywords[keywordsRange.first]))
					//if it is equal
					return keywordsRange.first;
				else
					//if not so much
					return -1;
			}

			++iters;
		}

		//if we iterated over every char
		if(iters == endPos - realStart &&
			compareString(inWho, realStart, endPos, keywords[keywordsRange.first]))
		{
			//checking for one should be enough
			//since if the first considerable is not completly equal, the rest
			//wont be just as much
			return keywordsRange.first;
				
			/*
			//we go and check the whole remaining list
			for(; keywordsRange.first < keywordsRange.second; ++keywordsRange.first)
			{
				//if we find exact match, return it
				if(compareString(inWho, realStart, endPos, keywords[keywordsRange.first]))
				{
					return keywordsRange.first;
				}
			}*/
		}

		//fallback
		return -1;
	}

	//Keywords needs to be sorted for Narrow search to work properly
	//otherwise, good bye code
	Lexer::TokenList& Lexer::tokenize(const std::string& input,
			const Lexer::KeywordList& keywords, const Lexer::TokenType& exampleTokens)
	{
		size_t curPos = 0;

		while(curPos < input.size())
		{
			auto& c = input[curPos];

			//if it is alphabetic, we can take this path, and optimize it a bit
			if(isalpha(c))
			{
				auto v = _findKeyword(input, curPos, keywords);

				if(v == -1)
				{
					curPos = addIdToken(input, curPos);
				}
				else
				{
					tokens.emplace_back(exampleTokens[v], curPos, currentLine);
					curPos = getNextStartingPos(input, curPos);
				}
			}
			//otherwise, it is special token, do some
			//less optimized(or more, depends on angle of view) checkings here
			else
			{
				//possible token-characters
				//< > = ! % ( ) [ ] { } + - * / , . ; : [numbers] [line-ending] ' " #
				//basically everything that is not starting with letter

				//switch has potential to generate a jump table instead of
				//what general chained if would generate
				switch(c)
				{
					case '+':
					{
						//can be +, ++ or +=
						if(curPos + 1 < input.size())
						{
							if(input[curPos + 1] == '+')
							{
								//++
								tokens.emplace_back(Token::Type::Operator_increment, curPos, currentLine);
								curPos += 2;
							}
							else if(input[curPos + 1] == '=')
							{
								//+=
								tokens.emplace_back(Token::Type::Operator_eqplus, curPos, currentLine);
								curPos += 2;
							}
							else
							{
								//+
								tokens.emplace_back(Token::Type::Operator_plus, curPos, currentLine);
								++curPos;
							}
						}
						else
						{
							//since this is last character of the input, it can only be +
							tokens.emplace_back(Token::Type::Operator_plus, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '\r':
					case '\n':
					{
						//push it, and calculate the offset
						tokens.emplace_back(Token::Type::Operator_newline, curPos, currentLine);
						curPos = getAfterNewline(input, curPos);

						++currentLine;
						break;
					}

					//if it is tab or space, do nothing with it
					case '\t':
					case ' ':
						++curPos;
						break;

					case '-':
					{
						//can be -, -- or -=
						if(curPos + 1 < input.size())
						{
							if(input[curPos + 1] == '-')
							{
								//--
								tokens.emplace_back(Token::Type::Operator_decrement, curPos, currentLine);
								curPos += 2;
							}
							else if(input[curPos + 1] == '=')
							{
								//-=
								tokens.emplace_back(Token::Type::Operator_eqminus, curPos, currentLine);
								curPos += 2;
							}
							else
							{
								//-
								tokens.emplace_back(Token::Type::Operator_minus, curPos, currentLine);
								++curPos;
							}
						}
						else
						{
							//since this is last char of the input, it is for sure single -
							tokens.emplace_back(Token::Type::Operator_minus, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '*':
					{
						if(curPos + 1 < input.size() && input[curPos + 1] == '=')
						{
							//*=
							tokens.emplace_back(Token::Type::Operator_eqmultiply, curPos, currentLine);
							curPos += 2;
						}
						else
						{
							//in here we are guaranteed it is single *
							tokens.emplace_back(Token::Type::Operator_multiply, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '/':
					{
						if(curPos + 1 < input.size())
						{
							// /*
							if(input[curPos + 1] == '*')
							{
								//to evade the current /*
								curPos += 2;
								size_t length = 0;
								size_t starting = curPos;
								size_t startingLine = currentLine;

								size_t matchCount = 1;

								//find matching */
								while(curPos + 1 < input.size())
								{
									//if it is */, lower the number of nested comment blocks
									if(input[curPos] == '*' && input[curPos + 1] == '/')
										--matchCount;
									//if it is /*, increase the number of nested comment blocks
									else if(input[curPos] == '/' && input[curPos + 1] == '*')
										++matchCount;

									//if we found our end
									if(!matchCount)
										break;

									if(_isNewline(input[curPos]))
									{
										++currentLine;
										curPos = getAfterNewline(input, curPos);
									}
									else
										++curPos;

									//even if there is \r\n, consider it single line
									++length;
								}

								//because even with ++ it would still point to the /
								//and on next iteration we would get Token::Type::Operator_divide
								//and we dont want that, do we
								curPos += 2;

								//NOTE: Token::Type::Operator_dComment stores all its nested dComments too!
								tokens.emplace_back(Token::Type::Operator_dComment,
									starting, startingLine, length);
							}
							// /=
							else if(input[curPos + 1] == '=')
							{
								tokens.emplace_back(Token::Type::Operator_eqdivide, curPos, currentLine);
								curPos += 2;
							}
							// //
							else if(input[curPos + 1] == '/')
							{
								// //!
								if(curPos + 2 < input.size() && input[curPos + 2] == '!')
								{
									curPos += 3;
									size_t length = 0;
									size_t starting = curPos;

									while(curPos < input.size() && input[curPos] != '\n')
									{
										curPos++;
										++length;
									}

									tokens.emplace_back(Token::Type::Operator_preprocessor, starting,
										currentLine, length);
								}
								else
								{
									// //
									while(curPos < input.size() && input[curPos] != '\n')
									{
										curPos++;
									}

									//do nothing here, since we dont have token for comment
								}
							}
							else
							{
								tokens.emplace_back(Token::Type::Operator_divide, curPos, currentLine);
								++curPos;
							}
						}
						//last character in the file, probably invalid, but nevertheless tokenize it
						else
						{
							tokens.emplace_back(Token::Type::Operator_divide, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '<':
					{
						//could be either <, <=, <<, <<=
						if(curPos + 1 < input.size() && input[curPos + 1] == '<')
						{
							//it is either << or <<=
							if(curPos + 2 < input.size() && input[curPos + 2] == '=')
							{
								//<<= for sure
								tokens.emplace_back(Token::Type::Operator_eqlshift, curPos, currentLine);

								//evade the < and =, point to the next char
								curPos += 3;
							}
							else
							{
								//<< for sure
								tokens.emplace_back(Token::Type::Operator_lshift, curPos, currentLine);

								//evade < and point to next char
								curPos += 2;
							}
						}
						//<= for sure
						else if(curPos + 1 < input.size() && input[curPos + 1] == '=')
						{
							tokens.emplace_back(Token::Type::Operator_lessequal, curPos, currentLine);
							curPos += 2;
						}
						else
						{
							//< for sure
							tokens.emplace_back(Token::Type::Operator_less, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '>':
					{
						//could be either >, >=, >> or >>=
						if(curPos + 1 < input.size() && input[curPos + 1] == '>')
						{
							//it is either >> or >>=
							if(curPos + 2 < input.size() && input[curPos + 2] == '=')
							{
								//>>= for sure
								tokens.emplace_back(Token::Type::Operator_eqrshift, curPos, currentLine);

								//evade the > and =, point to the next char
								curPos += 3;
							}
							else
							{
								//>> for sure
								tokens.emplace_back(Token::Type::Operator_rshift, curPos, currentLine);

								//evade > and point to next char
								curPos += 2;
							}
						}
						//>=
						else if(curPos + 1 < input.size() && input[curPos + 1] == '=')
						{
							tokens.emplace_back(Token::Type::Operator_biggerequal, curPos, currentLine);
							curPos += 2;
						}
						else
						{
							//> for sure
							tokens.emplace_back(Token::Type::Operator_bigger, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '=':
					{
						//can be = or ==
						if(curPos + 1 < input.size() && input[curPos + 1] == '=')
						{
							//==
							tokens.emplace_back(Token::Type::Operator_equal, curPos, currentLine);
							curPos += 2;
						}
						else
						{
							//=
							tokens.emplace_back(Token::Type::Operator_assign, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '!':
					{
						//can be ! or !=
						if(curPos + 1 < input.size() && input[curPos + 1] == '=')
						{
							//!=
							tokens.emplace_back(Token::Type::Operator_notequal, curPos, currentLine);
							curPos += 2;
						}
						else
						{
							//! is exactly equal to not, so we can store that no problem
							tokens.emplace_back(Token::Type::Keyword_not, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '%':
					{
						//can be % or %=
						if(curPos + 1 < input.size() && input[curPos + 1] == '=')
						{
							//%=
							tokens.emplace_back(Token::Type::Operator_eqmodulo, curPos, currentLine);
							curPos += 2;
						}
						else
						{
							//%
							tokens.emplace_back(Token::Type::Operator_modulo, curPos, currentLine);
							++curPos;
						}

						break;
					}

					case '(':
					{
						//(
						tokens.emplace_back(Token::Type::Operator_LPar, curPos, currentLine);
						++curPos;

						break;
					}

					case ')':
					{
						//)
						tokens.emplace_back(Token::Type::Operator_RPar, curPos, currentLine);
						++curPos;

						break;
					}

					case '[':
					{
						//[
						tokens.emplace_back(Token::Type::Operator_LBPar, curPos, currentLine);
						++curPos;

						break;
					}

					case ']':
					{
						//]
						tokens.emplace_back(Token::Type::Operator_RBPar, curPos, currentLine);
						++curPos;

						break;
					}

					case '{':
					{
						//{
						tokens.emplace_back(Token::Type::Operator_LCPar, curPos, currentLine);
						++curPos;

						break;
					}

					case '}':
					{
						//}
						tokens.emplace_back(Token::Type::Operator_RCPar, curPos, currentLine);
						++curPos;

						break;
					}

					case ',':
					{
						//,
						tokens.emplace_back(Token::Type::Operator_comma, curPos, currentLine);
						++curPos;

						break;
					}

					case '.':
					{
						//.
						tokens.emplace_back(Token::Type::Operator_dot, curPos, currentLine);
						++curPos;

						break;
					}

					case ';':
					{
						//;
						tokens.emplace_back(Token::Type::Operator_semi, curPos, currentLine);
						++curPos;

						break;
					}

					case '\'':
					{
						//'
						++curPos;
						size_t length = 0;
						size_t starting = curPos;
						size_t startingLine = currentLine;

						while(curPos < input.size() && input[curPos] != '\'')
						{
							if(_isNewline(input[curPos]))
							{
								++currentLine;
								curPos = getAfterNewline(input, curPos);
							}
							else
							{
								++curPos;
							}
								
							//even tho it could be \r\n, still consider it as one char
							++length;
						}

						tokens.emplace_back(Token::Type::Operator_rawcode, starting, startingLine, length);
						++curPos;

						break;
					}

					case '\"':
					{
						//"
						++curPos;
						size_t length = 0;
						size_t starting = curPos;
						size_t startingLine = currentLine;

						while(curPos < input.size())
						{
							//potentially end, but not if we find \", but end if we found \\"
							if(input[curPos] == '\"')
							{
								if(curPos > 2)
								{
									//if it has both \\, or the one last char is not \, it is
									//for sure end
									if(input[curPos - 1] == '\\' && input[curPos - 2] == '\\' ||
										input[curPos - 1] != '\\')
										break;
								}
								//thanks to ++curPos in the beginning of this branch
								//we are guaranteed to have curPos at least 1
								else
								{
									//if the preceeding character is not \, we know this
									//is the end of the string for us
									if(input[curPos - 1] != '\\')
										break;
								}
							}

							if(_isNewline(input[curPos]))
							{
								++currentLine;
								curPos = getAfterNewline(input, curPos);
							}
							else
							{
								++curPos;
							}

							//even tho it could be \r\n, still consider it as one char
							++length;
						}

						tokens.emplace_back(Token::Type::Operator_string, starting, startingLine, length);
						++curPos;

						break;
					}

					case '#':
					{
						//#
						if(compareString(input, curPos, curPos + 3, "#if"))
						{
							//#if
							tokens.emplace_back(Token::Type::Keyword_hashif, curPos, currentLine);
							curPos += 3;
						}
						else if(compareString(input, curPos, curPos + 5, "#else"))
						{
							//#else
							tokens.emplace_back(Token::Type::Keyword_hashelse, curPos, currentLine);
							curPos += 5;
						}
						else if(compareString(input, curPos, curPos + 7, "#elseif"))
						{
							//#elseif
							tokens.emplace_back(Token::Type::Keyword_hashelseif, curPos, currentLine);
							curPos += 7;
						}
						else if(compareString(input, curPos, curPos + 6, "#endif"))
						{
							//#endif
							tokens.emplace_back(Token::Type::Keyword_hashendif, curPos, currentLine);
							curPos += 6;
						}
						else
						{
							//invalid token, but let parser catch this
							curPos = addIdToken(input, curPos);
						}

						break;
					}

					case '$':
					{
						//textmacro argument
						curPos++;
						size_t starting = curPos;
						size_t startingLine = currentLine;
						size_t length = 0;
						while(curPos < input.size() && input[curPos] != '$')
						{
							if(_isNewline(input[curPos]))
							{
								++currentLine;
								curPos = getAfterNewline(input, curPos);
							}
							else
							{
								++curPos;
							}

							//even tho it could be \r\n, still consider it as one char
							++length;
						}

						++curPos;
						tokens.emplace_back(Token::Type::Operator_textmacroarg,
							starting, startingLine, length);
					}

					default:
					{
						//either integer or real literal, or just Id
						size_t length = _isIntegerLiteral(input, curPos);

						if(length)
						{
							if(curPos + length < input.size() && input[curPos + length] == '.')
							{
								//is real
								length = _isRealLiteral(input, curPos);
								tokens.emplace_back(Token::Type::Literal_real, curPos, currentLine, length);
							}
							else
							{
								//it is integer
								tokens.emplace_back(Token::Type::Literal_int, curPos, currentLine, length);
							}

							curPos += length;
						}
						else
						{
							//it is Id, invalid, but let parser catch this
							curPos = addIdToken(input, curPos);
						}
					}
				}
			}
		}

		return tokens;
	}

	const Lexer::TokenList& Lexer::getTokens() const
	{
		return tokens;
	}
}