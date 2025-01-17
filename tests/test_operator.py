from pytoml11 import Table, Boolean


def test_equality_basic():
    assert Boolean(True) == Boolean(True)
    assert Boolean(False) == Boolean(False)
    assert Boolean(True) != Boolean(False)
    assert Boolean(False) != Boolean(True)


def test_equality_nested():
    assert Table({"a": Boolean(True)}) == Table({"a": Boolean(True)})
    assert Table({"a": Boolean(True)}) != Table({"a": Boolean(False)})
    assert Table({"a": Boolean(True)}) != Table({"b": Boolean(True)})


def test_equality_with_comments():
    assert Table({"a": Boolean(True, comments=["comment"])}) == Table({"a": Boolean(True, comments=["comment"])})
    assert Table({"a": Boolean(True, comments=["comment"])}) != Table({"a": Boolean(True, comments=["comment2"])})
    assert Table({"a": Boolean(True, comments=["comment"])}) != Table({"a": Boolean(False, comments=["comment"])})
    assert Table({"a": Boolean(True, comments=["comment"])}) != Table({"b": Boolean(True, comments=["comment"])})
