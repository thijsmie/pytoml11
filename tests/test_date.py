from datetime import date

from pytoml11 import Array, Date, Table, dumps


def test_init_date():
    date_value = Date(date(2023, 1, 1))
    assert date_value.value == date(2023, 1, 1)


def test_copy_date():
    date_value = Date(date(2023, 1, 1))
    copy_date = date_value.copy()
    assert isinstance(copy_date, Date)
    assert copy_date.value == date(2023, 1, 1)
    assert id(copy_date) != id(date_value)


def test_repr_date():
    date_value = Date(date(2023, 1, 1))
    assert repr(date_value) == "Date(2023-01-01)"


def test_owned_property_date():
    date_value = Date(date(2023, 1, 1))
    assert date_value.owned is False
    array = Array([date_value])
    assert array[0].owned is True


def test_date_in_table():
    date_value = Date(date(2023, 1, 1))
    table = Table({"key": date_value})
    assert dumps(table) == "key = 2023-01-01\n\n"


def test_date_comments():
    date_value = Date(date(2023, 1, 1), comments=[" Comment 1", " Comment 2"])
    table = Table({"key": date_value})
    assert dumps(table) == "# Comment 1\n# Comment 2\nkey = 2023-01-01\n\n"
