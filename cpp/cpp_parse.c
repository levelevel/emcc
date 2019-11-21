#include "emcpp.h"

/*  文法：
- A.3 Preprocessing directives http://port70.net/~nsz/c/c11/n1570.html#A.3
    preprocessing_file  = group_part*
    group_part          = if_section
                        | control_line
                        | text_line
                        | "#" non_directive
    if_section          = if_group elif_group? else_group? endif_line
    if_group            = "#" "if" constant_expression new_line group?
                        = "#" "ifdef" identifier new_line group?
                        = "#" "ifndef" identifier new_line group?
    elif_group          = "#" "elif" constant_expression new_line group?
    else_group          = "#" "else" new_line group
    endif_line          = "#" "endif" new_line
    pp_token            = 
*/

void preprocessing_file(void) {

}
