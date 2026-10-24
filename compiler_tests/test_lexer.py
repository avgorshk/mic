"""Unit tests for the MICC lexer (compiler/lexer.py).

Run: python compiler_tests/test_lexer.py
"""

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'compiler'))
from lexer import Lexer, CompileError  # noqa: E402

RESULTS = []


def check(name, fn):
    try:
        fn()
        RESULTS.append((name, True, ''))
    except Exception as e:
        RESULTS.append((name, False, '%s: %s' % (type(e).__name__, e)))


def kinds(source):
    return [t[0] for t in Lexer(source).tokenize()]


def expect_error(source, needle, line=None, col=None):
    try:
        Lexer(source).tokenize()
    except CompileError as e:
        assert needle in e.msg, "error '%s' does not contain '%s'" % (e.msg, needle)
        if line is not None:
            assert e.line == line, 'line %d != %d' % (e.line, line)
        if col is not None:
            assert e.col == col, 'col %d != %d' % (e.col, col)
        return
    raise AssertionError('expected CompileError containing %r, got no error' % needle)


def test_simple_tokens():
    toks = Lexer('int x = 5;').tokenize()
    assert [(t[0], t[1]) for t in toks] == [
        ('INT', 'int'), ('IDENT', 'x'), ('=', '='), ('INTLIT', 5),
        (';', ';'), ('EOF', '')], str(toks)
    # positions: int@1:1, x@1:5, =@1:7, 5@1:9, ;@1:10
    assert [(t[2], t[3]) for t in toks] == [
        (1, 1), (1, 5), (1, 7), (1, 9), (1, 10), (1, 11)], str(toks)


def test_two_char_operators():
    assert kinds('a <= b && c != d || e == f') == [
        'IDENT', '<=', 'IDENT', '&&', 'IDENT', '!=', 'IDENT', '||',
        'IDENT', '==', 'IDENT', 'EOF']


def test_shift_and_compound_operators():
    assert kinds('a << b >> c += d -= e') == [
        'IDENT', '<<', 'IDENT', '>>', 'IDENT', '+=', 'IDENT', '-=',
        'IDENT', 'EOF']


def test_single_char_operators():
    assert kinds('+ - * / % & | ~ ! < > = ( ) { } ; ,') == [
        '+', '-', '*', '/', '%', '&', '|', '~', '!', '<', '>', '=',
        '(', ')', '{', '}', ';', ',', 'EOF']


def test_keywords_vs_identifiers():
    assert kinds('int if else while return') == [
        'INT', 'IF', 'ELSE', 'WHILE', 'RETURN', 'EOF']
    assert kinds('returnx interior whiles') == ['IDENT', 'IDENT', 'IDENT', 'EOF']


def test_integer_literals():
    toks = Lexer('0 7 42 0x10 0xFF').tokenize()
    values = [t[1] for t in toks[:-1]]
    assert values == [0, 7, 42, 16, 255], str(values)


def test_hex_literal_errors():
    expect_error('0x', 'invalid hex literal', line=1, col=1)
    expect_error('0xG', 'invalid hex literal', line=1, col=1)


def test_line_comments():
    toks = Lexer('// comment\nint x;').tokenize()
    assert [(t[0], t[1], t[2], t[3]) for t in toks] == [
        ('INT', 'int', 2, 1), ('IDENT', 'x', 2, 5), (';', ';', 2, 6),
        ('EOF', '', 2, 7)], str(toks)


def test_block_comments():
    toks = Lexer('int /* multi\nline */ x;').tokenize()
    assert [(t[0], t[1], t[2], t[3]) for t in toks] == [
        ('INT', 'int', 1, 1), ('IDENT', 'x', 2, 9), (';', ';', 2, 10),
        ('EOF', '', 2, 11)], str(toks)


def test_unterminated_comment():
    expect_error('int /* oops', 'unterminated comment', line=1, col=5)


def test_unexpected_character():
    expect_error('int @;', "unexpected character '@'", line=1, col=5)


def test_empty_source():
    assert kinds('') == ['EOF']
    assert kinds('   \n\t\n') == ['EOF']


check('simple tokens with positions', test_simple_tokens)
check('two-char operators', test_two_char_operators)
check('shift and compound-assignment operators', test_shift_and_compound_operators)
check('single-char operators', test_single_char_operators)
check('keywords vs identifiers', test_keywords_vs_identifiers)
check('integer literals (dec + hex)', test_integer_literals)
check('hex literal errors', test_hex_literal_errors)
check('line comments', test_line_comments)
check('block comments', test_block_comments)
check('unterminated comment', test_unterminated_comment)
check('unexpected character', test_unexpected_character)
check('empty source', test_empty_source)


def main():
    failed = 0
    for name, ok, msg in RESULTS:
        if ok:
            print('[PASS] %s' % name)
        else:
            print('[FAIL] %s: %s' % (name, msg))
            failed += 1
    print('Total: %d, failed: %d' % (len(RESULTS), failed))
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
