from pytoml11 import Array, Float, Table, dumps


def test_init_float():
    float_value = Float(3.14)
    assert float_value.value == 3.14


def test_copy_float():
    float_value = Float(3.14)
    copy_float = float_value.copy()
    assert isinstance(copy_float, Float)
    assert copy_float.value == 3.14
    assert id(copy_float) != id(float_value)


def test_repr_float():
    float_value = Float(3.14)
    assert repr(float_value) == "Float(3.14)"


def test_owned_property_float():
    float_value = Float(3.14)
    assert float_value.owned is False
    array = Array([float_value])
    assert array[0].owned is True


def test_float_in_table():
    float_value = Float(3.14)
    table = Table({"key": float_value})
    assert dumps(table) == "key = 3.14\n\n"


def test_float_comments():
    float_value = Float(3.14, comments=[" Comment 1", " Comment 2"])
    table = Table({"key": float_value})
    assert dumps(table) == "# Comment 1\n# Comment 2\nkey = 3.14\n\n"
