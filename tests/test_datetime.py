from datetime import datetime, timedelta, timezone

from pytoml11 import Array, DateTime, Table, dumps


def test_init_datetime_without_timezone():
    dt_value = DateTime(datetime(2023, 1, 1, 12, 34, 56))
    assert dt_value.value == datetime(2023, 1, 1, 12, 34, 56)


def test_init_datetime_with_timezone():
    dt_value = DateTime(datetime(2023, 1, 1, 12, 34, 56, tzinfo=timezone.utc))
    assert dt_value.value == datetime(2023, 1, 1, 12, 34, 56, tzinfo=timezone.utc)


def test_copy_datetime_without_timezone():
    dt_value = DateTime(datetime(2023, 3, 2, 12, 34, 56))
    copy_dt = dt_value.copy()
    assert isinstance(copy_dt, DateTime)
    assert copy_dt.value == datetime(2023, 3, 2, 12, 34, 56)
    assert id(copy_dt) != id(dt_value)


def test_copy_datetime_with_timezone():
    dt_value = DateTime(datetime(2023, 1, 1, 12, 34, 56, tzinfo=timezone.utc))
    copy_dt = dt_value.copy()
    assert isinstance(copy_dt, DateTime)
    assert copy_dt.value == datetime(2023, 1, 1, 12, 34, 56, tzinfo=timezone.utc)
    assert id(copy_dt) != id(dt_value)


def test_repr_datetime_without_timezone():
    dt_value = DateTime(datetime(2023, 3, 2, 12, 34, 56))
    assert repr(dt_value) == "DateTime(2023-03-02T12:34:56)"


def test_repr_datetime_with_timezone():
    dt_value = DateTime(
        datetime(2023, 1, 1, 12, 34, 56, tzinfo=timezone(timedelta(hours=1)))
    )
    assert repr(dt_value) == "DateTime(2023-01-01T12:34:56+01:00)"


def test_owned_property_datetime():
    dt_value = DateTime(datetime(2023, 1, 1, 12, 34, 56))
    assert dt_value.owned is False
    array = Array([dt_value])
    assert array[0].owned is True


def test_datetime_in_table_without_timezone():
    dt_value = DateTime(datetime(2023, 1, 1, 12, 34, 56))
    table = Table({"key": dt_value})
    assert dumps(table) == "key = 2023-01-01T12:34:56.000000\n\n"


def test_datetime_in_table_with_timezone():
    dt_value = DateTime(datetime(2023, 3, 2, 12, 34, 56, tzinfo=timezone.utc))
    table = Table({"key": dt_value})
    assert dumps(table) == "key = 2023-03-02T12:34:56.000000Z\n\n"


def test_datetime_comments():
    dt_value = DateTime(
        datetime(2023, 3, 2, 12, 34, 56), comments=[" Comment 1", " Comment 2"]
    )
    table = Table({"key": dt_value})
    assert (
        dumps(table) == "# Comment 1\n# Comment 2\nkey = 2023-03-02T12:34:56.000000\n\n"
    )
