/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_ASM_TAB_H_INCLUDED
# define YY_YY_ASM_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    INCLUDE = 258,
    IF = 259,
    DEFINED = 260,
    MACRO = 261,
    MACRO_STRING = 262,
    ORG = 263,
    ERROR = 264,
    ECHO1 = 265,
    INCBIN = 266,
    INCLEN = 267,
    INCWORD = 268,
    RES = 269,
    WORD = 270,
    BYTE = 271,
    LDA = 272,
    LDX = 273,
    LDY = 274,
    STA = 275,
    STX = 276,
    STY = 277,
    AND = 278,
    ORA = 279,
    EOR = 280,
    ADC = 281,
    SBC = 282,
    CMP = 283,
    CPX = 284,
    CPY = 285,
    TSX = 286,
    TXS = 287,
    PHA = 288,
    PLA = 289,
    PHP = 290,
    PLP = 291,
    SEI = 292,
    CLI = 293,
    NOP = 294,
    TYA = 295,
    TAY = 296,
    TXA = 297,
    TAX = 298,
    CLC = 299,
    SEC = 300,
    RTS = 301,
    JSR = 302,
    JMP = 303,
    BEQ = 304,
    BNE = 305,
    BCC = 306,
    BCS = 307,
    BPL = 308,
    BMI = 309,
    BVC = 310,
    BVS = 311,
    INX = 312,
    DEX = 313,
    INY = 314,
    DEY = 315,
    INC = 316,
    DEC = 317,
    LSR = 318,
    ASL = 319,
    ROR = 320,
    ROL = 321,
    BIT = 322,
    SYMBOL = 323,
    STRING = 324,
    LAND = 325,
    LOR = 326,
    LNOT = 327,
    LPAREN = 328,
    RPAREN = 329,
    COMMA = 330,
    COLON = 331,
    X = 332,
    Y = 333,
    HASH = 334,
    PLUS = 335,
    MINUS = 336,
    MULT = 337,
    DIV = 338,
    MOD = 339,
    LT = 340,
    GT = 341,
    EQ = 342,
    NEQ = 343,
    ASSIGN = 344,
    GUESS = 345,
    NUMBER = 346,
    vNEG = 347
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 145 "asm.y" /* yacc.c:1909  */

    i32 num;
    char *str;
    struct atom *atom;
    struct expr *expr;

#line 154 "asm.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ASM_TAB_H_INCLUDED  */
