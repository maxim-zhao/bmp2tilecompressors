/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 28 "asm.y" /* yacc.c:339  */

#include "int.h"
#include "parse.h"
#include "vec.h"
#include "membuf.h"
#include "log.h"
#include <stdio.h>
#define YYERROR_VERBOSE

static struct vec asm_atoms[1];

/* prototypes to silence compiler warnings */
int yylex(void);
void yyerror(const char *s);


#line 83 "asm.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "asm.tab.h".  */
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
#line 145 "asm.y" /* yacc.c:355  */

    i32 num;
    char *str;
    struct atom *atom;
    struct expr *expr;

#line 223 "asm.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_ASM_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 238 "asm.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  217
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   626

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  93
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  17
/* YYNRULES -- Number of rules.  */
#define YYNRULES  198
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  319

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   347

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   180,   180,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   191,   192,   194,   195,   196,   197,
     198,   200,   202,   205,   206,   208,   209,   210,   211,   212,
     213,   214,   215,   217,   218,   219,   220,   221,   223,   224,
     225,   226,   227,   229,   230,   231,   232,   233,   234,   235,
     237,   238,   239,   241,   242,   243,   245,   246,   247,   248,
     249,   250,   251,   252,   254,   255,   256,   257,   258,   259,
     260,   261,   263,   264,   265,   266,   267,   268,   269,   270,
     272,   273,   274,   275,   276,   277,   278,   279,   281,   282,
     283,   284,   285,   286,   287,   288,   290,   291,   292,   293,
     294,   295,   296,   297,   299,   300,   301,   302,   303,   304,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   334,   335,   336,   337,
     339,   340,   341,   342,   344,   345,   346,   347,   349,   350,
     351,   352,   353,   355,   356,   357,   358,   359,   361,   362,
     363,   364,   365,   367,   368,   369,   370,   371,   373,   374,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   386,
     387,   388,   389,   390,   391,   392,   393,   394,   396,   397,
     399,   400,   401,   402,   403,   404,   405,   406,   408
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INCLUDE", "IF", "DEFINED", "MACRO",
  "MACRO_STRING", "ORG", "ERROR", "ECHO1", "INCBIN", "INCLEN", "INCWORD",
  "RES", "WORD", "BYTE", "LDA", "LDX", "LDY", "STA", "STX", "STY", "AND",
  "ORA", "EOR", "ADC", "SBC", "CMP", "CPX", "CPY", "TSX", "TXS", "PHA",
  "PLA", "PHP", "PLP", "SEI", "CLI", "NOP", "TYA", "TAY", "TXA", "TAX",
  "CLC", "SEC", "RTS", "JSR", "JMP", "BEQ", "BNE", "BCC", "BCS", "BPL",
  "BMI", "BVC", "BVS", "INX", "DEX", "INY", "DEY", "INC", "DEC", "LSR",
  "ASL", "ROR", "ROL", "BIT", "SYMBOL", "STRING", "LAND", "LOR", "LNOT",
  "LPAREN", "RPAREN", "COMMA", "COLON", "X", "Y", "HASH", "PLUS", "MINUS",
  "MULT", "DIV", "MOD", "LT", "GT", "EQ", "NEQ", "ASSIGN", "GUESS",
  "NUMBER", "vNEG", "$accept", "stmts", "stmt", "atom", "exprs", "op",
  "am_im", "am_a", "am_ax", "am_ay", "am_zp", "am_zpx", "am_zpy", "am_ix",
  "am_iy", "expr", "lexpr", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347
};
# endif

