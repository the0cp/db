#ifndef WIN

#define TERM_RED    "\e[1;31m"
#define TERM_GREEN  "\e[1;32m"
#define TERM_YELLOW  "\e[1;33m"
#define TERM_BLUE   "\e[1;34m"
#define TERM_CYAN   "\e[1;36m"
#define TERM_WHITE  "\e[1;37m"
#define TERM_RESET  "\e[0m"
#define TERM_CLEAR  "clear"

#else

#define TERM_RED    ""
#define TERM_GREEN  ""
#define TERM_YELLOW  ""
#define TERM_BLUE   ""
#define TERM_CYAN   ""
#define TERM_WHITE  ""
#define TERM_RESET  ""
#define TERM_CLEAR  "cls"

#endif