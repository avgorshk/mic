"""Unit tests for the MICC parser (compiler/parser.py).

Run: python compiler_tests/test_parser.py
"""

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'compiler'))
from lexer import Lexer, CompileError  # noqa: E402
from parser import parse  # noqa: E402
from ast_nodes import dump  # noqa: E402

RESULTS = []


def check(name, fn):
    try:
        fn()
        RESULTS.append((name, True, ''))
    except Exception as e:
        RESULTS.append((name, False, '%s: %s' % (type(e).__name__, e)))


def dump_src(source):
    return '\n'.join(dump(parse(Lexer(source).tokenize())))


def expect_error(source, needle):
    try:
        parse(Lexer(source).tokenize())
    except CompileError as e:
        assert needle in e.msg, "error '%s' does not contain '%s'" % (e.msg, needle)
        return
    raise AssertionError('expected CompileError containing %r, got no error' % needle)


FIB = '''\
int fib(int n) {
    if (n < 2)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int main() {
    int r = fib(10);
    return r;
}
'''

FIB_DUMP = '''\
Program
  FuncDef fib
    Param n
    Block
      If
        Binary <
          Var n
          IntLit 2
        Return
          Var n
      Return
        Binary +
          Call fib
            Binary -
              Var n
              IntLit 1
          Call fib
            Binary -
              Var n
              IntLit 2
  FuncDef main
    Block
      VarDecl r
        Call fib
          IntLit 10
      Return
        Var r'''


def test_full_program_dump():
    assert dump_src(FIB) == FIB_DUMP, dump_src(FIB)


def test_precedence_mul_over_add():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a + b * c;
}
''')
    assert got == '''\
Program
  FuncDef main
    Block
      VarDecl a
        IntLit 1
      VarDecl b
        IntLit 2
      VarDecl c
        IntLit 3
      VarDecl d
        Binary +
          Var a
          Binary *
            Var b
            Var c''', got


def test_precedence_add_over_relational():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a < b + c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary <
          Var a
          Binary +
            Var b
            Var c'''), got


def test_precedence_shift_between_additive_and_relational():
    # C: + - bind tighter than << >>, which bind tighter than < <=
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a << b + c;
    int e = a + b << c;
    int f = a << b < c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary <<
          Var a
          Binary +
            Var b
            Var c
      VarDecl e
        Binary <<
          Binary +
            Var a
            Var b
          Var c
      VarDecl f
        Binary <
          Binary <<
            Var a
            Var b
          Var c'''), got


def test_shift_associativity():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a << b << c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary <<
          Binary <<
            Var a
            Var b
          Var c'''), got


def test_shift_constant_folding():
    got = dump_src('''\
int main() {
    int a = 1 << 3;
    int b = 100 >> 2;
}
''')
    assert got.endswith('''\
      VarDecl a
        IntLit 8
      VarDecl b
        IntLit 25'''), got


def test_shift_constant_folding_out_of_range_stays_binary():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = a << 40;
}
''')
    assert got.endswith('''\
      VarDecl b
        Binary <<
          Var a
          IntLit 40'''), got


def test_compound_assignment_sugar():
    got = dump_src('''\
int main() {
    int a = 1;
    a += 2;
    a -= 3;
}
''')
    assert got.endswith('''\
      Assign a
        Binary +
          Var a
          IntLit 2
      Assign a
        Binary -
          Var a
          IntLit 3'''), got


def test_unary_binding():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = ~a - -b;
}
''')
    assert got.endswith('''\
      VarDecl c
        Binary -
          Unary ~
            Var a
          Unary -
            Var b'''), got


def test_left_associativity():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a - b - c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary -
          Binary -
            Var a
            Var b
          Var c'''), got


def test_multiplicative_associativity():
    got = dump_src('''\
int main() {
    int a = 20;
    int b = 10;
    int c = 5;
    int d = a / b * c;
    int e = a * b / c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary *
          Binary /
            Var a
            Var b
          Var c
      VarDecl e
        Binary /
          Binary *
            Var a
            Var b
          Var c'''), got


def test_logic_operators():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a && b || !c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary ||
          Binary &&
            Var a
            Var b
          Unary !
            Var c'''), got


def test_parentheses_override():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = (a + b) * c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary *
          Binary +
            Var a
            Var b
          Var c'''), got


def test_dangling_else():
    got = dump_src('''\
int main() {
    int x = 1;
    if (x)
        if (x)
            x = 2;
        else
            x = 3;
}
''')
    assert got == '''\
Program
  FuncDef main
    Block
      VarDecl x
        IntLit 1
      If
        Var x
        If
          Var x
          Assign x
            IntLit 2
          Else
            Assign x
              IntLit 3''', got


def test_while_and_empty_return():
    got = dump_src('''\
