from pytoml11 import Array, Integer, Table, dumps


def test_init_integer():
    integer = Integer(42)
    assert integer.value == 42


def test_copy_integer():
    integer = Integer(42)
    copy_integer = integer.copy()
    assert isinstance(copy_integer, Integer)
    assert copy_integer.value == 42
    assert id(copy_integer) != id(integer)


def test_repr_integer():
    integer = Integer(42)
    assert repr(integer) == "Integer(42)"


def test_owned_property_integer():
    integer = Integer(42)
    assert integer.owned is False
    array = Array([integer])
    assert array[0].owned is True


def test_integer_in_table():
    integer = Integer(42)
    table = Table({"key": integer})
    assert dumps(table) == "key = 42\n\n"


def test_integer_comments():
    integer = Integer(42, comments=[" Comment 1", " Comment 2"])
    table = Table({"key": integer})
    assert dumps(table) == "# Comment 1\n# Comment 2\nkey = 42\n\n"
