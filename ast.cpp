#include "ast.hpp"

PyObject *IntNode::accept(NodeVisitor *visitor) { return visitor->visitIntNode(this); }
PyObject *FloatNode::accept(NodeVisitor *visitor) { return visitor->visitFloatNode(this); }
PyObject *StringNode::accept(NodeVisitor *visitor) { return visitor->visitStringNode(this); }
PyObject *BooleanNode::accept(NodeVisitor *visitor) { return visitor->visitBooleanNode(this); }
PyObject *NullNode::accept(NodeVisitor *visitor) { return visitor->visitNullNode(this); }
PyObject *NameNode::accept(NodeVisitor *visitor) { return visitor->visitNameNode(this); }
PyObject *BinaryOpNode::accept(NodeVisitor *visitor) { return visitor->visitBinaryOpNode(this); }
PyObject *UnaryOpNode::accept(NodeVisitor *visitor) { return visitor->visitUnaryOpNode(this); }
PyObject *AssignNode::accept(NodeVisitor *visitor) { return visitor->visitAssignNode(this); }
PyObject *BlockNode::accept(NodeVisitor *visitor) { return visitor->visitBlockNode(this); }
PyObject *ProgramNode::accept(NodeVisitor *visitor) { return visitor->visitProgramNode(this); }
PyObject *PrintNode::accept(NodeVisitor *visitor) { return visitor->visitPrintNode(this); }