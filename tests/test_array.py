from pytoml11 import Array, Boolean, Integer, String


def test_init_array():
    array = Array([Integer(1), Boolean(True), String("hello")])
    assert len(array) == 3
    assert isinstance(array[0], Integer)
    assert isinstance(array[1], Boolean)
    assert isinstance(array[2], String)


def test_append_array():
    array = Array([])
    integer = Integer(42)
    array.append(integer)
    assert len(array) == 1
    assert array[0] is integer
    assert integer.owned is True


def test_insert_array():
    array = Array([Integer(1), String("hello")])
    boolean = Boolean(True)
    array.insert(1, boolean)
    assert len(array) == 3
    assert array[1] is boolean
    assert boolean.owned is True


def test_setitem_array():
    array = Array([Integer(1), String("hello")])
    boolean = Boolean(True)
    array[1] = boolean
    assert array[1] is boolean
    assert boolean.owned is True


def test_delitem_array():
    array = Array([Integer(1), Boolean(True), String("hello")])
    boolean = array[1]
    del array[1]
    assert len(array) == 2
    assert boolean.owned is False


def test_pop_array():
    array = Array([Integer(1), Boolean(True), String("hello")])
    boolget = array[1]
    assert boolget.owned is True
    boolean = array.pop(1)
    assert len(array) == 2
    assert boolean.owned is False
    assert boolean is boolget


def test_copy_array():
    array = Array([Integer(1), Boolean(True), String("hello")])
    copy_array = array.copy()
    assert len(copy_array) == len(array)
    assert copy_array[0].value == array[0].value
    assert copy_array[1].value == array[1].value
    assert copy_array[2].value == array[2].value
    assert id(copy_array) != id(array)


def test_array_value_property():
    prev = [Integer(1), Boolean(True), String("hello")]
    array = Array(prev)
    assert len(array.value) == 3
    assert array[0] is prev[0]
    assert array[1] is prev[1]
    assert array[2] is prev[2]
    assert array[0].owned
    assert array[1].owned
    assert array[2].owned


def test_array_repr():
    array = Array([Integer(1), Boolean(True), String("hello")])
    assert repr(array) == 'Array([Integer(1), Boolean(True), String("hello")])'


def test_array_extend():
    array = Array([Integer(1), Boolean(True)])
    to_extend = [String("hello"), Integer(42)]

    array.extend(to_extend)

    assert len(array) == 4
    assert isinstance(array[2], String)
    assert array[2].value == "hello"
    assert isinstance(array[3], Integer)
    assert array[3].value == 42

    # Check ownership
    assert array[0].owned is True
    assert array[1].owned is True
    assert array[2].owned is True
    assert array[3].owned is True


def test_array_clear():
    val = Integer(1)
    array = Array([val, Boolean(True), String("hello")])
    assert val.owned is True
    array.clear()
    assert len(array) == 0
    assert array.value == []
    assert val.owned is False


def test_array_contains():
    val = Integer(1)
    array = Array([val, Boolean(True), String("hello")])
    assert val in array
    assert Boolean(True) in array
    assert String("hello") in array
    assert Integer(42) not in array
    assert Boolean(False) not in array
    assert String("world") not in array
    assert Array([]) not in array
    assert Array([Integer(2)]) not in array
