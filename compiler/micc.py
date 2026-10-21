"""MICC compiler driver.

Usage:
    python micc.py input.micc [--dump] [-o out.asm]

Without -o: prints the AST dump (M2 behavior).
With -o: compiles to assembly text and writes it to out.asm.
On any error prints [ERROR] line L, col C: <message> to stderr, exit 1.
"""

import argparse
import sys

from lexer import Lexer, CompileError
from parser import parse
from ast_nodes import dump
from codegen import gen


def compile_to_ast(source):
    tokens = Lexer(source).tokenize()
    return parse(tokens)


def compile_to_asm(source):
    return gen(compile_to_ast(source))


def main(argv=None):
    parser = argparse.ArgumentParser(
        description='MICC compiler for the micsim instruction set')
    parser.add_argument('input', help='.micc source file')
    parser.add_argument('-o', '--output', default=None,
                        help='write assembly text to this file')
    parser.add_argument('--dump', action='store_true',
                        help='print the AST dump (default when -o is absent)')
    args = parser.parse_args(argv)

    try:
        with open(args.input, 'r', encoding='utf-8') as f:
            source = f.read()
        if args.output:
            asm_text = compile_to_asm(source)
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(asm_text)
        else:
            program = compile_to_ast(source)
            print('\n'.join(dump(program)))
    except CompileError as e:
        print('[ERROR] line %d, col %d: %s' % (e.line, e.col, e.msg),
              file=sys.stderr)
        return 1
    except OSError as e:
        print('[ERROR] %s' % e, file=sys.stderr)
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())

