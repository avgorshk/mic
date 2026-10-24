"""Recursive-descent parser for the MICC language.

Grammar (operators listed tightest to loosest binding):
    program   := { funcdef }
    funcdef   := 'int' IDENT '(' [ 'int' IDENT {',' 'int' IDENT} ] ')' block
    block     := '{' { stmt } '}'
    stmt      := 'int' IDENT [ '=' expr ] ';'        (variable declaration)
               | IDENT '=' expr ';'                  (assignment)
               | 'if' '(' expr ')' stmt [ 'else' stmt ]
               | 'while' '(' expr ')' stmt
               | 'return' [ expr ] ';'
               | block
               | expr ';'                            (expression statement)
    expr      := or { '||' or }
    or        := and { '&&' and }
    and       := bitor { '|' bitor }
    bitor     := bitand { '&' bitand }
    bitand    := equality { ('==' | '!=') equality }
    equality  := relational { ('<' | '<=' | '>' | '>=') relational }
    relational:= shift
    shift     := additive { ('<<' | '>>') additive }
    additive  := multiplicative { ('+' | '-') multiplicative }
    multiplicative := unary
    unary     := ('-' | '!' | '~') unary | primary
    primary   := INTLIT | IDENT | IDENT '(' [ expr {',' expr} ] ')' | '(' expr ')'

Assignment: '=' plain, or '+='/'-=' sugar (x += e == x = x + e).
Comparison operators cannot be chained (a < b < c is an error).
Shift counts: constant shifts are folded at parse time; runtime shifts
call the shl/shr library functions (arithmetic right shift).

Scope: variables are function-scoped (params + declared locals); a variable
must be declared before first use. Duplicate declarations are errors.
'main' must be defined and take no parameters. Undefined function names in
calls are left to the codegen stage (M3), which sees the whole program.
"""

