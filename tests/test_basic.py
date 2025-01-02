import pytoml11 as tml


def test_boolean():
    b = tml.Boolean(True)

def test_float():
    f = tml.Float(1.0)


def test_integer():
    i = tml.Integer(1)


def test_none():
    n = tml.None_(None)


def test_string():
    s = tml.String("test")


def test_array():
    a = tml.Array([tml.Boolean(True), tml.Integer(1), tml.Float(1.0), tml.String("test")])

def test_table():
    t = tml.Table({"test": tml.Boolean(True)})