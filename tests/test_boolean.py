from pytoml11 import Array, Boolean


def test_init_true():
    true_boolean = Boolean(True)
    assert true_boolean.value is True


def test_init_false():
    false_boolean = Boolean(False)
    assert false_boolean.value is False


def test_copy():
    true_boolean = Boolean(True)
    copy_boolean = true_boolean.copy()
    assert isinstance(copy_boolean, Boolean)
    assert copy_boolean.value is True
    assert id(copy_boolean) != id(true_boolean)


def test_repr_true():
    true_boolean = Boolean(True)
    assert repr(true_boolean) == "Boolean(True)"


def test_owned_property():
    true_boolean = Boolean(True)
    assert true_boolean.owned is False
    array = Array([true_boolean])
    assert array[0].owned is True
