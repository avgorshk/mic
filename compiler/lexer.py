"""Lexer for the MICC language.

Tokens: ('IDENT', name, line, col) | ('INTLIT', value, line, col)
| keyword types ('INT','IF','ELSE','WHILE','RETURN')
| operator/punctuation types ('+','-','*','/','%','&','|','~','!',
  '<','<=','>','>=','==','!=','&&','||','=','(',')','{','}',';',',')
| ('EOF', '', line, col).

Comments: // line comments and /* ... */ block comments.
Integer literals: decimal or 0x-prefixed hex.
"""


class CompileError(Exception):
    def __init__(self, line, col, msg):
        super().__init__(msg)
        self.line = line
        self.col = col
        self.msg = msg


KEYWORDS = {
    'int': 'INT',
    'if': 'IF',
    'else': 'ELSE',
    'while': 'WHILE',
    'return': 'RETURN',
}

TWO_CHAR_OPS = ('==', '!=', '<=', '>=', '&&', '||', '<<', '>>', '+=', '-=')
ONE_CHAR_OPS = '+-*/%&|~!<>=(){};,'

ALNUM_START = set('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_')
ALNUM = ALNUM_START | set('0123456789')
DIGITS = set('0123456789')
HEX_DIGITS = DIGITS | set('abcdefABCDEF')


class Lexer:
    def __init__(self, source):
        self.src = source
        self.pos = 0
        self.line = 1
        self.col = 1

    def _error(self, msg):
        raise CompileError(self.line, self.col, msg)

    def _peek(self, ahead=0):
        i = self.pos + ahead
        return self.src[i] if i < len(self.src) else ''

    def _advance(self):
        c = self.src[self.pos]
        self.pos += 1
        if c == '\n':
            self.line += 1
            self.col = 1
        else:
            self.col += 1
        return c

    def _skip_whitespace_and_comments(self):
        while self.pos < len(self.src):
            c = self._peek()
            if c in ' \t\r\n':
                self._advance()
            elif c == '/' and self._peek(1) == '/':
                while self.pos < len(self.src) and self._peek() != '\n':
                    self._advance()
            elif c == '/' and self._peek(1) == '*':
                start_line, start_col = self.line, self.col
                self._advance()
                self._advance()
                while True:
                    if self.pos >= len(self.src):
                        raise CompileError(
                            start_line, start_col, 'unterminated comment')
                    if self._peek() == '*' and self._peek(1) == '/':
                        self._advance()
                        self._advance()
                        break
                    self._advance()
            else:
                break

    def _read_number(self):
        line, col = self.line, self.col
        start = self.pos
        if self._peek() == '0' and self._peek(1) in ('x', 'X'):
            self._advance()
            self._advance()
            if self.pos >= len(self.src) or self._peek() not in HEX_DIGITS:
                raise CompileError(line, col, 'invalid hex literal')
            while self.pos < len(self.src) and self._peek() in HEX_DIGITS:
                self._advance()
            value = int(self.src[start + 2:self.pos], 16)
        else:
            while self.pos < len(self.src) and self._peek() in DIGITS:
                self._advance()
            value = int(self.src[start:self.pos], 10)
        return ('INTLIT', value, line, col)

    def _read_ident(self):
        line, col = self.line, self.col
        start = self.pos
        while self.pos < len(self.src) and self._peek() in ALNUM:
            self._advance()
        name = self.src[start:self.pos]
        if name in KEYWORDS:
            return (KEYWORDS[name], name, line, col)
        return ('IDENT', name, line, col)

    def next_token(self):
        self._skip_whitespace_and_comments()
        if self.pos >= len(self.src):
            return ('EOF', '', self.line, self.col)
        line, col = self.line, self.col
        c = self._peek()

        if c in DIGITS:
            return self._read_number()
        if c in ALNUM_START:
            return self._read_ident()

        two = c + self._peek(1)
        if two in TWO_CHAR_OPS:
            self._advance()
            self._advance()
            return (two, two, line, col)
        if c in ONE_CHAR_OPS:
            self._advance()
            return (c, c, line, col)
        raise CompileError(line, col, "unexpected character '%s'" % c)

    def tokenize(self):
        tokens = []
        while True:
            tok = self.next_token()
            tokens.append(tok)
            if tok[0] == 'EOF':
                return tokens