int main() {
    int i = 0;
    while (i < 10) {
        i = i + 1;
    }
    return;
}
''')
    assert got == '''\
Program
  FuncDef main
    Block
      VarDecl i
        IntLit 0
      While
        Binary <
          Var i
          IntLit 10
        Block
          Assign i
            Binary +
              Var i
              IntLit 1
      Return''', got


def test_call_with_args():
    got = dump_src('''\
int main() {
    int x = 1;
    int y = f(1, x, g(2));
}
''')
    assert got.endswith('''\
      VarDecl y
        Call f
          IntLit 1
          Var x
          Call g
            IntLit 2'''), got


def test_expression_statement():
    got = dump_src('''\
int main() {
    int x = 1;
    f(x);
}
''')
    assert got.endswith('''\
      ExprStmt
        Call f
          Var x'''), got


def test_comparison_chain_rejected():
    expect_error('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a < b < c;
}
''', 'comparison operators cannot be chained')
    expect_error('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a == b == c;
}
''', 'comparison operators cannot be chained')


def test_assignment_not_an_expression():
    expect_error('''\
int main() {
    int a = 1;
    int b = 2;
    a = b = 3;
}
''', "expected ';'")


def test_undeclared_variable():
    expect_error('''\
int main() {
    x = 1;
}
''', "use of undeclared variable 'x'")


def test_redeclaration():
    expect_error('''\
int main() {
    int x;
    int x;
}
''', "redeclaration of 'x'")


def test_duplicate_function():
    expect_error('''\
int main() {
    return 0;
}
int main() {
    return 0;
}
''', "duplicate function 'main'")


def test_main_missing():
    expect_error('''\
int foo() {
    return 0;
}
''', "function 'main' must be defined")


def test_main_with_params():
    expect_error('''\
int main(int x) {
    return 0;
}
''', 'main must not take parameters')


def test_unsupported_type():
    expect_error('''\
int main() {
    long x;
}
''', "unsupported type 'long' (only 'int' is supported)")
    expect_error('''\
int f(long x) {
    return 0;
}
int main() {
    return 0;
}
''', "unsupported type 'long' (only 'int' is supported)")


def test_bitwise_precedence():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a | b & c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary |
          Var a
          Binary &
            Var b
            Var c'''), got


def test_bitwise_vs_equality():
    got = dump_src('''\
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = a & b == c;
}
''')
    assert got.endswith('''\
      VarDecl d
        Binary &
          Var a
          Binary ==
            Var b
            Var c'''), got


def test_missing_semicolon():
    expect_error('''\
int main() {
    int x = 1
}
''', "expected ';'")


def test_missing_paren():
    expect_error('''\
int main() {
    if (1 {
    }
}
''', "expected ')'")


def test_empty_program_rejected():
    expect_error('', "function 'main' must be defined")


check('full program dump (fib + main)', test_full_program_dump)
check('precedence: * binds tighter than +', test_precedence_mul_over_add)
check('precedence: + binds tighter than <', test_precedence_add_over_relational)
check('precedence: shift between additive and relational',
     test_precedence_shift_between_additive_and_relational)
check('shift associativity (left)', test_shift_associativity)
check('shift constant folding', test_shift_constant_folding)
check('out-of-range constant shift stays a node',
     test_shift_constant_folding_out_of_range_stays_binary)
check('+= -= assignment sugar', test_compound_assignment_sugar)
check('unary binding', test_unary_binding)
check('left associativity of -', test_left_associativity)
check('left associativity of * /', test_multiplicative_associativity)
check('&& || ! tree', test_logic_operators)
check('parentheses override', test_parentheses_override)
check('bitwise: & binds tighter than |', test_bitwise_precedence)
check('bitwise: == binds tighter than &', test_bitwise_vs_equality)
check('dangling else binds to nearest if', test_dangling_else)
check('while + empty return', test_while_and_empty_return)
check('call with args', test_call_with_args)
check('expression statement', test_expression_statement)
check('comparison chains rejected', test_comparison_chain_rejected)
check('assignment is not an expression', test_assignment_not_an_expression)
check('undeclared variable rejected', test_undeclared_variable)
check('redeclaration rejected', test_redeclaration)
check('duplicate function rejected', test_duplicate_function)
check('missing main rejected', test_main_missing)
check('main with params rejected', test_main_with_params)
check('unsupported types rejected', test_unsupported_type)
check('missing semicolon rejected', test_missing_semicolon)
check('missing paren rejected', test_missing_paren)
check('empty program rejected', test_empty_program_rejected)


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
