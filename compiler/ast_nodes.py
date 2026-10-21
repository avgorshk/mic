"""AST node definitions and dump formatting for the MICC compiler."""


class Node:
    pass


class Program(Node):
    def __init__(self, funcs):
        self.funcs = funcs


class FuncDef(Node):
    def __init__(self, name, params, body, line):
        self.name = name
        self.params = params
        self.body = body
        self.line = line


class Block(Node):
    def __init__(self, stmts):
        self.stmts = stmts


class VarDecl(Node):
    def __init__(self, name, init):
        self.name = name
        self.init = init


class Assign(Node):
    def __init__(self, name, expr):
        self.name = name
        self.expr = expr


class If(Node):
    def __init__(self, cond, then_body, else_body):
        self.cond = cond
        self.then_body = then_body
        self.else_body = else_body


class While(Node):
    def __init__(self, cond, body):
        self.cond = cond
        self.body = body


class Return(Node):
    def __init__(self, expr):
        self.expr = expr


class ExprStmt(Node):
    def __init__(self, expr):
        self.expr = expr


class Binary(Node):
    def __init__(self, op, left, right):
        self.op = op
        self.left = left
        self.right = right


class Unary(Node):
    def __init__(self, op, operand):
        self.op = op
        self.operand = operand


class IntLit(Node):
    def __init__(self, value):
        self.value = value


class Var(Node):
    def __init__(self, name):
        self.name = name


class Call(Node):
    def __init__(self, name, args):
        self.name = name
        self.args = args


def dump(node, indent=0):
    """Render the AST as a stable, human-readable string.

    One node per line, 2 spaces per level, children on following lines.
    If/While print cond, body, and an 'Else' marker line when present.
    """
    pad = '  ' * indent
    head = []
    children = []

    if isinstance(node, Program):
        head = ['Program']
        children = node.funcs

    elif isinstance(node, FuncDef):
        head = ['FuncDef %s' % node.name]
        head += ['  Param %s' % p for p in node.params]
        children = [node.body]

    elif isinstance(node, Block):
        head = ['Block']
        children = node.stmts

    elif isinstance(node, VarDecl):
        head = ['VarDecl %s' % node.name]
        children = [node.init] if node.init is not None else []

    elif isinstance(node, Assign):
        head = ['Assign %s' % node.name]
        children = [node.expr]

    elif isinstance(node, If):
        out = [pad + 'If']
        out += dump(node.cond, indent + 1)
        out += dump(node.then_body, indent + 1)
        if node.else_body is not None:
            out.append(pad + '  Else')
            out += dump(node.else_body, indent + 2)
        return out

    elif isinstance(node, While):
        head = ['While']
        children = [node.cond, node.body]

    elif isinstance(node, Return):
        head = ['Return']
        children = [node.expr] if node.expr is not None else []

    elif isinstance(node, ExprStmt):
        head = ['ExprStmt']
        children = [node.expr]

    elif isinstance(node, Binary):
        head = ['Binary %s' % node.op]
        children = [node.left, node.right]

    elif isinstance(node, Unary):
        head = ['Unary %s' % node.op]
        children = [node.operand]

    elif isinstance(node, IntLit):
        head = ['IntLit %d' % node.value]

    elif isinstance(node, Var):
        head = ['Var %s' % node.name]

    elif isinstance(node, Call):
        head = ['Call %s' % node.name]
        children = node.args

    else:
        head = ['?%s' % type(node).__name__]

    out = [pad + l for l in head]
    for c in children:
        out += dump(c, indent + 1)
    return out
