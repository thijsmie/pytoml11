import pytest

from pytoml11 import Boolean, Item, Table, dumps


@pytest.fixture
def item() -> Item:
    # Items are abstract classes, so we can't create an instance of them.
    # We'll use a Boolean instance instead.
    return Boolean(True)


def test_item_inheritance(item: Item):
    assert isinstance(item, Item)


def test_item_comments():
    assert Boolean(True).comments == []
    assert Boolean(
        True, comments=["This is a comment", "Another comment"]
    ).comments == ["This is a comment", "Another comment"]


def test_items_set_comments(item: Item):
    item.comments = [" A", " B"]
    assert dumps(Table({"item": item})) == "# A\n# B\nitem = true\n\n"


def test_item_owned(item: Item):
    assert item.owned is False


def test_item_format():
    t = Table(
        {"item": Boolean(True, comments=[" This is a comment", " Another comment"])}
    )
    assert dumps(t) == "# This is a comment\n# Another comment\nitem = true\n\n"
