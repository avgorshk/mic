"""MICC compiler driver.

M2 stage: lex + parse + AST dump. Codegen to .asm arrives in M3.

Usage:
    python micc.py input.micc [--dump]

Prints the AST dump on success (exit 0). On a lex/parse error prints
[ERROR] line L, col C: <message> to stderr and exits 1.
"""

import argparse
import sys

from lexer import Lexer, CompileError
from parser import parse
from ast_nodes import dump


def compile_to_ast(source):
    tokens = Lexer(source).tokenize()
    return parse(tokens)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description='MICC compiler (M2: lexer + parser + AST dump)')
    parser.add_argument('input', help='.micc source file')
    parser.add_argument('--dump', action='store_true',
                        help='print the AST dump (default in M2)')
    args = parser.parse_args(argv)

    try:
        with open(args.input, 'r', encoding='utf-8') as f:
            source = f.read()
        program = compile_to_ast(source)
    except CompileError as e:
        print('[ERROR] line %d, col %d: %s' % (e.line, e.col, e.msg),
              file=sys.stderr)
        return 1
    except OSError as e:
        print('[ERROR] %s' % e, file=sys.stderr)
        return 1

    print('\n'.join(dump(program)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
