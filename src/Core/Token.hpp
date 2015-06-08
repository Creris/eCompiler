#ifndef _JH_HEADER_TOKEN_
#define _JH_HEADER_TOKEN_

namespace jh{
	class Token{
	public:
		enum class Type{
			Literal_null = 1,				//null
			Literal_bool_true,				//true
			Literal_bool_false,				//false
			Literal_int,					//0, 1, 2, 54987, -45632
			Literal_real,					//2.5, 2., .2
			
			/*
				vJass
			*/
			Literal_super,					//super
			Literal_this,					//this
			
			/*
				Jass
			*/
			Keyword_function,				//function
			Keyword_endfunction,			//endfunction
			Keyword_globals,				//globals
			Keyword_endglobals,				//endglobals
			Keyword_array,
			Keyword_takes,					//takes
			Keyword_returns,				//returns
			Keyword_local,					//local
			Keyword_set,					//set
			Keyword_call,					//call
			Keyword_if,						//if x
			Keyword_then,					//then
			Keyword_else,					//else
			Keyword_elseif,					//elseif X
			Keyword_endif,					//endif
			Keyword_loop,					//loop
			Keyword_exitwhen,				//exitwhen COND
			Keyword_endloop,				//endloop
			Keyword_constant,				//constant
			Keyword_return,					//return
			Keyword_or,						//or
			Keyword_and,					//and
			Keyword_not,					//not
			Keyword_extends,				//extends
			
			Keyword_native,					//native
			Keyword_type,					//type
			
			/*
				vJass
			*/
			Keyword_debug,					//debug
			Keyword_library,				//library
			Keyword_endlibrary,				//endlibrary
			Keyword_uses,					//uses
			Keyword_requires,				//requires
			Keyword_needs,					//needs
			Keyword_initializer,			//initializer
			Keyword_optional,				//optional
			Keyword_scope,					//scope
			Keyword_endscope,				//endscope
			Keyword_struct,					//struct
			Keyword_class,					//class
			Keyword_endstruct,				//endstruct
			Keyword_endclass,				//endclass
			Keyword_method,					//method
			Keyword_endmethod,				//endmethod
			Keyword_operator,				//method operator
			Keyword_static,					//static
			Keyword_private,				//private
			Keyword_public,					//public
			Keyword_readonly,				//readonly
			Keyword_temporary,				//temporary
			Keyword_template,				//template
			Keyword_alias,					//alias
			Keyword_inline,					//inline
			Keyword_deprecated,				//deprecated
			Keyword_textmacro,				//textmacro
			Keyword_runtextmacro,			//runtextmacro
			Keyword_endtextmacro,			//endtextmacro
			Keyword_module,					//module
			Keyword_endmodule,				//endmodule
			Keyword_external,				//external
			Keyword_endexternal,			//endexternal
			Keyword_externalblock,			//externalblock
			Keyword_endexternalblock,		//endexternalblock
			Keyword_interface,				//interface
			Keyword_endinterface,			//endinterface
			Keyword_defaults,				//defaults
			Keyword_stub,					//stub
			Keyword_sizeof,					//sizeof(not implemented)
			Keyword_static_assert,			//static_assert
			Keyword_compiletime,			//compiletime
			Keyword_while,					//while
			Keyword_endwhile,				//endwhile
			Keyword_break,					//break
			Keyword_for,					//for
			Keyword_endfor,					//endfor
			Keyword_implement,				//implement
			Keyword_catch,					//catch
			Keyword_thistype,				//thistype
			Keyword_final,					//final
			Keyword_hashif,					//#if
			Keyword_hashelseif,				//#elseif
			Keyword_hashelse,				//#else
			Keyword_hashendif,				//#endif
			Keyword_auto,					//auto
			Keyword_override,				//override
			Keyword_mutable,				//mutable
			Keyword_endblock,				//endblock
			Keyword_allocator,				//allocator
			Keyword_endallocator,			//endallocator
			Keyword_using,					//using
			Keyword_hook,					//hook
			Keyword_before,					//before
			Keyword_after,					//after
			Keyword_constructor,			//constructor
			Keyword_endconstructor,			//endconstructor
			Keyword_construct,				//construct
			Keyword_destructor,				//destructor
			Keyword_enddestructor,			//enddestructor
			Keyword_import,					//import
			Keyword_encrypted,				//encrypted
			Keyword_priority,				//priority
			Keyword_extendor,				//extendor
			Keyword_endextendor,			//endextendor
			Keyword_concept,				//concept
			Keyword_endconcept,				//endconcept
			
			/*
				Jass
			*/
			Operator_newline,				//\n, \r\n, \r
			Operator_plus,					//+
			Operator_minus,					//-
			Operator_multiply,				//*
			Operator_divide,				// /
			Operator_LPar,					//(
			Operator_RPar,					//)
			Operator_LBPar,					//[
			Operator_RBPar,					//]
			Operator_assign,				//=
			Operator_equal,					//==
			Operator_notequal,				//!=
			Operator_less,					//<
			Operator_bigger,				//>
			Operator_biggerequal,			//>=
			Operator_lessequal,				//<=
			Operator_rawcode,				//'
			Operator_string,				//"
			Operator_comma,					//,
			
			/*
				vJass
			*/
			Operator_modulo,				//%
			Operator_rshift,				//>>
			Operator_lshift,				//<<
			Operator_eqplus,				//+=
			Operator_eqminus,				//-=
			Operator_eqmultiply,			//*=
			Operator_eqdivide,				///=
			Operator_eqmodulo,				//%=
			Operator_eqrshift,				//>>=
			Operator_eqlshift,				//<<=
			Operator_increment,				//++
			Operator_decrement,				//--
			Operator_dComment,				///* block block block */
			Operator_preprocessor,			// //!
			Operator_LCPar,					//{
			Operator_RCPar,					//}
			Operator_semi,					//;
			Operator_dot,					//.
			Operator_textmacroarg,			//$ARG$

			/*
				UNIVERSAL
			*/
			Id								//any identifier that did not fit other type

			/*
				Tokens with 'length' set:
					Operator_preprocessor - up until next line
					Operator_rawcode - up until ending, the ending operator is not included
					Operator_string - up until ending, the ending operator is not included
					Id - the identifier's lenght in given string
					Operator_dComment - stores the length of the block.
					Operator_textmacroarg - stores the length of argument(text between 2 $)

					In all cases 'line' stores the line that opening of the token lies in.
			*/
		}type;
		
		//first position of token in the file, the last position
		//can be calculated as (position + raw.size() - 1)
		int position;

		//the length of raw value
		int length;

		//line given token is declared at
		//for Operator_newline, this is always the line that the '\n' is placed at
		int line;

		Token(Token::Type t, int pos, int lin, int len = 0) : type(t), length(len), position(pos), line(lin)
		{
		}
	};
}

#endif	//_JH_HEADER_TOKEN_