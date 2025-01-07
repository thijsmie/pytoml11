from pytoml11 import Array, Null, Table, dumps


def test_init_none():
    Nullvalue = Null()
    assert Nullvalue.value is None


def test_init_none_with_value():
    Nullvalue = Null(None)
    assert Nullvalue.value is None


def test_copy_none():
    Nullvalue = Null()
    copy_none = Nullvalue.copy()
    assert isinstance(copy_none, Null)
    assert copy_none.value is None
    assert id(copy_none) != id(Nullvalue)


def test_repr_none():
    Nullvalue = Null()
    assert repr(Nullvalue) == "Null()"


def test_owned_property_none():
    Nullvalue = Null()
    assert Nullvalue.owned is False
    array = Array([Nullvalue])
    assert array[0].owned is True


def test_Nullin_table():
    Nullvalue = Null()
    table = Table({"key": Nullvalue})
    assert dumps(table) == "key = null\n\n"


def test_Nullcomments():
    Nullvalue = Null(comments=[" Comment 1", " Comment 2"])
    table = Table({"key": Nullvalue})
    assert dumps(table) == "# Comment 1\n# Comment 2\nkey = null\n\n"
