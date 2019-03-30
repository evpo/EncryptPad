import sys
import os
def AppendLineFile(ops, file_name):
    if not os.path.isfile(file_name):
        return

    with open(file_name, 'r') as ops_file:
        for line in ops_file.readlines():
            for word in line.rstrip('\n').split(' '):
                ops.append(word)

def Settings( **kwargs ):
    ops= []
    AppendLineFile(ops, os.getenv('HOME') + '/.ycm/system_flags.txt')
    AppendLineFile(ops, 'compiler_options.txt')
    return {'flags': ops}