from lexer import CompileError
import ast_nodes as ast


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
        self.scope = None  # set of declared names for the current function

    def _cur(self):
        return self.tokens[self.pos]

    def _next(self):
        tok = self.tokens[self.pos]
        if tok[0] != 'EOF':
            self.pos += 1
        return tok

    def _check(self, kind):
        return self._cur()[0] == kind

    def _expect(self, kind, what=None):
        tok = self._cur()
        if tok[0] != kind:
            raise CompileError(tok[2], tok[3],
                               "expected %s, got '%s'"
                               % (what or repr(kind), tok[1]))
        return self._next()

    # ------------------------------------------------------------- program

    def parse(self):
        funcs = []
        seen = set()
        while not self._check('EOF'):
            f = self._parse_funcdef()
            if f.name in seen:
                raise CompileError(f.line, 1, "duplicate function '%s'" % f.name)
            seen.add(f.name)
            funcs.append(f)
        if 'main' not in seen:
            raise CompileError(1, 1, "function 'main' must be defined")
        return ast.Program(funcs)

    def _parse_funcdef(self):
        tok = self._expect('INT')
        name_tok = self._expect('IDENT', 'function name')
        self._expect('(')
        params = []
        if self._check('INT'):
            while True:
                self._next()  # 'int'
                p = self._expect('IDENT', 'parameter name')
                if p[1] in params:
                    raise CompileError(p[2], p[3],
                                       "duplicate parameter '%s'" % p[1])
                params.append(p[1])
                if not self._check(','):
                    break
                self._next()  # ','
        elif self._check('IDENT'):
            tok = self._cur()
            raise CompileError(tok[2], tok[3],
                               "unsupported type '%s' (only 'int' is supported)"
                               % tok[1])
        self._expect(')', "')'")
        if name_tok[1] == 'main' and params:
            raise CompileError(name_tok[2], name_tok[3],
                               "main must not take parameters")
        saved_scope = self.scope
        self.scope = set(params)
        body = self._parse_block()
        self.scope = saved_scope
        return ast.FuncDef(name_tok[1], params, body, name_tok[2])

    # ---------------------------------------------------------- statements

    def _parse_block(self):
        self._expect('{')
        stmts = []
        while not self._check('}'):
            stmts.append(self._parse_stmt())
        self._expect('}', "'}'")
        return ast.Block(stmts)

    def _parse_stmt(self):
        tok = self._cur()
        if tok[0] == 'INT':
            return self._parse_vardecl()
        if tok[0] == 'IDENT' and self.tokens[self.pos + 1][0] == 'IDENT':
            # "long x;" style declaration with a type we don't support
            raise CompileError(tok[2], tok[3],
                               "unsupported type '%s' (only 'int' is supported)"
                               % tok[1])
        if tok[0] == 'IF':
            return self._parse_if()
        if tok[0] == 'WHILE':
            return self._parse_while()
        if tok[0] == 'RETURN':
            return self._parse_return()
        if tok[0] == '{':
            return self._parse_block()
        if tok[0] == 'IDENT':
            # assignment (IDENT '=' | '+=' | '-=') or expression statement
            next_kind = self.tokens[self.pos + 1][0]
            if next_kind in ('=', '+=', '-='):
                name_tok = self._next()
                self._check_declared(name_tok)
                op_tok = self._next()
                expr = self._parse_expr()
                self._expect(';', "';'")
                if op_tok[0] == '=':
                    return ast.Assign(name_tok[1], expr)
                # x += e  ==  x = x + e ; x -= e  ==  x = x - e
                return ast.Assign(
                    name_tok[1],
                    ast.Binary(op_tok[0][0], ast.Var(name_tok[1]), expr))
            expr = self._parse_expr()
            self._expect(';', "';'")
            return ast.ExprStmt(expr)
        # expression statement starting with a literal, '(' or unary op
        expr = self._parse_expr()
        self._expect(';', "';'")
        return ast.ExprStmt(expr)

    def _check_declared(self, tok):
        if self.scope is not None and tok[1] not in self.scope:
            raise CompileError(tok[2], tok[3],
                               "use of undeclared variable '%s'" % tok[1])

    def _parse_vardecl(self):
        tok = self._expect('INT')
        name_tok = self._expect('IDENT', 'variable name')
        if self.scope is not None and name_tok[1] in self.scope:
            raise CompileError(name_tok[2], name_tok[3],
                               "redeclaration of '%s'" % name_tok[1])
        if self.scope is not None:
            self.scope.add(name_tok[1])
        init = None
        if self._check('='):
            self._next()
            init = self._parse_expr()
        self._expect(';', "';'")
        return ast.VarDecl(name_tok[1], init)

    def _parse_if(self):
        self._expect('IF')
        self._expect('(')
        cond = self._parse_expr()
        self._expect(')', "')'")
        then_body = self._parse_stmt()
        else_body = None
        if self._check('ELSE'):
            self._next()
            else_body = self._parse_stmt()
        return ast.If(cond, then_body, else_body)

    def _parse_while(self):
        self._expect('WHILE')
        self._expect('(')
        cond = self._parse_expr()
        self._expect(')', "')'")
        body = self._parse_stmt()
        return ast.While(cond, body)

    def _parse_return(self):
        self._expect('RETURN')
        expr = None
        if not self._check(';'):
            expr = self._parse_expr()
        self._expect(';', "';'")
        return ast.Return(expr)

    # ---------------------------------------------------------- expressions

    def _parse_expr(self):
        node = self._parse_or()
        while self._check('||'):
            self._next()
            node = ast.Binary('||', node, self._parse_or())
        return node

    def _parse_or(self):
        node = self._parse_bitor()
        while self._check('&&'):
            self._next()
            node = ast.Binary('&&', node, self._parse_bitor())
        return node

    def _parse_bitor(self):
        node = self._parse_bitand()
        while self._check('|'):
            self._next()
            node = ast.Binary('|', node, self._parse_bitand())
        return node

    def _parse_bitand(self):
        node = self._parse_equality()
        while self._check('&'):
            self._next()
            node = ast.Binary('&', node, self._parse_equality())
        return node

    def _parse_equality(self):
        node = self._parse_relational()
        while self._check('==') or self._check('!='):
            op = self._next()[0]
            node = ast.Binary(op, node, self._parse_relational())
            self._reject_comparison_chain()
        return node

    def _parse_relational(self):
        node = self._parse_shift()
        while (self._check('<') or self._check('<=') or self._check('>')
               or self._check('>=')):
            op = self._next()[0]
            node = ast.Binary(op, node, self._parse_shift())
            self._reject_comparison_chain()
        return node

    def _parse_shift(self):
        node = self._parse_additive()
        while self._check('<<') or self._check('>>'):
            op = self._next()[0]
            rhs = self._parse_additive()
            # constant folding: IntLit << IntLit with a sane count
            if isinstance(node, ast.IntLit) and isinstance(rhs, ast.IntLit):
                n = rhs.value
                if 0 <= n <= 31:
                    v = (node.value << n) if op == '<<' else (node.value >> n)
                    if -0x80000000 <= v <= 0xFFFFFFFF:
                        node = ast.IntLit(v, node.line)
                        continue
            node = ast.Binary(op, node, rhs)
        return node

    def _reject_comparison_chain(self):
        tok = self._cur()
        if (tok[0] in ('<', '<=', '>', '>=', '==', '!=')):
            raise CompileError(tok[2], tok[3],
                               'comparison operators cannot be chained')

    def _parse_additive(self):
        node = self._parse_multiplicative()
        while self._check('+') or self._check('-'):
            op = self._next()[0]
            node = ast.Binary(op, node, self._parse_multiplicative())
        return node

    def _parse_multiplicative(self):
        node = self._parse_unary()
        while self._check('*') or self._check('/') or self._check('%'):
            op = self._next()[0]
            node = ast.Binary(op, node, self._parse_unary())
        return node

    def _parse_unary(self):
        if (self._check('-') or self._check('!') or self._check('~')):
            op_tok = self._next()
            operand = self._parse_unary()
            # constant folding: -5 -> IntLit(-5), ~5 -> IntLit(-6), !5 -> 0/1
            if isinstance(operand, ast.IntLit):
                v = operand.value
                if op_tok[0] == '-':
                    return ast.IntLit(-v, op_tok[2])
                if op_tok[0] == '~':
                    return ast.IntLit(~v, op_tok[2])
                return ast.IntLit(0 if v else 1, op_tok[2])
            return ast.Unary(op_tok[0], operand)
        return self._parse_primary()

    def _parse_primary(self):
        tok = self._cur()
        if tok[0] == 'INTLIT':
            self._next()
            return ast.IntLit(tok[1], tok[2])
        if tok[0] == 'IDENT':
            name_tok = self._next()
            if self._check('('):
                self._next()
                args = []
                if not self._check(')'):
                    while True:
                        args.append(self._parse_expr())
                        if not self._check(','):
                            break
                        self._next()
                self._expect(')', "')'")
                return ast.Call(name_tok[1], args, name_tok[2])
            self._check_declared(name_tok)
            return ast.Var(name_tok[1])
        if tok[0] == '(':
            self._next()
            node = self._parse_expr()
            self._expect(')', "')'")
            return node
        raise CompileError(tok[2], tok[3],
                           "expected expression, got '%s'" % tok[1])


def parse(tokens):
    return Parser(tokens).parse()
