#include "list.hpp"

#include <cprime/support/prelude.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace cprime::ast {
namespace {

using Catch::Matchers::RangeEquals;

TEST_CASE("List is initially empty", "[cprime][ast]")
{
    struct Data final : public ListHook
    {
    };

    List<Data> list{};

    REQUIRE(list.size() == 0);
    REQUIRE(list.begin() == list.end());
    // front() и back() здесь не вызываем -- это UB
}

TEST_CASE("List supports insertion at both ends", "[cprime][ast]")
{
    struct Data final : public ListHook
    {
        int value;

        explicit Data(int value)
            : value{value}
        {
        }

        auto operator==(const Data& rhs) const -> bool
        {
            return value == rhs.value;
        }
    };

    List<Data> list{};

    Data a{101};
    Data b{102};
    Data c{103};
    Data d{104};
    Data e{105};
    Data f{106};

    list.push_back(c);
    list.push_back(d);
    list.push_back(e);
    REQUIRE(list.size() == 3);
    REQUIRE(list.front() == Data{103});
    REQUIRE(list.back() == Data{105});
    REQUIRE_THAT(
        list,
        RangeEquals({
            Data{103},
            Data{104},
            Data{105},
        }));

    list.push_front(b);
    list.push_front(a);
    REQUIRE(list.size() == 5);
    REQUIRE(list.front() == Data{101});
    REQUIRE(list.back() == Data{105});
    REQUIRE_THAT(
        list,
        RangeEquals({
            Data{101},
            Data{102},
            Data{103},
            Data{104},
            Data{105},
        }));

    list.push_back(f);
    REQUIRE(list.size() == 6);
    REQUIRE(list.front() == Data{101});
    REQUIRE(list.back() == Data{106});
    REQUIRE_THAT(
        list,
        RangeEquals({
            Data{101},
            Data{102},
            Data{103},
            Data{104},
            Data{105},
            Data{106},
        }));
}

TEST_CASE("Default-constructed ListIterators compare equal", "[cprime][ast]")
{
    struct Data final : public ListHook
    {
    };

    REQUIRE(ListIterator<Data>{} == ListIterator<Data>{});
}

// TODO: add sentinel for ast::List
// TEST_CASE("List supports bidirectional iteration", "[cprime][ast]")
// {
//     struct Data : public ListHook
//     {
//     };
//
//     Data a{};
//     Data b{};
//     Data c{};
//
//     List<Data> list{};
//
//     list.push_back(a);
//     list.push_back(b);
//     list.push_back(c);
//
//     auto it = list.begin();
//
//     REQUIRE(&*it == &a);
//     REQUIRE(&*++it == &b);
//     REQUIRE(&*it++ == &b);
//     REQUIRE(&*it == &c);
//     REQUIRE(++it == list.end());
//
//     REQUIRE(it-- == list.end());
//     REQUIRE(&*it == &c);
//     REQUIRE(&*--it == &b);
//     REQUIRE(&*it-- == &b);
//     REQUIRE(&*it == &a);
//     REQUIRE(it == list.begin());
// }

} // namespace
} // namespace cprime::ast
