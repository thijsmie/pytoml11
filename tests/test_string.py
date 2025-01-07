from pytoml11 import Array, String, Table, dumps


def test_init_string():
    string_value = String("hello")
    assert string_value.value == "hello"


def test_copy_string():
    string_value = String("hello")
    copy_string = string_value.copy()
    assert isinstance(copy_string, String)
    assert copy_string.value == "hello"
    assert id(copy_string) != id(string_value)


def test_repr_string():
    string_value = String("hello")
    assert repr(string_value) == 'String("hello")'


def test_owned_property_string():
    string_value = String("hello")
    assert string_value.owned is False
    array = Array([string_value])
    assert array[0].owned is True


def test_string_in_table():
    string_value = String("hello")
    table = Table({"key": string_value})
    assert dumps(table) == 'key = "hello"\n\n'


def test_string_comments():
    string_value = String("hello", comments=[" Comment 1", " Comment 2"])
    table = Table({"key": string_value})
    assert dumps(table) == '# Comment 1\n# Comment 2\nkey = "hello"\n\n'