#define YYPACT_NINF -98

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-98)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     352,   -68,   -55,   -15,   -98,   -13,     5,     8,    17,    24,
      35,    38,    14,    21,    28,    49,    64,    73,    14,    14,
      14,    14,    14,    14,    42,    42,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,    79,    79,    79,    79,    79,    79,    79,    79,
      79,    79,   -98,   -98,   -98,   -98,    73,    73,    73,    73,
      73,    73,   116,   -61,   286,   -98,   -98,   -98,    45,     7,
      47,    79,    51,    57,    62,    79,    79,    79,    65,    66,
     -98,    79,    79,    79,    79,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -59,    79,    79,   -98,   -98,   -98,
     -98,   -98,   -45,   -98,   -98,   -98,   -98,   -98,   485,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   141,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,    79,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,   -98,
     -98,   -98,   -98,   -98,   -98,    79,    79,   -98,   -98,    44,
      69,     7,     7,   520,   -57,    70,   437,    74,   -43,   -32,
     495,   -24,   141,    -1,    81,    82,   421,   141,   -98,   505,
      26,    79,    79,    79,    79,    79,   448,   515,    75,    80,
     141,   141,   141,   -98,    88,   -98,   184,   -18,    79,    79,
      79,    79,     7,     7,   -98,   -98,   -98,   -98,   -98,    79,
     -98,    79,    79,   -98,    79,   -98,    87,    68,    92,    85,
      86,   -98,   -98,   -16,   -16,   -98,   -98,   -98,   -98,    90,
      95,   -98,   141,   141,   141,   141,   -98,   118,    61,   432,
     453,   141,   -98,    79,   108,   124,   -98,   -98,   -98,   -98,
     -98,    79,   -98,   464,   -98,   -98,   475,   -98,   -98
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   111,   110,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   136,   137,   138,   139,     0,     0,   148,   153,
     158,   163,     0,     0,     0,     3,    14,    16,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,     0,     0,     0,     0,   188,    25,    28,    29,    30,
      26,    27,    31,    32,   171,     0,     0,    33,    36,    37,
      34,    35,   171,    38,    41,    42,    39,    40,   171,    45,
      46,    47,    43,    44,    48,    49,    52,    50,    51,   171,
      55,    53,    54,    56,    59,    60,    61,    57,    58,    62,
      63,    64,    67,    68,    69,    65,    66,    70,    71,    72,
      75,    76,    77,    73,    74,    78,    79,    80,    83,    84,
      85,    81,    82,    86,    87,    88,    91,    92,    93,    89,
      90,    94,    95,    96,    99,   100,   101,    97,    98,   102,
     103,     0,   104,   106,   105,   107,   109,   108,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   142,   143,
     140,   141,   146,   147,   144,   145,   151,   152,   149,   150,
     156,   157,   154,   155,   161,   162,   159,   160,   166,   167,
     164,   165,   169,   168,     4,     0,     0,     1,     2,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    24,     0,     0,     0,     0,   170,   184,   174,
       0,     0,     0,     0,     0,     0,     0,   174,     0,     0,
     174,     5,     6,    12,     0,   192,     0,     0,     0,     0,
       0,     0,     0,     0,     7,    13,     8,     9,    10,     0,
      20,     0,     0,    18,     0,    19,     0,     0,   185,     0,
       0,   172,   173,   179,   180,   181,   182,   183,   185,     0,
       0,   193,   194,   195,   196,   197,   191,   190,     0,     0,
       0,    23,   186,     0,     0,     0,   175,   176,   198,    11,
      21,     0,    17,     0,   178,   177,     0,   187,    22
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -98,   -98,   135,   -98,   -73,   -98,   469,   408,   420,   172,
     158,   220,   186,   594,   603,   -12,   -97
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    64,    65,    66,   231,    67,    86,    87,    88,    89,
      90,    91,   101,    92,    93,   119,   224
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint16 yytable[] =
{
      94,   102,   108,    94,   233,    68,    94,    94,    94,    94,
      94,    94,   220,   262,   263,   214,   240,   264,    69,    78,
      79,   241,   242,   243,   244,   245,    78,    79,   215,   216,
     248,   268,   269,    78,    79,   241,   242,   243,   244,   245,
      78,    79,   270,   271,   108,   108,   108,   108,   108,   108,
     273,   274,   262,   263,    78,    79,   291,   223,    70,   226,
      71,    78,    79,   230,   232,   232,   243,   244,   245,   236,
     237,   238,   239,   275,   274,    80,    78,    79,    72,   221,
     222,    73,    80,   246,   247,    78,    79,    81,    83,    80,
      74,    78,    79,    82,    95,    83,    80,    75,    85,    84,
      82,    95,    83,   281,   282,    85,    96,    82,    76,    83,
      80,    77,    85,    84,   219,    95,   225,    80,   253,    85,
     227,    82,    81,    83,   255,   257,   228,   171,    78,    79,
      83,   229,    80,    85,    84,   309,   274,    95,   234,   235,
      85,    80,   254,   303,   265,    83,    95,    80,   267,    96,
     276,   277,    95,   282,    83,    85,   290,   281,    84,   250,
      83,   302,   305,   306,    85,   296,   297,   304,   307,   308,
      85,   100,   106,   112,   117,   121,   127,   135,   143,   151,
     159,   167,   174,   177,    80,    99,   314,   111,   262,    95,
     126,   134,   142,   150,   158,   166,   298,    83,   315,   218,
       0,   171,   118,   251,   252,     0,     0,    85,     0,   223,
     256,     0,     0,     0,   190,   194,   198,   202,   206,   210,
     213,   241,   242,   243,   244,   245,     0,     0,     0,   283,
     284,   285,   286,   287,   107,   113,     0,   122,   128,   136,
     144,   152,   160,   168,     0,     0,   292,   293,   294,   295,
     223,   223,     0,     0,     0,     0,     0,   232,   288,   299,
     300,     0,   301,     0,   241,   242,   243,   244,   245,   258,
     259,   260,   261,     0,     0,     0,   191,   195,   199,   203,
     207,   211,     0,     0,     0,     0,   217,     0,     0,     1,
       2,   313,     3,     4,     5,     6,     7,     8,     0,   316,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,     1,     2,     0,     3,     4,
       5,     6,     7,     8,     0,     0,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    98,   104,   109,   116,   120,   124,   132,   140,   148,
     156,   164,   173,   176,   105,   110,     0,     0,   125,   133,
     141,   149,   157,   165,     0,     0,     0,     0,     0,     0,
     178,   179,   180,   181,   182,   183,   184,   185,   186,   187,
       0,     0,     0,     0,   188,   192,   196,   200,   204,   208,
     212,     0,     0,     0,     0,     0,   189,   193,   197,   201,
     205,   209,    97,   103,     0,     0,     0,   123,   131,   139,
     147,   155,   163,   172,   175,   278,   279,     0,     0,     0,
       0,   241,   242,   243,   244,   245,   310,   311,     0,     0,
       0,   266,   241,   242,   243,   244,   245,   241,   242,   243,
     244,   245,   288,     0,     0,     0,     0,   312,   241,   242,
     243,   244,   245,   241,   242,   243,   244,   245,   317,     0,
       0,     0,     0,     0,   241,   242,   243,   244,   245,   318,
       0,     0,     0,     0,     0,   241,   242,   243,   244,   245,
     249,     0,     0,     0,     0,   241,   242,   243,   244,   245,
     272,     0,     0,     0,     0,   241,   242,   243,   244,   245,
     280,     0,     0,     0,     0,   241,   242,   243,   244,   245,
     289,     0,     0,     0,     0,   241,   242,   243,   244,   245,
     241,   242,   243,   244,   245,   258,   259,   260,   261,   114,
       0,     0,   129,   137,   145,   153,   161,   169,   115,     0,
       0,   130,   138,   146,   154,   162,   170
};

