import subprocess
import sys
import json
import textwrap
import re
import shutil
from colors import *

'''
    Element colors
    -------
    See ./colors.py
'''
ERR_COLOR               = B_MAX_RED
WARN_COLOR              = B_MAX_PURPLE
MISC_COLOR              = B_MAX_CYAN
FILE_PATH_COLOR         = B_MAX_GREEN
LINE_NUM_COLOR          = B_MAX_YELLOW
MSG_PROMPT_COLOR        = B_MAX_BLUE
MSG_COLOR               = CYAN
MSG_STR_COLOR           = YELLOW
ERR_CODE_PROMPT_COLOR   = ERR_COLOR
WARN_CODE_PROMPT_COLOR  = WARN_COLOR
ERR_CARET_COLOR         = ERR_COLOR
WARN_CARET_COLOR        = WARN_COLOR

err_num = 1
warn_num = 1



def create_indent_string (n):
    indent = ''
    for _ in range(n):
        indent = ' ' + indent
    return indent


def print_message (msg, type_str, file_path, line_number, caret_cols):

    term_size = shutil.get_terminal_size()
    term_cols = term_size.columns
    max_err_dig = 2
    code_indent = '        '
    msg_indent  = '        '
    global err_num
    global warn_num

    # Error  :  <file path>  :  <line number>
    TYPE_COLOR = MISC_COLOR
    type_str = ''.join([c.upper() if i == 0 else c for i,c in enumerate(type_str)])
    err_buff = ''
    msg_num = -1

    if type_str == 'Error':
        TYPE_COLOR = ERR_COLOR
        msg_num = err_num
        err_num += 1
        err_buff = create_indent_string (len('Warning') - len('Error'))
        err_buff = create_indent_string ((max_err_dig - len(str(msg_num))) + len(err_buff))
    elif type_str == 'Warning':
        TYPE_COLOR = WARN_COLOR
        msg_num = warn_num
        warn_num += 1
        err_buff = create_indent_string (max_err_dig - len(str(msg_num)))
    else:
        msg_num = '\b'
        err_buff = create_indent_string (len('Warning') - len(type_str))
        err_buff = create_indent_string ((max_err_dig - len(msg_num)) + len(err_buff) + 2)
        warn_num = ''

    error = f"{err_buff}{TYPE_COLOR}{type_str} {str(msg_num)}{RESET}:  {FILE_PATH_COLOR}{file_path}{RESET} : {LINE_NUM_COLOR}{line_number}{RESET}"

    # message
    prompt = ">>>  "
    msg_str = ''.join([c.upper() if i == 0 else c for i,c in enumerate(msg)])
    msg_str = f"{MSG_PROMPT_COLOR}{prompt}{MSG_COLOR}{msg_str}{RESET}"
    msg_str = textwrap.fill (msg_str, initial_indent=msg_indent, subsequent_indent= msg_indent + f"{'': <{len(prompt)}}", width=term_cols)
    msg_str = re.sub (r"‘([^‘]*)’", rf"‘{MSG_STR_COLOR}\1{RESET}{MSG_COLOR}’", msg_str)

    # line of code
    code_line = ''
    stripped_spaces = 0
    try:
        with open(file_path, 'r') as file:
            for current_line_number, line in enumerate(file, start=1):
                if current_line_number == line_number:
                    orig_code_line = line
                    code_line = orig_code_line.lstrip()
                    stripped_spaces = len(orig_code_line) - len(code_line)
                    break
                else:
                    code_line = f"Unable to find line number {line_number} in \"{file_path}\""
    except FileNotFoundError:
        code_line = f"Unable to find \"{file_path}\""

    CODE_PROMPT_COLOR = MISC_COLOR
    if type_str == 'Warning':
        CODE_PROMPT_COLOR = WARN_CODE_PROMPT_COLOR
    elif type_str == 'Error':
        CODE_PROMPT_COLOR = ERR_CODE_PROMPT_COLOR
    code_line = code_indent + f"{CODE_PROMPT_COLOR}{prompt}{RESET}" + code_line.rstrip('\n')

    # caret
    CARET_COLOR = MISC_COLOR
    if type_str == 'Warning':
        CARET_COLOR = WARN_CARET_COLOR
    elif type_str == 'Error':
        CARET_COLOR = ERR_CARET_COLOR
    caret_indent = create_indent_string (caret_cols - stripped_spaces - 2)
    if len(caret_indent) == 0:
        code_indent = code_indent[1:]
    caret = code_indent + caret_indent + f'{"": <{len(prompt)}}' + f'{CARET_COLOR}⤴{RESET}'

    # print error message
    print (error)
    print (msg_str)
    print (code_line)
    print (caret)
    print ("")


def format_gcc_output (command):
    
    global err_num
    global warn_num

    # run command, get output
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    stdout, stderr = process.communicate()
    output = stdout + stderr
    process.kill()
    #print (output)
    #sys.exit(0)

    # get start of json
    i = output.find('[{')
    if i == -1:
        print ("\n  No error messages found\n")
        sys.exit(1)
                      
    # get end of json
    j = output.rindex("}]")

    # get json string
    json_str = output[i:j+2]
    #print (output[i:j+2])
    #sys.exit(0)

    # split json [{ objects }] into separate strings
    bracket_splits = json_str.split("\n[]\n")
    err_jsons = []
    for split in bracket_splits:
        splits = split.split('\n')
        for s in splits:
            err_jsons.append(s)
    #print(newline_splits)
    #sys.exit(0)

    for err in err_jsons:

        # convert to dict
        msg_dict = json.loads(err)
        #print (json.dumps (msg_dict, indent=4))
        #sys.exit(0)

        print ("")
        print ("")

        err_num = 1
        warn_num = 1

        # print errors
        for msg in msg_dict:

            type_str = msg['kind']
            file_path = ''
            line_number = 0
            caret_cols_list = []
            caret_cols = 0
            caret_indent = ''

            # locations
            for loc in msg['locations']:

                if 'caret' in loc:
                    caret = loc['caret']
                    file_path = caret.get('file')
                    line_number = caret.get('line')
                    caret_cols_list.append(caret.get('column'))

            caret_cols = min(caret_cols_list)

            print_message (msg['message'], type_str, file_path, line_number, caret_cols)

