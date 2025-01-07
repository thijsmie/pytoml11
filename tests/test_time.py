from datetime import time

from pytoml11 import Array, Table, Time, dumps


def test_init_time():
    time_value = Time(time(12, 34, 56))
    assert time_value.value == time(12, 34, 56)


def test_copy_time():
    time_value = Time(time(12, 34, 56))
    copy_time = time_value.copy()
    assert isinstance(copy_time, Time)
    assert copy_time.value == time(12, 34, 56)
    assert id(copy_time) != id(time_value)


def test_repr_time():
    time_value = Time(time(12, 34, 56))
    assert repr(time_value) == "Time(12:34:56)"


def test_owned_property_time():
    time_value = Time(time(12, 34, 56))
    assert time_value.owned is False
    array = Array([time_value])
    assert array[0].owned is True


def test_time_in_table():
    time_value = Time(time(12, 34, 56))
    table = Table({"key": time_value})
    assert dumps(table) == "key = 12:34:56.000000\n\n"


def test_time_comments():
    time_value = Time(time(12, 34, 56), comments=[" Comment 1", " Comment 2"])
    table = Table({"key": time_value})
    assert dumps(table) == "# Comment 1\n# Comment 2\nkey = 12:34:56.000000\n\n"