static const yytype_int16 yycheck[] =
{
      12,    13,    14,    15,    77,    73,    18,    19,    20,    21,
      22,    23,     5,    70,    71,    76,    75,    74,    73,    12,
      13,    80,    81,    82,    83,    84,    12,    13,    89,    90,
      75,    74,    75,    12,    13,    80,    81,    82,    83,    84,
      12,    13,    74,    75,    56,    57,    58,    59,    60,    61,
      74,    75,    70,    71,    12,    13,    74,    69,    73,    71,
      73,    12,    13,    75,    76,    77,    82,    83,    84,    81,
      82,    83,    84,    74,    75,    68,    12,    13,    73,    72,
      73,    73,    68,    95,    96,    12,    13,    73,    81,    68,
      73,    12,    13,    79,    73,    81,    68,    73,    91,    85,
      79,    73,    81,    77,    78,    91,    85,    79,    73,    81,
      68,    73,    91,    85,    69,    73,    69,    68,    74,    91,
      69,    79,    73,    81,   221,   222,    69,    85,    12,    13,
      81,    69,    68,    91,    85,    74,    75,    73,    73,    73,
      91,    68,    73,    75,    74,    81,    73,    68,    74,    85,
      69,    69,    73,    78,    81,    91,    68,    77,    85,   171,
      81,    74,    77,    77,    91,   262,   263,    75,    78,    74,
      91,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    68,    13,    78,    15,    70,    73,
      18,    19,    20,    21,    22,    23,   269,    81,    74,    64,
      -1,    85,    16,   215,   216,    -1,    -1,    91,    -1,   221,
     222,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    80,    81,    82,    83,    84,    -1,    -1,    -1,   241,
     242,   243,   244,   245,    14,    15,    -1,    17,    18,    19,
      20,    21,    22,    23,    -1,    -1,   258,   259,   260,   261,
     262,   263,    -1,    -1,    -1,    -1,    -1,   269,    74,   271,
     272,    -1,   274,    -1,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    -1,    -1,    -1,    -1,     0,    -1,    -1,     3,
       4,   303,     6,     7,     8,     9,    10,    11,    -1,   311,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,     3,     4,    -1,     6,     7,
       8,     9,    10,    11,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    14,    15,    -1,    -1,    18,    19,
      20,    21,    22,    23,    -1,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      -1,    -1,    -1,    -1,    56,    57,    58,    59,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,    56,    57,    58,    59,
      60,    61,    13,    14,    -1,    -1,    -1,    18,    19,    20,
      21,    22,    23,    24,    25,    74,    75,    -1,    -1,    -1,
      -1,    80,    81,    82,    83,    84,    74,    75,    -1,    -1,
      -1,    74,    80,    81,    82,    83,    84,    80,    81,    82,
      83,    84,    74,    -1,    -1,    -1,    -1,    74,    80,    81,
      82,    83,    84,    80,    81,    82,    83,    84,    74,    -1,
      -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,    74,
      -1,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      75,    -1,    -1,    -1,    -1,    80,    81,    82,    83,    84,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    15,
      -1,    -1,    18,    19,    20,    21,    22,    23,    15,    -1,
      -1,    18,    19,    20,    21,    22,    23
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     6,     7,     8,     9,    10,    11,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    94,    95,    96,    98,    73,    73,
      73,    73,    73,    73,    73,    73,    73,    73,    12,    13,
      68,    73,    79,    81,    85,    91,    99,   100,   101,   102,
     103,   104,   106,   107,   108,    73,    85,    99,   100,   102,
     103,   105,   108,    99,   100,   101,   103,   104,   108,   100,
     101,   102,   103,   104,   106,   107,   100,   103,   105,   108,
     100,   103,   104,    99,   100,   101,   102,   103,   104,   106,
     107,    99,   100,   101,   102,   103,   104,   106,   107,    99,
     100,   101,   102,   103,   104,   106,   107,    99,   100,   101,
     102,   103,   104,   106,   107,    99,   100,   101,   102,   103,
     104,   106,   107,    99,   100,   101,   102,   103,   104,   106,
     107,    85,    99,   100,   103,    99,   100,   103,   100,   100,
     100,   100,   100,   100,   100,   100,   100,   100,   100,   101,
     103,   104,   100,   101,   103,   104,   100,   101,   103,   104,
     100,   101,   103,   104,   100,   101,   103,   104,   100,   101,
     103,   104,   100,   103,    76,    89,    90,     0,    95,    69,
       5,    72,    73,   108,   109,    69,   108,    69,    69,    69,
     108,    97,   108,    97,    73,    73,   108,   108,   108,   108,
      75,    80,    81,    82,    83,    84,   108,   108,    75,    75,
     108,   108,   108,    74,    73,   109,   108,   109,    85,    86,
      87,    88,    70,    71,    74,    74,    74,    74,    74,    75,
      74,    75,    75,    74,    75,    74,    69,    69,    74,    75,
      75,    77,    78,   108,   108,   108,   108,   108,    74,    75,
      68,    74,   108,   108,   108,   108,   109,   109,    97,   108,
     108,   108,    74,    75,    75,    77,    77,    78,    74,    74,
      74,    75,    74,   108,    78,    74,   108,    74,    74
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    93,    94,    94,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    96,    96,    96,    96,
      96,    96,    96,    97,    97,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
     109,   109,   109,   109,   109,   109,   109,   109,   109
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     1,     2,     3,     3,     4,     4,     4,
       4,     6,     4,     4,     1,     1,     1,     6,     4,     4,
       4,     6,     8,     3,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     2,
       2,     2,     2,     1,     2,     2,     2,     2,     1,     2,
       2,     2,     2,     1,     2,     2,     2,     2,     2,     2,
       2,     1,     3,     3,     2,     4,     4,     5,     5,     3,
       3,     3,     3,     3,     2,     3,     4,     6,     1,     1,
       3,     3,     2,     3,     3,     3,     3,     3,     4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 181 "asm.y" /* yacc.c:1646  */
    { new_label((yyvsp[-1].str)); }
#line 1608 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 182 "asm.y" /* yacc.c:1646  */
    { new_symbol_expr((yyvsp[-2].str), (yyvsp[0].expr)); }
#line 1614 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 183 "asm.y" /* yacc.c:1646  */
    { new_symbol_expr_guess((yyvsp[-2].str), (yyvsp[0].expr)); }
#line 1620 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 184 "asm.y" /* yacc.c:1646  */
    { push_if_state((yyvsp[-1].expr)); }
#line 1626 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 185 "asm.y" /* yacc.c:1646  */
    { set_org((yyvsp[-1].expr)); }
#line 1632 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 186 "asm.y" /* yacc.c:1646  */
    { asm_error((yyvsp[-1].str)); }
#line 1638 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 187 "asm.y" /* yacc.c:1646  */
    { asm_echo((yyvsp[-1].str), NULL); }
#line 1644 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 188 "asm.y" /* yacc.c:1646  */
    { asm_echo((yyvsp[-3].str), (yyvsp[-1].atom)); }
#line 1650 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 189 "asm.y" /* yacc.c:1646  */
    { asm_include((yyvsp[-1].str)); }
#line 1656 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 190 "asm.y" /* yacc.c:1646  */
    { push_macro_state((yyvsp[-1].str)); }
#line 1662 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 191 "asm.y" /* yacc.c:1646  */
    { vec_push(asm_atoms, &(yyvsp[0].atom)); }
#line 1668 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 192 "asm.y" /* yacc.c:1646  */
    { macro_append((yyvsp[0].str)); }
#line 1674 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 194 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = (yyvsp[0].atom); }
#line 1680 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 195 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_res((yyvsp[-3].expr), (yyvsp[-1].expr)); }
#line 1686 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 196 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = exprs_to_word_exprs((yyvsp[-1].atom)); }
#line 1692 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 197 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = exprs_to_byte_exprs((yyvsp[-1].atom)); }
#line 1698 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 198 "asm.y" /* yacc.c:1646  */
    {
            (yyval.atom) = new_incbin((yyvsp[-1].str), NULL, NULL); }
#line 1705 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 200 "asm.y" /* yacc.c:1646  */
    {
            (yyval.atom) = new_incbin((yyvsp[-3].str), (yyvsp[-1].expr), NULL); }
#line 1712 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 202 "asm.y" /* yacc.c:1646  */
    {
            (yyval.atom) = new_incbin((yyvsp[-5].str), (yyvsp[-3].expr), (yyvsp[-1].expr)); }
#line 1719 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 205 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = exprs_add((yyvsp[-2].atom), (yyvsp[0].expr)); }
#line 1725 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 206 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_exprs((yyvsp[0].expr)); }
#line 1731 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 208 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xA9, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 1737 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 209 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xA5, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1743 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 210 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xB5, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 1749 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 211 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xAD, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1755 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 212 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xBD, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1761 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 213 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xB9, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1767 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 214 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xA1, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 1773 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 215 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xB1, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 1779 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 217 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xA2, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 1785 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 218 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xA6, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1791 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 219 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xB6, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1797 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 220 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xAE, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1803 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 221 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xBE, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1809 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 223 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xA0, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 1815 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 224 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xA4, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1821 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 225 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xB4, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1827 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 226 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xAC, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1833 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 227 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xBC, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1839 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 229 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x85, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1845 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 230 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x95, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1851 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 231 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x8D, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1857 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 232 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x9D, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1863 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 233 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x99, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1869 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 234 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x81, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1875 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 235 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x91, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1881 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 237 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x86, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1887 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 238 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x96, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1893 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 239 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x8e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1899 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 241 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x84, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1905 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 242 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x94, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1911 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 243 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x8c, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1917 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 245 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x29, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 1923 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 246 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x25, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1929 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 247 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x35, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 1935 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 248 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x2d, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1941 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 249 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x3d, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1947 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 250 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x39, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1953 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 251 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x21, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 1959 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 252 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x31, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 1965 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 254 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x09, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 1971 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 255 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x05, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 1977 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 256 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x15, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 1983 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 257 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x0d, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1989 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 258 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x1d, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 1995 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 259 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x19, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2001 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 260 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x01, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2007 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 261 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x11, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2013 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 263 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x49, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 2019 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 264 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x45, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 2025 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 265 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x55, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2031 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 266 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x4d, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2037 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 267 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x5d, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2043 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 268 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x59, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2049 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 269 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x41, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2055 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 270 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x51, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2061 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 272 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x69, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 2067 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 273 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x65, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 2073 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 274 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x75, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2079 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 275 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x6D, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2085 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 276 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x7D, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2091 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 277 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x79, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2097 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 278 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x61, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2103 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 279 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x71, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2109 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 281 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xe9, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 2115 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 282 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xe5, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 2121 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 283 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xf5, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2127 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 284 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xeD, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2133 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 285 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xfD, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2139 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 286 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xf9, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2145 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 287 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xe1, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2151 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 288 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xf1, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2157 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 290 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xc9, ATOM_TYPE_OP_ARG_UI8, (yyvsp[0].expr)); }
#line 2163 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 291 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xc5, ATOM_TYPE_OP_ARG_U8,  (yyvsp[0].expr)); }
#line 2169 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 292 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xd5, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2175 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 293 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xcD, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2181 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 294 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xdD, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2187 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 295 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xd9, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2193 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 296 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xc1, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2199 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 297 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xd1, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2205 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 299 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xe0, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2211 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 300 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xe4, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2217 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 301 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xec, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2223 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 302 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xc0, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2229 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 303 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xc4, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2235 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 304 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xcc, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2241 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 306 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x9A); }
#line 2247 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 307 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0xBA); }
#line 2253 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 308 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x48); }
#line 2259 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 309 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x68); }
#line 2265 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 310 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x08); }
#line 2271 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 311 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x28); }
#line 2277 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 312 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x78); }
#line 2283 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 313 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x58); }
#line 2289 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 314 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0xea); }
#line 2295 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 315 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x98); }
#line 2301 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 316 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0xa8); }
#line 2307 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 317 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x8a); }
#line 2313 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 318 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0xaa); }
#line 2319 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 319 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x18); }
#line 2325 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 320 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x38); }
#line 2331 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 321 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x60); }
#line 2337 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 323 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x20, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2343 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 324 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x4c, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2349 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 325 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xf0, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2355 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 326 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xd0, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2361 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 327 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x90, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2367 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 328 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xb0, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2373 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 329 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x10, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2379 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 330 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x30, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2385 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 331 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x50, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2391 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 332 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x70, ATOM_TYPE_OP_ARG_I8,  (yyvsp[0].expr)); }
#line 2397 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 334 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0xe8); }
#line 2403 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 335 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0xca); }
#line 2409 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 336 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0xc8); }
#line 2415 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 337 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x88); }
#line 2421 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 339 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xe6, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2427 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 340 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xf6, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2433 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 341 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xee, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2439 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 342 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xfe, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2445 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 344 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xc6, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2451 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 345 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xd6, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2457 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 346 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xce, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2463 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 347 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0xde, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2469 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 349 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x4a); }
#line 2475 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 350 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x46, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2481 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 351 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x56, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2487 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 352 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x4e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2493 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 353 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x5e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2499 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 355 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x0a); }
#line 2505 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 356 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x06, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2511 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 357 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x16, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2517 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 358 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x0e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2523 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 359 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x1e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2529 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 361 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x6a); }
#line 2535 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 362 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x66, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2541 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 363 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x76, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2547 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 364 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x6e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2553 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 365 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x7e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2559 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 367 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op0(0x2a); }
#line 2565 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 368 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x26, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2571 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 369 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x36, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2577 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 370 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x2e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2583 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 371 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x3e, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2589 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 373 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x24, ATOM_TYPE_OP_ARG_U8, (yyvsp[0].expr)); }
#line 2595 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 374 "asm.y" /* yacc.c:1646  */
    { (yyval.atom) = new_op(0x2c, ATOM_TYPE_OP_ARG_U16, (yyvsp[0].expr)); }
#line 2601 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 376 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2607 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 377 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2613 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 378 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); }
#line 2619 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 379 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); }
#line 2625 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 380 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[0].expr); }
#line 2631 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 381 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); }
#line 2637 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 382 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-2].expr); }
#line 2643 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 383 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-3].expr); }
#line 2649 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 384 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-3].expr); }
#line 2655 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 386 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(PLUS, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2661 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 387 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(MINUS, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2667 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 388 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(MULT, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2673 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 389 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(DIV, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2679 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 390 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(MOD, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2685 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 391 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op1(vNEG, (yyvsp[0].expr)); }
#line 2691 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 392 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2697 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 393 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_inclen((yyvsp[-1].str)); }
#line 2703 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 394 "asm.y" /* yacc.c:1646  */
    {
            (yyval.expr) = new_expr_incword((yyvsp[-3].str), (yyvsp[-1].expr)); }
#line 2710 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 396 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_number((yyvsp[0].num)); }
#line 2716 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 397 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_symref((yyvsp[0].str)); }
#line 2722 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 399 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(LOR, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2728 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 400 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(LAND, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2734 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 401 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op1(LNOT, (yyvsp[0].expr)); }
#line 2740 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 402 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = (yyvsp[-1].expr); }
#line 2746 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 403 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(LT, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2752 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 404 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(GT, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2758 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 405 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(EQ, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2764 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 406 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_expr_op2(NEQ, (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2770 "asm.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 408 "asm.y" /* yacc.c:1646  */
    { (yyval.expr) = new_is_defined((yyvsp[-1].str)); }
#line 2776 "asm.tab.c" /* yacc.c:1646  */
    break;


#line 2780 "asm.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 410 "asm.y" /* yacc.c:1906  */


void yyerror (const char *s)
{
    fprintf (stderr, "line %d, %s\n", num_lines, s);
}

void asm_set_source(struct membuf *buffer);

int assembleSinglePass(struct membuf *source, struct membuf *dest)
{
    int val;

    yydebug = 0;
    asm_src_buffer_push(source);
    vec_init(asm_atoms, sizeof(struct atom*));
    val = yyparse();
    if(val == 0)
    {
        output_atoms(dest, asm_atoms);
    }
    vec_free(asm_atoms, NULL);
    return val;
}
